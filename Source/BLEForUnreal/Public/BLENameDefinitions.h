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
	const FName HeartRateBpm = FName("heartrate.bpm"); // float-field


	/*
	 * Fitness machine service specification
	 *
	*/

	// Training status
	const FName FMSTSWattControl = FName("fms.ts.wattcontrol");

	// Fitness machine status
	const FName FMSStatusTargetPowerChanged = FName("fms.status.targetpowerchanged");

	// Indoor bike data
	const FName FMSIndoorBikeInstantaneousSpeed = FName("fms.indoorbikedata.instantaneousspeed");
	const FName FMSIndoorBikeAverageSpeed = FName("fms.indoorbikedata.averagespeed");
	const FName FMSIndoorBikeInstantaneousCadence = FName("fms.indoorbikedata.instantaneouscadence");
	const FName FMSIndoorBikeAverageCadence = FName("fms.indoorbikedata.averagecadence");
	const FName FMSIndoorBikeTotalDistance = FName("fms.indoorbikedata.totaldistance");
	const FName FMSIndoorBikeResistanceLevel = FName("fms.indoorbikedata.resistancelevel");
	const FName FMSIndoorBikeInstantaneousPower = FName("fms.indoorbikedata.instantaneouspower");
	const FName FMSIndoorBikeAveragePower = FName("fms.indoorbikedata.averagepower");
	const FName FMSIndoorBikeExpendedEnergy = FName("fms.indoorbikedata.expendedenergy");
	const FName FMSIndoorBikeHeartRate = FName("fms.indoorbikedata.heartrate");
	const FName FMSIndoorBikeMetabolicEquivalent = FName("fms.indoorbikedata.metabolicequivalent");
	const FName FMSIndoorBikeElapsedTime = FName("fms.indoorbikedata.elapsedtime");
	const FName FMSIndoorBikeRemainingTime = FName("fms.indoorbikedata.remainingtime");
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

	/*
	 * Fitness machine service specification
	 *
	*/
	const FName FMS_SupportedPowerRange = FName("fms.ranges.power");
	const FName FMS_SupportedSpeedRange = FName("fms.ranges.speed");
	const FName FMS_SupportedInclinationRange = FName("fms.ranges.inclination");
}

namespace BLEWriteNames {

	/*
	 * Heart Rate Service Specification
	 * https://www.bluetooth.com/specifications/specs/heart-rate-service-1-0/
	*/
	const FName HeartRateControlPoint = FName("heartrate.controlpoint");


	/*
	 * Fitness machine service specification
	 *
	*/
	// FMS Control point
	const FName FMSCP_RequestControl = FName("fms.cp.requestcontrol");
	const FName FMSCP_TargetPower = FName("fms.cp.targetpower");
}