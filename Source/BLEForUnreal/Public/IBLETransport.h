#pragma once

#include "CoreMinimal.h"

struct FBLEScanResult;

DECLARE_DELEGATE_OneParam(FONBLETransportDeviceFound, const FBLEScanResult& /*Result*/);
DECLARE_DELEGATE_TwoParams(FONBLEConnectComplete, const FString& DeviceId, bool bSuccess);

class IBLETransport
{
public:
	virtual ~IBLETransport() = default;

	static TUniquePtr<IBLETransport> CreatePlatformTransport();

	virtual void StartScan() = 0;
	virtual void StopScan() = 0;

	virtual void ConnectToDevice(const FString& DeviceId) = 0;

	FONBLETransportDeviceFound OnDeviceFound;
	FONBLEConnectComplete OnConnectComplete;
};