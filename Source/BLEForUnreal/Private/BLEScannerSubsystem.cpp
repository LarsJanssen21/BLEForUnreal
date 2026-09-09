

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

	Transport->OnDeviceFound.BindUObject(this, &UBLEScannerSubsystem::HandleTransportDeviceFound);
}

void UBLEScannerSubsystem::Deinitialize()
{
	
}

void UBLEScannerSubsystem::HandleTransportDeviceFound(const FBLEScanResult& result)
{
	// Transport callbacks are guaranteed to be marshaled onto the game thread
	// by IBLETransport itself, so it's safe to broadcast directly here
	OnDeviceDiscovered.Broadcast(result);
}
