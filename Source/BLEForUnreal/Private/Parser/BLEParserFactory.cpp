
#include "Parser/BLEParserFactory.h"

#include "Parser/IBLECharacteristicParser.h"
#include "BLEGattUuids.h"
#include "BLEUuid.h"

#include "Parser/HeartRateParser.h"

TArray<TUniquePtr<IBLECharacteristicParser>> CreateParsersForServices(
	const TArray<FString>& DiscoveredServiceUuids)
{
	TArray<TUniquePtr<IBLECharacteristicParser>> Result;

	if (DiscoveredServiceUuids.Contains(BLEGattUuids::HeartRateService))
	{
		Result.Add(MakeUnique<FHeartRateParser>());
	}

	return Result;
}