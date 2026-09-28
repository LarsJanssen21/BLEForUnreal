#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FManufacturerStringParser : public IBLECharacteristicParser
{
public:
	READ_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::DeviceInformationService,
		BLEGattUuids::ManufacturerNameString,
		BLEReadNames::ManufacturerName
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		const UTF8CHAR* str = reinterpret_cast<const UTF8CHAR*>(Data.GetData());
		FString Manufacturer = FString(str);

		return { { BLEReadNames::ManufacturerName, Manufacturer } };
	}

	virtual void Reset() override { }
};

class FModelNumberStringParser : public IBLECharacteristicParser
{
public:
	READ_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::DeviceInformationService,
		BLEGattUuids::ModelNumberString,
		BLEReadNames::ModelNumber
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		return {};
	}

	virtual void Reset() override { }
};