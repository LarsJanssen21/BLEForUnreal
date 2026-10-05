#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FManufacturerStringParser : public IBLECharacteristicParser
{
public:
	READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::DeviceInformationService,
		BLEGattUuids::ManufacturerNameString,
		BLEReadNames::ManufacturerName
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		FString Manufacturer = FString(Data.Num(), reinterpret_cast<const UTF8CHAR*>(Data.GetData()));

		return { { BLEReadNames::ManufacturerName, Manufacturer } };
	}

	virtual void Reset() override { }
};

class FModelNumberStringParser : public IBLECharacteristicParser
{
public:
	READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::DeviceInformationService,
		BLEGattUuids::ModelNumberString,
		BLEReadNames::ModelNumber
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		FString ModelNumber = FString(Data.Num(), reinterpret_cast<const UTF8CHAR*>(Data.GetData()));

		return { { BLEReadNames::ModelNumber, ModelNumber } };
	}

	virtual void Reset() override { }
};