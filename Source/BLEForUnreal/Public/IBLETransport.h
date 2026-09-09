#pragma once

#include "CoreMinimal.h"

struct FBLEScanResult;

DECLARE_DELEGATE_OneParam(FONBLETransportDeviceFound, const FBLEScanResult& /*Result*/);

class IBLETransport
{
public:
	virtual ~IBLETransport() = default;

	static TUniquePtr<IBLETransport> CreatePlatformTransport();

	virtual void StartScan() = 0;
	virtual void StopScan() = 0;

	FONBLETransportDeviceFound OnDeviceFound;
};