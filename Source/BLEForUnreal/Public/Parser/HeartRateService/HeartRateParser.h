#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FHeartRateParser : public IBLECharacteristicParser
{
public:
	METRIC_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::HeartRateService,
		BLEGattUuids::HeartRateMeasurement,
		BLEMetricNames::HeartRateBpm
	);

	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override
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

	virtual void Reset() override { }
};