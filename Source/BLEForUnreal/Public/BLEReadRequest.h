#pragma once

#include "CoreMinimal.h"

#include "BLEReadRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadRequestCompleted, FString, String);

/*
 * Object returned from BLEDevice. alive for as long as the request callback has not been called
 * Safe to immediately release after OnRequestCompleted has been bound.
 * State can be queried through IsActive() method
*/
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