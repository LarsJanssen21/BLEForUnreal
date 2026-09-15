#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Ticker.h"

#include "BLEDevice.h"
#include "BLETypes.h"

#include "BLEScannerSubsystem.generated.h"

class IBLETransport;
class UBLEScanRequest;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBLEDeviceConnected, UBLEDevice*, ConnectedDevice, bool, bSuccess);

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

	UFUNCTION(BLueprintCallable, Category="BLE")
	UBLEScanRequest* StartFilteredScan(EBLEDeviceCategory Category);

	UPROPERTY(BlueprintAssignable, category="BLE")
	FOnBLEDeviceConnected OnDeviceConnected;

public:
	void UnregisterScanRequest(UBLEScanRequest* Request);

private:
	void StartScan();
	
	void StopScan();

	void HandleTransportDeviceFound(const FBLEScanResult& result);
private:
	TUniquePtr<IBLETransport> Transport;

	UPROPERTY()
	TArray<UBLEDevice*> ConnectedDevices;

	bool bIsScanning = false;
	TArray<UBLEScanRequest*> ScanRequests;
};
