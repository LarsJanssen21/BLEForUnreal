
#include "Parser/BLEParserFactory.h"

#include "Parser/IBLECharacteristicParser.h"
#include "BLEGattUuids.h"
#include "BLEUuid.h"

#include "Parser/HeartRateParser.h"

TArray<TUniquePtr<IBLECharacteristicParser>> CreateParsersForServices(
	const TArray<FString>& DiscoveredServiceUuids)
{
	TArray<TUniquePtr<IBLECharacteristicParser>> Result;

	TSet<FString> NormalizedServiceUuids;
	for (const auto& ServiceUuid : DiscoveredServiceUuids)
	{
		FString Normalized = BLEUuid::Normalize(ServiceUuid);
		if (!Normalized.IsEmpty())
		{
			NormalizedServiceUuids.Add(Normalized);
		}
	}

	if (NormalizedServiceUuids.Contains(BLEUuid::Normalize(BLEGattUuids::HeartRateService)))
	{
		Result.Add(MakeUnique<FHeartRateParser>());
	}

	return Result;
}