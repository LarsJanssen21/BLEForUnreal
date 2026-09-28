#pragma once

#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FDeviceNameParser : public IBLECharacteristicParser
{
public:
	READ_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::GAPService,
		BLEGattUuids::DeviceName,
		BLEReadNames::DeviceName
	);

	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override
	{
		const UTF8CHAR* str = reinterpret_cast<const UTF8CHAR*>(Data.GetData());
		FString DeviceName = FString(Data.Num(), str);

		return { { BLEReadNames::DeviceName, DeviceName } };
	}

	virtual void Reset() override {}
};