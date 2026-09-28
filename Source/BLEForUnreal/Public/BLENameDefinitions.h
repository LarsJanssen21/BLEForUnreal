#pragma once

#include "CoreMinimal.h"

/*
 * This is where the names of all the different metrics(BLE-spec notifications), read-requests (BLE-spec read), and write-requests (BLE-spec write)
 * Are located, if developers add their own characteristic implementations they should state the full name of the characteristic 
 * as well as a link to the specific versioned specification that was used.
 * 
 * This file should be taken in full as a sheet for answering the following questions
 * - Which metrics are available?
 * - What value should I expect to return from my call
*/

namespace BLEMetricNames {

	/*
	 * Heart Rate Service Specification 
	 * https://www.bluetooth.com/specifications/specs/heart-rate-service-1-0/	
	*/
	const FName HeartRateBpm = FName("heartratebpm"); // float-field

}

namespace BLEReadNames {

	/*
	 * GAP Service 
	*/
	const FName DeviceName = FName("devicename");

	/*
	 *  Device Information Service Specification
	 * https://www.bluetooth.com/specifications/specs/device-information-service-1-1/
	*/
	const FName ManufacturerName = FName("manufacturername"); // string-field
	const FName ModelNumber = FName("modelnumber"); // string-field

}

namespace BLEWriteNames {

	// Reserved for future use

}