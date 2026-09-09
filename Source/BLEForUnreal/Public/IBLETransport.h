#pragma once

#include "CoreMinimal.h"

class IBLETransport
{
public:
	virtual ~IBLETransport() = default;

	static TUniquePtr<IBLETransport> CreatePlatformTransport();

	virtual void StartScan() = 0;
	virtual void StopScan() = 0;
};