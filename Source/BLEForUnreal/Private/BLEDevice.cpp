
#include "BLEDevice.h"

#include "IBLETransport.h"

#include "Parser/BLEParserFactory.h"
#include "Parser/IBLECharacteristicParser.h"

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

bool UBLEDevice::SubscribeToMetric(FName MetricName)
{
	IBLECharacteristicParser* const* FoundParser = AvailableParsers.Find(MetricName);
	if (!FoundParser || !Transport)
	{
		return false;
	}

	IBLECharacteristicParser* Parser = *FoundParser;

	const FString CharUuid = Parser->GetCharacteristicUuid();
	if (!ActiveParsers.Contains(CharUuid))
	{
		Parser->Reset();
		Transport->SubscribeToCharacteristic(DeviceId, Parser->GetServiceUuid(), CharUuid);
		ActiveParsers.Add({ CharUuid, Parser });
	}

	return true;
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
		OnMetricUpdated.Broadcast(Metric.MetricName, Metric.Value);
	}
}