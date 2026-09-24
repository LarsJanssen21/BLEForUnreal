#pragma once

#include "CoreMinimal.h"

#include "BLEReadRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadRequestCompleted, FString, String);

UCLASS(BlueprintType)
class BLEFORUNREAL_API UBLEReadRequest : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category="BLE")
	FOnReadRequestCompleted OnRequestCompleted;

	UFUNCTION(BlueprintPure, Category="BLE")
	bool IsActive() const { return bIsAlive; }

private:
	friend class UBLEDevice;

	bool bIsAlive = true;

};