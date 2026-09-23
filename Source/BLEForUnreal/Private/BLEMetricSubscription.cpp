#include "BLEMetricSubscription.h"

#include "BLEDevice.h"

void UBLEMetricSubscription::Unsubscribe()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;

	if (UBLEDevice* Device = OwningDevice.Get())
	{
		Device->RemoveSubscription(this);
	}
}