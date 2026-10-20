#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FFMSIndoorBikeData : public IBLECharacteristicParser
{
public:
	METRIC_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSIndoorBikeData,
		BLEMetricNames::FMSIndoorBikeInstantaneousSpeed, BLEMetricNames::FMSIndoorBikeAverageSpeed,
		BLEMetricNames::FMSIndoorBikeInstantaneousCadence, BLEMetricNames::FMSIndoorBikeAverageCadence,
		BLEMetricNames::FMSIndoorBikeTotalDistance, BLEMetricNames::FMSIndoorBikeResistanceLevel,
		BLEMetricNames::FMSIndoorBikeInstantaneousPower, BLEMetricNames::FMSIndoorBikeAveragePower,
		BLEMetricNames::FMSIndoorBikeExpendedEnergy, BLEMetricNames::FMSIndoorBikeHeartRate,
		BLEMetricNames::FMSIndoorBikeMetabolicEquivalent, BLEMetricNames::FMSIndoorBikeElapsedTime,
		BLEMetricNames::FMSIndoorBikeRemainingTime
	);

	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override
	{
		TArray<FBLEMetric> Metrics = {};

		uint16 Flags = *reinterpret_cast<const uint16*>(&Data[0]);

		uint16 Index = 2;

		if ((Flags & MoreDataFlag) == 0)
		{
			float InstSpeed = static_cast<float>(CastType<uint16>(Data, Index)) / 100.0f;

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeInstantaneousSpeed, InstSpeed });
		}
		if (Flags & AverageSpeedFlag)
		{
			float AverageSpeed = static_cast<float>(CastType<uint16>(Data, Index)) / 100.0f;

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeAverageSpeed, AverageSpeed });
		}
		if (Flags & InstantaneousCadenceFlag)
		{
			float InstCadence = static_cast<float>(CastType<uint16>(Data, Index)) / 2.0f;

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeInstantaneousCadence, InstCadence });
		}
		if (Flags & AverageCadenceFlag)
		{
			float AvgCadence = static_cast<float>(CastType<uint16>(Data, Index)) / 2.0f;

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeAverageCadence, AvgCadence });
		}
		if (Flags & TotalDistanceFlag)
		{
			// Actually a 24-bit integeer
			
			//uint32 TotalDist = (*reinterpret_cast<const uint32*>(&Data[Index])) & 0x00ffffffU;

			uint32 TotalDist = 0;
			memcpy(&TotalDist, &Data[Index], 3);

			Metrics.Add({
				BLEMetricNames::FMSIndoorBikeTotalDistance,
				static_cast<float>(TotalDist) / 1000.0f
				});

			Index += 3;
		}
		if (Flags & ResistanceLevelFlag)
		{
			float ResistanceLevel = static_cast<float>(CastType<uint8>(Data, Index));

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeResistanceLevel, ResistanceLevel });
		}
		if (Flags & InstantaneousPowerFlag)
		{
			float InstPower = static_cast<float>(CastType<int16>(Data, Index));

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeInstantaneousPower, InstPower });
		}
		if (Flags & AveragePowerFlag)
		{
			float AveragePower = static_cast<float>(CastType<int16>(Data, Index));

			Metrics.Add({ BLEMetricNames::FMSIndoorBikeAveragePower, AveragePower });
		}

		return Metrics;
	}

	virtual void Reset() {};

private:
	static const uint16 MoreDataFlag = (1 << 0);
	static const uint16 AverageSpeedFlag = (1 << 1);
	static const uint16 InstantaneousCadenceFlag = (1 << 2);
	static const uint16 AverageCadenceFlag = (1 << 3);
	static const uint16 TotalDistanceFlag = (1 << 4);
	static const uint16 ResistanceLevelFlag = (1 << 5);
	static const uint16 InstantaneousPowerFlag = (1 << 6);
	static const uint16 AveragePowerFlag = (1 << 7);
	static const uint16 ExpendedEnergyFlag = (1 << 8);
	static const uint16 HeartRateFlag = (1 << 9);
	static const uint16 MetabolicEquivalentFlag = (1 << 10);
	static const uint16 ElapsedTimeFlag = (1 << 11);
	static const uint16 RemainingTimeFlag = (1 << 12);

	template<typename T>
	T CastType(const TArray<uint8>& Data, uint16& Index)
	{
		T Value = *reinterpret_cast<const T*>(&Data[Index]);
		Index += static_cast<uint16>(sizeof(T));
		return Value;
	}
};