#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FFMSTrainingStatusParser : public IBLECharacteristicParser
{
public:
	METRIC_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSTrainingStatus,
		BLEMetricNames::FMSTSWattControl
	);

	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override
	{
		uint8 Flags = Data[0];
		if (!(Flags & 0b1))
		{
			// Training status string not present
			return {};
		}
		if (Flags & 0b10)
		{
			// Extended string present
		}

		// Is not watt control change
		if (Data[1] != 0x0c)
		{
			return {};
		}

		int32 StringSize = Data.Num() - 2;
		FString String(StringSize, reinterpret_cast<const UTF8CHAR*>(&Data[2]));

		return { {BLEMetricNames::FMSTSWattControl, String} };
	}

	virtual void Reset() override { }
};