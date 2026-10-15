#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FFMSStatusParser : public IBLECharacteristicParser
{
public:
	METRIC_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSStatus,
		BLEMetricNames::FMSStatusTargetPowerChanged
	);

	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override
	{
		TArray<FBLEMetric> Metrics;

		uint8 OpCode = Data[0];
		// Target power changed
		if (OpCode == 0x08)
		{
			float Wattage = static_cast<float>(*reinterpret_cast<const int16*>(&Data[1]));
			Metrics.Add({ BLEMetricNames::FMSStatusTargetPowerChanged, Wattage });
		}

		return Metrics;
	}

	virtual void Reset() override {};
};