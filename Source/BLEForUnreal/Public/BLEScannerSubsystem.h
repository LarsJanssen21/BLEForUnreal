#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Containers/Ticker.h"

#include "BLEDevice.h"
#include "BLETypes.h"
#include "IBLETransport.h"

#include "BLEScannerSubsystem.generated.h"

class IBLETransport;
class UBLEScanRequest;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBLEDeviceConnected, UBLEDevice*, ConnectedDevice, bool, bSuccess);

/**
 * 
 */
UCLASS()
class BLEFORUNREAL_API UBLEScannerSubsystem : public UGameInstanceSubsystem
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

	UFUNCTION(BlueprintCallable, Category="BLE")
	void ConnectToDevice(const FString& DeviceId);

	UPROPERTY(BlueprintAssignable, category="BLE")
	FOnBLEDeviceConnected OnDeviceConnected;

public:
	void UnregisterScanRequest(UBLEScanRequest* Request);

	void HandleTransportDeviceFound(const FBLEScanResult& result);
	void HandleTransportConnectionComplete(const FString& DeviceId, 
		const TArray<FString>& DiscoveredServiceUuids, 
		bool bSuccess);
	void HandleTransportCharacteristicUpdated(const FString& DeviceId,
		const FString& CharacteristicUuid,
		const FBLECharacteristicData& Data);
private:
	void StartScan();
	void StopScan();

private:
	TUniquePtr<IBLETransport> Transport;

	UPROPERTY()
	TArray<UBLEDevice*> ConnectedDevices;

	bool bIsScanning = false;
	TArray<UBLEScanRequest*> ScanRequests;
};
