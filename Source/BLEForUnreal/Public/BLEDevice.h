

#pragma once

#include "CoreMinimal.h"

#include "IBLETransport.h"

#include "BLEDevice.generated.h"


class IBLECharacteristicParser;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMetricUpdated, FName, MetricName, float, Value);

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
	bool SubscribeToMetric(FName MetricName);

	UFUNCTION(BlueprintCallable, Category="BLE")
	void UnsubscribeFromMetric(FName MetricName);

	UPROPERTY(BlueprintAssignable, Category="BLE")
	FOnMetricUpdated OnMetricUpdated;

	/*
	 * Used by UBLEScannerSubsystem to pass through updates on subscribed characteristics
	*/
	void HandleCharacteristicData(const FString& CharacteristicUuid, const FBLECharacteristicData& Data);

private:
	UPROPERTY()
	FString DeviceId;

	/*	Never owned here - Valid only as long as the owning Subsystem is alive	*/
	IBLETransport* Transport = nullptr;

	// Sole owner of every parser this device could use
	TArray<TUniquePtr<IBLECharacteristicParser>> OwnedParsers;

	TMap<FName, IBLECharacteristicParser*> AvailableParsers; // Metric name --> Parser
	TMap<FString, IBLECharacteristicParser*> ActiveParsers; // Characteristic UUID -> Parser
	TMap<FString, int32_t> ParserSubscriberCount; // Characteristic UUID -> Subscriber Count

	TMap<FName, float> LatestMetricValues;
};
