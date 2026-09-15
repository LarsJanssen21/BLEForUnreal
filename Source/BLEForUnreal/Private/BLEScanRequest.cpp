#include "BLEScanRequest.h"

#include "BLEScannerSubsystem.h"

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

	return false;
}