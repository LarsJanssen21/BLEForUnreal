#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FFMSSupportedSpeedRange : public IBLECharacteristicParser
{
public:
	READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSSupportedSpeedRange,
		BLEReadNames::FMS_SupportedSpeedRange
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		return {
			{
				BLEReadNames::FMS_SupportedSpeedRange,
				{
					static_cast<float>(*reinterpret_cast<const uint16*>(&Data[0])), // Minimum speed
					static_cast<float>(*reinterpret_cast<const uint16*>(&Data[2])), // Maximum speed
					static_cast<float>(*reinterpret_cast<const uint16*>(&Data[4]))  // Minimum increment value
				}
			}
		};
	}

	virtual void Reset() {};
};

class FFMSSupportedInclinationRange : public IBLECharacteristicParser
{
public:
	READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSSupportedInclinationRange,
		BLEReadNames::FMS_SupportedInclinationRange
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		return {
			{
				BLEReadNames::FMS_SupportedInclinationRange,
				{
					static_cast<float>(*reinterpret_cast<const int16*>(&Data[0])), // Minimum incline
					static_cast<float>(*reinterpret_cast<const int16*>(&Data[2])), // Maximum incline
					static_cast<float>(*reinterpret_cast<const uint16*>(&Data[4])) // Minimum increment value
				}
			}
		};
	}

	virtual void Reset() {};
};

class FFMSSupportedPowerRange : public IBLECharacteristicParser
{
public:
	READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSSupportedPowerRange,
		BLEReadNames::FMS_SupportedPowerRange
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		return { 
			{
				BLEReadNames::FMS_SupportedPowerRange,
				{
					static_cast<float>(*reinterpret_cast<const int16*>(&Data[0])), // Minimum power
					static_cast<float>(*reinterpret_cast<const int16*>(&Data[2])), // Maximum power
					static_cast<float>(*reinterpret_cast<const uint16*>(&Data[4])) // Minimum increment
				} 
			} 
		};
	}

	virtual void Reset() {};
};

void AddFMSSupportedRangeParsers(TArray<TUniquePtr<IBLECharacteristicParser>>& Parsers)
{
	Parsers.Add(MakeUnique<FFMSSupportedSpeedRange>());
	Parsers.Add(MakeUnique<FFMSSupportedInclinationRange>());
	Parsers.Add(MakeUnique<FFMSSupportedPowerRange>());
}