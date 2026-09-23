
#include "BLEDevice.h"

#include "IBLETransport.h"

#include "Parser/BLEParserFactory.h"
#include "Parser/IBLECharacteristicParser.h"
#include "BLEMetricSubscription.h"

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
			AvailableParsers.Add({ MetricName, RawParser });
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
	AvailableParsers.GenerateKeyArray(Result);
	return Result;
}

UBLEMetricSubscription* UBLEDevice::SubscribeToMetric(FName MetricName)
{
	IBLECharacteristicParser* const* FoundParser = AvailableParsers.Find(MetricName);
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

void UBLEDevice::HandleCharacteristicData(const FString& CharacteristicUuid, const FBLECharacteristicData& Data)
{
	IBLECharacteristicParser* const* FoundParser = ActiveParsers.Find(CharacteristicUuid);
	if (!FoundParser)
	{
		return;
	}

	for (const FBLEMetric& Metric : (*FoundParser)->Parse(Data))
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

void UBLEDevice::RemoveSubscription(UBLEMetricSubscription* Subscription)
{
	if (!Subscription)
	{
		return;
	}

	ActiveSubscriptions.RemoveSingleSwap(Subscription);

	IBLECharacteristicParser* const* FoundParser = AvailableParsers.Find(Subscription->GetMetricName());
	if (!FoundParser || !Transport)
	{
		return;
	}

	IBLECharacteristicParser* Parser = *FoundParser;
	const FString CharUuid = Parser->GetCharacteristicUuid();

	bool bAnyRemaningSubscriptionsOnCharacteristic = 
		ActiveSubscriptions.ContainsByPredicate([this, &CharUuid](const UBLEMetricSubscription* Other)
		{
			IBLECharacteristicParser* const* OtherParser = AvailableParsers.Find(Other->GetMetricName());
			return OtherParser && (*OtherParser)->GetCharacteristicUuid() == CharUuid;
		}
	);

	if (!bAnyRemaningSubscriptionsOnCharacteristic)
	{
		Transport->UnsubscribeFromCharacteristic(DeviceId, Parser->GetServiceUuid(), CharUuid);
		ActiveParsers.Remove(CharUuid);
	}
}