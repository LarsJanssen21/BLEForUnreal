
#include "Parser/BLEParserFactory.h"

#include "Parser/IBLECharacteristicParser.h"
#include "BLEGattUuids.h"
#include "BLEUuid.h"

#include "Parser/GenericAccessProfile/GenericAccessProfileParser.h"
#include "Parser/HeartRateService/HeartRateParser.h"
#include "Parser/DeviceInformationService/DeviceInformationParser.h"

TArray<TUniquePtr<IBLECharacteristicParser>> CreateParsersForServices(
	const TArray<FString>& DiscoveredServiceUuids)
{
	TArray<TUniquePtr<IBLECharacteristicParser>> Result;

	/*
	 * Generic access profile 
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::GAPService))
	{
		Result.Add(MakeUnique<FDeviceNameParser>());
	}

	/*
	 * Heart rate service
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::HeartRateService))
	{
		Result.Add(MakeUnique<FHeartRateParser>());
		Result.Add(MakeUnique<FHeartRateControlPoint>());
	}

	/*
	 * Device information service
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::DeviceInformationService))
	{
		Result.Add(MakeUnique<FManufacturerStringParser>());
		Result.Add(MakeUnique<FModelNumberStringParser>());
	}

	return Result;
}