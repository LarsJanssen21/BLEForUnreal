#pragma once

#include "CoreMinimal.h"

#include "BLEMetricSubscription.generated.h"

class UBLEDevice;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMetricValueUpdated, float, Value);

UCLASS(BlueprintType)
class BLEFORUNREAL_API UBLEMetricSubscription : public UObject
{
	GENERATED_BODY()

public:
	/*
	 * Can be bound by user to receive value from changed metric
	 * as soon as the device sends it
	*/
	UPROPERTY(BlueprintAssignable, Category="BLE")
	FOnMetricValueUpdated OnValueUpdated;

	/*
	 * Closes active connection to receiving notifications.
	 * Must be called before discarding objects.
	 * Is otherwise left dangling and delegate will still be fired
	*/
	UFUNCTION(BlueprintCallable, Category="BLE")
	void Unsubscribe();

	/*
	 * Returns name of currently bound 
	*/
	UFUNCTION(BlueprintPure, Category="BLE")
	FName GetMetricName() const { return MetricName; }

	/*
	 * Indicates whether the current subscription is active
	 * @return true if Subscription will broadcast to OnValueUpdated and cannot be safely discarded,
	 * false if Subscription is stale and can be safely discarded.
	*/
	UFUNCTION(BlueprintPure, Category="BLE")
	bool IsActive() const { return bIsActive; }


private:
	friend class UBLEDevice;

	FName MetricName;

	UPROPERTY()
	TWeakObjectPtr<UBLEDevice> OwningDevice;

	bool bIsActive = true;
};