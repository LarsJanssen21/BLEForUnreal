#include "BLEScanRequest.h"

#include "BLEScannerSubsystem.h"

#include "BLEUuid.h"

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
				FString ServiceGUID = BLEUuid::Normalize(BLEUuid::ExpandShortUuidString(BLEUuid::HeartRateServiceUUID));
				if (BLEUuid::AreEqual(ServiceGUID, service))
				{
					return true;
				}
				break;
			}
		}
	}

	return false;
}