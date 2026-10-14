#pragma once

#include "BLETypes.generated.h"

UENUM(BlueprintType)
enum class EBLEScanResultSignalStrength : uint8
{
	Excellent,
	Good,
	Weak
};

USTRUCT(BlueprintType)
struct FBLEScanResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly)
	FString DeviceLocalName;

	UPROPERTY(BlueprintReadOnly)
	EBLEScanResultSignalStrength SignalStrength;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> AdvertisedServices; /* Canonical Normalize Service UUID*/
};

UENUM(BlueprintType)
enum class EBLEDeviceCategory : uint8
{
	Any,
	HeartRate,
	FitnessMachine
};

UENUM(BlueprintType)
enum class EBLEDeviceConnectionReturnCode : uint8
{
	Success,
	Unreachable,
	ProtocolError,
	AccessDenied
};