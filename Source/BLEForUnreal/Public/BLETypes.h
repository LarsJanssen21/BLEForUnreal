#pragma once

#include "BLETypes.generated.h"

USTRUCT(BlueprintType)
struct FBLEScanResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly)
	FString DeviceLocalName;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> AdvertisedServices;
};

UENUM(BlueprintType)
enum class EBLEDeviceCategory : uint8
{
	Any,
	HeartRate
};