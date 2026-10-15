#include "CoreMinimal.h"

class IBLECharacteristicParser;

TArray<TUniquePtr<IBLECharacteristicParser>> CreateParsersForServices(
	const TArray<FString>& DiscoveredServiceUuids, const TArray<FString>& DiscoveredCharacteristicUuids);