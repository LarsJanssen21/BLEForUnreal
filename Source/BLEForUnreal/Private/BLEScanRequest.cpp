#include "BLEScanRequest.h"

#include "BLEScannerSubsystem.h"

#include "BLEHelpers.h"
#include "BLEIDs.h"

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
				FString ServiceGUID = BLEHelpers::ShortServiceUUIDToGUIDString(static_cast<uint16>(BLEServiceID::HeartRateService));
				if (service.Equals(ServiceGUID, ESearchCase::IgnoreCase))
				{
					return true;
				}
				break;
			}
		}
	}

	return false;
}