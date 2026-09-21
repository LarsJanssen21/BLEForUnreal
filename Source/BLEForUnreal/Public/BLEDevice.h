

#pragma once

#include "CoreMinimal.h"


#include "BLEDevice.generated.h"

class IBLETransport;
class IBLECharacteristicParser;

/**
 * 
 */
UCLASS(BlueprintType)
class BLEFORUNREAL_API UBLEDevice : public UObject
{
	GENERATED_BODY()

public:
	UBLEDevice();
	UBLEDevice(FVTableHelper& Helper);
	virtual ~UBLEDevice() override;

	void Initialize(const FString& InDeviceId, IBLETransport* InTransport, 
		const TArray<FString>& DiscoveredServiceUuids);

	UFUNCTION(BlueprintCallable, Category="BLE")
	void Disconnect();

	UFUNCTION(BlueprintPure, category="BLE")
	const FString& GetDeviceId() const { return DeviceId; }

	UFUNCTION(BlueprintPure, category="BLE")
	TArray<FName> GetAvailableMetrics() const;

	UFUNCTION(BlueprintCallable, Category="BLE")
	bool SubscribeToMetric(const FName& MetricName);

private:
	UPROPERTY()
	FString DeviceId;

	/*	Never owned here - Valid only as long as the owning Subsystem is alive	*/
	IBLETransport* Transport = nullptr;

	// Sole owner of every parser this device could use
	TArray<TUniquePtr<IBLECharacteristicParser>> OwnedParsers;

	TMap<FName, IBLECharacteristicParser*> AvailableParsers; // Metric name --> Parser
	TMap<FString, IBLECharacteristicParser*> ActiveParsers; // Characteristic UUID -> Parser
};
