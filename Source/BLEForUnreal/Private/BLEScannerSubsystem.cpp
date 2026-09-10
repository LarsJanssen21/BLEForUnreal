

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
	Super::Initialize(Collection);

	Transport = IBLETransport::CreatePlatformTransport();

	if (Transport.IsValid())
	{
		Transport->OnDeviceFound.BindUObject(this, &UBLEScannerSubsystem::HandleTransportDeviceFound);
	}
}

void UBLEScannerSubsystem::Deinitialize()
{
	StopScan();

	Transport.Reset();

	Super::Deinitialize();
}

void UBLEScannerSubsystem::StartScan(float TimeoutSeconds)
{
	if (!Transport.IsValid() || bIsScanning)
	{
		return;
	}

	bIsScanning = true;
	Transport->StartScan();

	if (TimeoutSeconds > 0.0f)
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UBLEScannerSubsystem::HandleTickerEvent), 
			TimeoutSeconds
		);
	}
}

void UBLEScannerSubsystem::StopScan()
{
	if (!Transport.IsValid() || !bIsScanning)
	{
		return;
	}

	bIsScanning = false;
	Transport->StopScan();

	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
}

void UBLEScannerSubsystem::HandleTransportDeviceFound(const FBLEScanResult& Result)
{
	// Transport callbacks are guaranteed to be marshaled onto the game thread
	// by IBLETransport itself, so it's safe to broadcast directly here
	OnDeviceDiscovered.Broadcast(Result);
}

bool UBLEScannerSubsystem::HandleTickerEvent(float DeltaTime)
{
	StopScan();
	OnScanTimeout.Broadcast();

	return false; // Removes the ticker automatically from the FTSTicker, no need to call RemoveTicker()
}