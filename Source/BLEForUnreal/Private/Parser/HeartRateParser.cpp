#include "Parser/HeartRateParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

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

TArray<FBLEMetric> FHeartRateParser::ParseNotify(const TArray<uint8>& Data)
{
	float HeartRate = 0.0f;

	const uint8_t* BasePtr = Data.GetData();
	if (BasePtr)
	{
		uint8_t FlagsField = BasePtr[0];

		const void* HeartRateMeasurementField = static_cast<const void*>(&BasePtr[1]);

		if ((FlagsField & 0x1) != 0)
		{
			HeartRate = static_cast<float>(*static_cast<const uint16_t*>(HeartRateMeasurementField));
		}
		else
		{
			HeartRate = static_cast<float>(*static_cast<const uint8_t*>(HeartRateMeasurementField));
		}
	}

	return { {BLEMetricNames::HeartRateBpm, HeartRate} };
}