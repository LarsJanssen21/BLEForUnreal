#include "BLEScanRequest.h"

#include "BLEScannerSubsystem.h"

#include "BLEUuid.h"
#include "BLEGattUuids.h"

void UBLEScanRequest::StopScan()
{
	if (UBLEScannerSubsystem* Subsystem = OwningSubsystem.Get())
	{
		Subsystem->UnregisterScanRequest(this);
		IsMarkedStale = true;
	}
}

bool UBLEScanRequest::MatchesFilter(const FBLEScanResult& Result) const
{
	if (Category == EBLEDeviceCategory::Any)
	{
		return true;
	}

	for (const auto& service : Result.AdvertisedServices)
	{
		switch (Category)
		{
			case EBLEDeviceCategory::HeartRate:
			{
				if (BLEUuid::AreEqual(BLEGattUuids::HeartRateService, service))
				{
					return true;
				}
				break;
			}
			case EBLEDeviceCategory::FitnessMachine:
			{
				if (service == BLEGattUuids::FitnessMachineService)
				{
					return true;
				}
				break;
			}
		}
	}

	return false;
}