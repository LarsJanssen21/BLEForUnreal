

#pragma once

#include "CoreMinimal.h"

#include "IBLETransport.h"

#include "BLEDevice.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class BLEFORUNREAL_API UBLEDevice : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(const FString& InDeviceId, IBLETransport* InTransport);

	UFUNCTION(BlueprintPure, category="BLE")
	const FString& GetDeviceId() const { return DeviceId; }

private:
	UPROPERTY()
	FString DeviceId;

	/*	Never owned here - Valid only as long as the owning Subsystem is alive	*/
	IBLETransport* Transport = nullptr;
};
