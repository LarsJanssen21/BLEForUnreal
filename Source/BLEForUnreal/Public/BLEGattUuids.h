#pragma once

#include "CoreMinimal.h"

/*	
 * GATT UUIDs as defined in the 
 * Bluetooth Assigned Numbers Specification 
 * (https://www.bluetooth.com/specifications/assigned-numbers/)
 * These need to fully normalized:
 *  - 128 bits
 *  - Lowercase
 *  - No braces (so no "{xxxx-xxxx}")
 *  - The only shape ever compared internally
 */

namespace BLEGattUuids {

	const FString HeartRateService = FString(TEXT("0000180d-0000-1000-8000-00805f9b34fb"));
	const FString HeartRateMeasurement = FString(TEXT("00002a37-0000-1000-8000-00805f9b34fb"));

}