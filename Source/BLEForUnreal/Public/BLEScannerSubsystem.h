

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Ticker.h"

#include "BLEScannerSubsystem.generated.h"

class IBLETransport;

USTRUCT(BlueprintType)
struct FBLEScanResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString DeviceId;

	UPROPERTY(BlueprintReadOnly)
	FString DeviceLocalName;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONBLEDeviceDiscovered, const FBLEScanResult&, ScanResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FONBLEScanTimeout);

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

	UFUNCTION(BlueprintCallable, Category="BLE")
	void StartScan(float TimeoutSeconds = 15.0f);

	UFUNCTION(BlueprintCallable, Category="BLE")
	void StopScan();

	UPROPERTY(BlueprintAssignable, category="BLE")
	FONBLEDeviceDiscovered OnDeviceDiscovered;

	UPROPERTY(BlueprintAssignable, category="BLE")
	FONBLEScanTimeout OnScanTimeout;

private:
	void HandleTransportDeviceFound(const FBLEScanResult& result);

	bool HandleTickerEvent(float deltaTime);
private:
	TUniquePtr<IBLETransport> Transport;

	bool bIsScanning = false;

	FTSTicker::FDelegateHandle TickerHandle;
};
