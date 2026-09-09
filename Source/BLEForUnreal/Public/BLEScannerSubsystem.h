

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "BLEScannerSubsystem.generated.h"

class IBLETransport;

USTRUCT(BlueprintType)
struct FBLEScanResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString DeviceId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONBLEDeviceDiscovered, const FBLEScanResult&, ScanResult);

/**
 * 
 */
UCLASS()
class BLEFORUNREAL_API UBLEScannerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UBLEScannerSubsystem();
	UBLEScannerSubsystem(FVTableHelper& Helper);
	virtual ~UBLEScannerSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UPROPERTY(BlueprintAssignable, category="BLE")
	FONBLEDeviceDiscovered OnDeviceDiscovered;

private:
	void HandleTransportDeviceFound(const FBLEScanResult& result);

private:
	TUniquePtr<IBLETransport> Transport;
};
