#pragma once

#include "CoreMinimal.h"

class BLEHelpers
{
public:
	static FString ShortServiceUUIDToGUIDString(uint16 ServiceUuid)
	{
		return FString::Printf(TEXT("{0000%04X-0000-1000-8000-00805F9B34FB}"), ServiceUuid);
	}
};
