#pragma once

#include "CoreMinimal.h"

#include "BLETypes.h"

#include "BLEScanRequest.generated.h"

class UBLEScannerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFilteredDeviceDiscovered, const FBLEScanResult&, Result);

/*
* Object that filters discovered devices by advertised service UUIDs
*/
UCLASS(BlueprintType)
class UBLEScanRequest : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnFilteredDeviceDiscovered OnDeviceDiscovered;

	/* 
	If true StopScan has been called on the object and scanning has ceased.
	To scan a new request has to be made through the UBLEScannersSubsytem
	*/
	UPROPERTY(BlueprintReadOnly)
	bool IsMarkedStale = true;

	/*
	Stops scan request.
	Removes request from owning UBLESubsystem registry and is marked stale for further use after calling
	*/
	UFUNCTION(BlueprintCallable, Category="BLE")
	void StopScan();

	bool MatchesFilter(const FBLEScanResult& Result) const;

private:
	friend class UBLEScannerSubsystem;

	EBLEDeviceCategory Category = EBLEDeviceCategory::Any;

	UPROPERTY()
	TWeakObjectPtr<UBLEScannerSubsystem> OwningSubsystem;
};
