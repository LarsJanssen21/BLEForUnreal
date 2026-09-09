

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "BLEScannerSubsystem.generated.h"

class IBLETransport;

/**
 * 
 */
UCLASS()
class BLEFORUNREAL_API UBLEScannerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UBLEScannerSubsystem();
	UBLEScannerSubsystem(FVTableHelper& Helper);
	virtual ~UBLEScannerSubsystem() override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	TUniquePtr<IBLETransport> Transport;
};
