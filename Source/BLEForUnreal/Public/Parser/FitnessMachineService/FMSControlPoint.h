#pragma once

#include "CoreMinimal.h"
#include "Parser/IBLECharacteristicParser.h"

#include "BLEGattUuids.h"
#include "BLENameDefinitions.h"

class FFMSControlPointParser : public IBLECharacteristicParser
{
public:
	WRITE_ONLY_CHARACTERISTIC_IMPLEMENTATION(
		BLEGattUuids::FitnessMachineService,
		BLEGattUuids::FMSControlPoint,
		BLEWriteNames::FMSCP_RequestControl
	);

	virtual TArray<FBLEWrite> ParseWriteRequest(const TArray<uint8>& Data) override
	{
		// if first byte is not 0x80 this is not an op code return
		if (Data[0] != 0x80)
		{
			return {};
		}

		FName WriteName = OpCodeToWriteName(Data[1]);
		bool bSuccess = Data[2] == 0x01;

		return { {WriteName, bSuccess} };
	}

	virtual TArray<uint8> PrepareWriteRequestBuffer(FName WriteName, uint32 Value) override
	{
		uint8 OpCode = WriteNameToOpCode(WriteName);

		TArray<uint8> Buffer{};

		Buffer.Add(OpCode);

		switch (OpCode)
		{
			case 0x00:
			{
				break;
			}
		}

		return Buffer;
	}

	virtual void Reset() override { }

	virtual bool SupportsIndicate() const override { return true; }

private:
	FName OpCodeToWriteName(uint8 OpCode)
	{
		switch (OpCode)
		{
			case 0x00:
			{
				return BLEWriteNames::FMSCP_RequestControl;
				break;
			}
		}
	}

	uint8 WriteNameToOpCode(FName WriteName)
	{
		if (WriteName == BLEWriteNames::FMSCP_RequestControl)
		{
			return 0x00;
		}

		return 0xff;
	}
};