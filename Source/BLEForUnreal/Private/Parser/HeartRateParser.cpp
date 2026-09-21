#include "Parser/HeartRateParser.h"

#include "BLEGattUuids.h"
#include "BLEMetricNames.h"

FString FHeartRateParser::GetServiceUuid() const
{
	return BLEGattUuids::HeartRateService;
}

FString FHeartRateParser::GetCharacteristicUuid() const
{
	return BLEGattUuids::HeartRateMeasurement;
}

TArray<FName> FHeartRateParser::GetSupportedMetrics() const
{
	return { BLEMetricNames::HeartRateBpm };
}

TArray<FBLEMetric> FHeartRateParser::Parse(const TArray<uint8>& Data)
{
	return {};
}

void FHeartRateParser::Reset()
{

}