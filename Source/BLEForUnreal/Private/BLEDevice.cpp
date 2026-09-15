
#include "BLEDevice.h"

void UBLEDevice::Initialize(const FString& InDeviceId, IBLETransport* InTransport)
{
	DeviceId = InDeviceId;

	Transport = InTransport;
}
