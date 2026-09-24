
#include "BLEDevice.h"

#include "IBLETransport.h"

#include "Parser/BLEParserFactory.h"
#include "Parser/IBLECharacteristicParser.h"
#include "BLEMetricSubscription.h"
#include "BLEReadRequest.h"

UBLEDevice::UBLEDevice() = default;
UBLEDevice::UBLEDevice(FVTableHelper& Helper) : Super(Helper) {}
UBLEDevice::~UBLEDevice() = default;

void UBLEDevice::Initialize(const FString& InDeviceId, IBLETransport* InTransport, 
	const TArray<FString>& DiscoveredServiceUuids)
{
	DeviceId = InDeviceId;
	Transport = InTransport;

	for (TUniquePtr<IBLECharacteristicParser>& Parser : CreateParsersForServices(DiscoveredServiceUuids))
	{
		IBLECharacteristicParser* RawParser = Parser.Get();
		OwnedParsers.Add(MoveTemp(Parser));

		for (const FName& MetricName : RawParser->GetSupportedMetrics())
		{
			AvailableParsersByMetric.Add({ MetricName, RawParser });
		}

		for (const FName& ReadName : RawParser->GetSupportedReadRequests())
		{
			AvailableParsersByRead.Add({ ReadName, RawParser });
		}
	}
}

void UBLEDevice::Disconnect()
{
	if (Transport)
	{
		Transport->Disconnect(DeviceId);
	}
}

TArray<FName> UBLEDevice::GetAvailableMetrics() const
{
	TArray<FName> Result;
	AvailableParsersByMetric.GenerateKeyArray(Result);
	return Result;
}

UBLEMetricSubscription* UBLEDevice::SubscribeToMetric(FName MetricName)
{
	IBLECharacteristicParser* const* FoundParser = AvailableParsersByMetric.Find(MetricName);
	if (!FoundParser || !Transport)
	{
		return nullptr;
	}

	IBLECharacteristicParser* Parser = *FoundParser;
	const FString CharUuid = Parser->GetCharacteristicUuid();

	bool bIsAlreadyIsActive = ActiveParsers.Contains(CharUuid);

	if (!bIsAlreadyIsActive)
	{
		Parser->Reset();
		Transport->SubscribeToCharacteristic(DeviceId, Parser->GetServiceUuid(), CharUuid);
		ActiveParsers.Add({ CharUuid, Parser });
	}

	UBLEMetricSubscription* Subscription = NewObject<UBLEMetricSubscription>(this);
	Subscription->MetricName = MetricName;
	Subscription->OwningDevice = this;
	ActiveSubscriptions.Add(Subscription);

	return Subscription;
}

UBLEReadRequest* UBLEDevice::RequestValueRead(FName ReadName)
{
	IBLECharacteristicParser* const* FoundParser = AvailableParsersByRead.Find(ReadName);
	if (*FoundParser)
	{
		return nullptr;
	}

	UBLEReadRequest* Request = NewObject<UBLEReadRequest>(this);

	TArray<TObjectPtr<UBLEReadRequest>>& RequestArray = ReadRequests.FindOrAdd(ReadName).Array;
	RequestArray.Add(Request);
	if (RequestArray.Num() == 1)
	{
		// Handle through the transport or subsystem?
		// as the BLE spec states that only one request can be handled, others are discarded.
		// At the OS level windows still queues internally. Should we rely on this behaviour?
		// Or handle it in the subsystem to explicitly queue these operations?
	}

	return Request;
}

void UBLEDevice::HandleCharacteristicData(const FString& CharacteristicUuid, const FBLECharacteristicData& Data)
{
	IBLECharacteristicParser* const* FoundParser = ActiveParsers.Find(CharacteristicUuid);
	if (!FoundParser)
	{
		return;
	}

	IBLECharacteristicParser* Parser = *FoundParser;

	if (!Parser->GetSupportedMetrics().IsEmpty())
	{
		for (const FBLEMetric& Metric : (*FoundParser)->ParseNotify(Data))
		{
			LatestMetricValues.Add(Metric.MetricName, Metric.Value);

			for (UBLEMetricSubscription* Subscription : ActiveSubscriptions)
			{
				if (Subscription && Subscription->GetMetricName() == Metric.MetricName)
				{
					Subscription->OnValueUpdated.Broadcast(Metric.Value);
				}
			}
		}
	}

	if (!Parser->GetSupportedReadRequests().IsEmpty())
	{
		for (const FBLERead& Read : (*FoundParser)->ParseReadRequest(Data))
		{
			FReadArray* FoundRequestArray = ReadRequests.Find(Read.ReadName);
			if (FoundRequestArray)
			{
				for (UBLEReadRequest* Request : (*FoundRequestArray).Array)
				{
					Request->OnRequestCompleted.Broadcast(Read.String);
				}

				ReadRequests.Remove(Read.ReadName);
			}
		}
	}
}

void UBLEDevice::RemoveSubscription(UBLEMetricSubscription* Subscription)
{
	if (!Subscription)
	{
		return;
	}

	ActiveSubscriptions.RemoveSingleSwap(Subscription);

	IBLECharacteristicParser* const* FoundParser = AvailableParsersByMetric.Find(Subscription->GetMetricName());
	if (!FoundParser || !Transport)
	{
		return;
	}

	IBLECharacteristicParser* Parser = *FoundParser;
	const FString CharUuid = Parser->GetCharacteristicUuid();

	bool bAnyRemaningSubscriptionsOnCharacteristic = 
		ActiveSubscriptions.ContainsByPredicate([this, &CharUuid](const UBLEMetricSubscription* Other)
		{
			IBLECharacteristicParser* const* OtherParser = AvailableParsersByMetric.Find(Other->GetMetricName());
			return OtherParser && (*OtherParser)->GetCharacteristicUuid() == CharUuid;
		}
	);

	if (!bAnyRemaningSubscriptionsOnCharacteristic)
	{
		Transport->UnsubscribeFromCharacteristic(DeviceId, Parser->GetServiceUuid(), CharUuid);
		ActiveParsers.Remove(CharUuid);
	}
}