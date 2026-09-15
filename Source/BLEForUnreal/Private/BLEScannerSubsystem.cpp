

#include "BLEScannerSubsystem.h"

#include "IBLETransport.h"
#include "BLEScanRequest.h"

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

UBLEScanRequest* UBLEScannerSubsystem::StartFilteredScan(EBLEDeviceCategory Category)
{
	UBLEScanRequest* Request = NewObject<UBLEScanRequest>();
	Request->Category = Category;
	Request->OwningSubsystem = this;
	Request->IsMarkedStale = false;

	ScanRequests.Add(Request);

	if (!bIsScanning)
	{
		StartScan();
	}

	return Request;
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
		ScanTimeoutTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UBLEScannerSubsystem::HandleScanTimeoutTickerEvent),
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

	FTSTicker::GetCoreTicker().RemoveTicker(ScanTimeoutTickerHandle);
}

void UBLEScannerSubsystem::UnregisterScanRequest(UBLEScanRequest* Request)
{
	ScanRequests.RemoveSingleSwap(Request);

	if (ScanRequests.Num() == 0)
	{
		StopScan();
	}
}

void UBLEScannerSubsystem::HandleTransportDeviceFound(const FBLEScanResult& Result)
{
	// Transport callbacks are guaranteed to be marshaled onto the game thread
	// by IBLETransport itself, so it's safe to broadcast directly here
	OnDeviceDiscovered.Broadcast(Result);

	for (UBLEScanRequest* Request : ScanRequests)
	{
		if (Request && Request->MatchesFilter(Result))
		{
			Request->OnDeviceDiscovered.Broadcast(Result);
		}
	}
}

bool UBLEScannerSubsystem::HandleScanTimeoutTickerEvent(float DeltaTime)
{
	StopScan();
	OnScanTimeout.Broadcast();

	return false; // Removes the ticker automatically from the FTSTicker, no need to call RemoveTicker()
}