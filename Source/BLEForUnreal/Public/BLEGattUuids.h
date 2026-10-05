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

	/*
	 * Generic access profile 
	*/
	const FString GAPService = FString(TEXT("00001800-0000-1000-8000-00805f9b34fb"));
	const FString DeviceName = FString(TEXT("00002a00-0000-1000-8000-00805f9b34fb"));

	/*
	 * Heart rate service
	*/
	const FString HeartRateService = FString(TEXT("0000180d-0000-1000-8000-00805f9b34fb"));
	const FString HeartRateMeasurement = FString(TEXT("00002a37-0000-1000-8000-00805f9b34fb"));
	const FString HeartRateControlPoint = FString(TEXT("00002a39-0000-1000-8000-00805f9b34fb"));


	/*
	 * Device information service
	*/
	const FString DeviceInformationService = FString(TEXT("0000180a-0000-1000-8000-00805f9b34fb"));
	const FString ManufacturerNameString = FString(TEXT("00002a29-0000-1000-8000-00805f9b34fb"));
	const FString ModelNumberString = FString(TEXT("00002a24-0000-1000-8000-00805f9b34fb"));

	/*
	 * Fitness Machine Service
	*/
	const FString FitnessMachineService = FString(TEXT("00001826-0000-1000-8000-00805f9b34fb"));

}