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
	UPROPERTY(BlueprintAssignable, Category="BLE")
	FOnMetricValueUpdated OnValueUpdated;

	UFUNCTION(BlueprintCallable, Category="BLE")
	void Unsubscribe();


	UFUNCTION(BlueprintPure, Category="BLE")
	FName GetMetricName() const { return MetricName; }

	UFUNCTION(BlueprintPure, Category="BLE")
	bool IsActive() const { return bIsActive; }


private:
	friend class UBLEDevice;

	FName MetricName;

	UPROPERTY()
	TWeakObjectPtr<UBLEDevice> OwningDevice;

	bool bIsActive = true;
};