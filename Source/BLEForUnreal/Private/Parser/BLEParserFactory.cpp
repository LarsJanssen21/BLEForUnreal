
#include "Parser/BLEParserFactory.h"

#include "Parser/IBLECharacteristicParser.h"
#include "BLEGattUuids.h"
#include "BLEUuid.h"

#include "Parser/HeartRateService/HeartRateParser.h"

#include "Parser/DeviceInformationService/DeviceInformationParser.h"

#include "Parser/GenericAccessProfile/GenericAccessProfileParser.h"

#include "Parser/FitnessMachineService/FMSControlPoint.h"
#include "Parser/FitnessMachineService/FMSTrainingStatus.h"
#include "Parser/FitnessMachineService/FMSStatus.h"
#include "Parser/FitnessMachineService/FMSRangeSupport.h"

#define ADD_PARSER_IF_SUPPORTED(CharUuid, ParserClassName) \
if(DiscoveredCharacteristicUuids.Contains(CharUuid)) Result.Add(MakeUnique<ParserClassName>());

TArray<TUniquePtr<IBLECharacteristicParser>> CreateParsersForServices(
	const TArray<FString>& DiscoveredServiceUuids, const TArray<FString>& DiscoveredCharacteristicUuids)
{
	TArray<TUniquePtr<IBLECharacteristicParser>> Result;

	/*
	 * Generic access profile 
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::GAPService))
	{
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::DeviceName, FDeviceNameParser)
	}

	/*
	 * Heart rate service
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::HeartRateService))
	{
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::HeartRateMeasurement, FHeartRateParser)
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::HeartRateControlPoint, FHeartRateControlPoint);
	}

	/*
	 * Device information service
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::DeviceInformationService))
	{
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::ManufacturerNameString, FManufacturerStringParser);
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::ModelNumberString, FModelNumberStringParser);
	}

	/*
	 * Fitness machine service
	*/
	if (DiscoveredServiceUuids.Contains(BLEGattUuids::FitnessMachineService))
	{
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSControlPoint, FFMSControlPointParser);
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSTrainingStatus, FFMSTrainingStatusParser);
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSStatus, FFMSStatusParser);

		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSSupportedSpeedRange, FFMSSupportedSpeedRange);
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSSupportedInclinationRange, FFMSSupportedInclinationRange);
		ADD_PARSER_IF_SUPPORTED(BLEGattUuids::FMSSupportedPowerRange, FFMSSupportedPowerRange);
	}

	return Result;
}