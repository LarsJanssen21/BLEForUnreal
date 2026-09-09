

#include "BLEScannerSubsystem.h"

#include "IBLETransport.h"

UBLEScannerSubsystem::UBLEScannerSubsystem() = default;

UBLEScannerSubsystem::UBLEScannerSubsystem(FVTableHelper& Helper)
	:Super(Helper)
{

}

UBLEScannerSubsystem::~UBLEScannerSubsystem() = default;

void UBLEScannerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Transport = IBLETransport::CreatePlatformTransport();
}

void UBLEScannerSubsystem::Deinitialize()
{

}
