#pragma once

#include "CoreMinimal.h"

#include "BLEWriteRequest.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWriteRequestCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWriteIndicationReceived, uint8, ReturnCode);

UCLASS(BlueprintType)
class UBLEWriteRequest : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnWriteRequestCompleted OnWriteRequestCompleted;

	UPROPERTY(BlueprintAssignable)
	FOnWriteIndicationReceived OnWriteIndicationReceived;

	bool IsStale() const { return bRequestCompletedCalled && bWriteIndicationCalled; }

protected:
	friend class UBLEDevice;

	FName WriteName;

	bool bRequestCompletedCalled = false;
	bool bWriteIndicationCalled = false;
};