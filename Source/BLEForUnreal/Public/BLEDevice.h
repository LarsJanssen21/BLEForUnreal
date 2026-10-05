

#pragma once

#include "CoreMinimal.h"

#include "IBLETransport.h"

#include "BLEDevice.generated.h"


class IBLECharacteristicParser;
class UBLEMetricSubscription;
class UBLEReadRequest;
class UBLEWriteRequest;

USTRUCT()
struct FReadArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UBLEReadRequest>> Array;
};

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

	UFUNCTION(BlueprintPure, category="BLE")
	TArray<FName> GetAvailableReads() const;

	UFUNCTION(BlueprintPure, Category="BLE")
	TArray<FName> GetAvailableWrites() const;

	/*
	 * Subscribe to notifications on metric. Return value should be used.
	 * Handle is only marked as invalid and garbage collected when Unsubscribe is called
	 * on the returned object.
	*/
	UFUNCTION(BlueprintCallable, Category="BLE", Meta=(ReturnValueShouldBeUsed="true"))
	UBLEMetricSubscription* SubscribeToMetric(FName MetricName);

	/*
	 * Request a value read. Calls OnRequestCompleted when finished.
	 * Returned handle can be discarded after binding to OnRequestCompleted as
	 * handle is marked stale and garbage collected when OnRequestCompleted has been called
	*/
	UFUNCTION(BlueprintCallable, Category="BLE")
	UBLEReadRequest* RequestValueRead(FName ReadName);

	/*
	 * Submit a value write. Calls OnWriteRequestCompleted when finished.
	 * Returned handle can be discarded after binding to OnWriteRequestCompleted as
	 * handle is marked stale and garbage collected when OnWriteRequestCompleted has been called
	*/
	UFUNCTION(BlueprintCallable, Category="BLE")
	UBLEWriteRequest* SubmitValueWriteInt32(FName WriteName, int32 InValue);

	/*
	 * Used by UBLEScannerSubsystem to pass through updates on all charcateristic
	*/
	void HandleCharacteristicData(const FString& CharacteristicUuid, const FBLECharacteristicData& Data);

private:
	IBLECharacteristicParser* FindWriteParser(FName WriteName);

	friend class UBLEMetricSubscription;

	void RemoveSubscription(UBLEMetricSubscription* Subscription);

private:
	UPROPERTY()
	FString DeviceId;

	/*	Never owned here - Valid only as long as the owning Subsystem is alive	*/
	IBLETransport* Transport = nullptr;

	// Sole owner of every parser this device could use
	TArray<TUniquePtr<IBLECharacteristicParser>> OwnedParsers;

	/*
	 * Parsers that are alive (i.e. Actively processing Notifications, ready to respond to read requests)
	 * Metric(Notify) parsers are alive as long as they have subscribers
	 * Read parsers are alive for the lifetime of the device instance after they've been called once.
	*/
	TMap<FString, IBLECharacteristicParser*> ActiveParsers; // Characteristic UUID -> Parser

	TMap<FName, IBLECharacteristicParser*> AvailableParsersByMetric; // Metric name --> Parser
	TMap<FName, IBLECharacteristicParser*> AvailableParsersByRead; // Read name --> Parser
	TMap<FName, IBLECharacteristicParser*> AvailableParsersByWrite; // Write name --> Parser
	TMap<FString, int32_t> ParserSubscriberCount; // Characteristic UUID -> Subscriber Count

	UPROPERTY()
	TArray<TObjectPtr<UBLEMetricSubscription>> ActiveSubscriptions;
	UPROPERTY()
	TMap<FName, FReadArray> ReadRequests;

	TMap<FName, float> LatestMetricValues;
};
