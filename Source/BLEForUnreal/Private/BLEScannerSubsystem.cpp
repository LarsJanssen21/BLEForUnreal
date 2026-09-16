

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
		Transport->OnConnectComplete.BindUObject(this, &UBLEScannerSubsystem::HandleTransportConnectionComplete);
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

void UBLEScannerSubsystem::ConnectToDevice(const FString& DeviceId)
{
	if (Transport.IsValid())
	{
		Transport->ConnectToDevice(DeviceId);
	}
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
	for (UBLEScanRequest* Request : ScanRequests)
	{
		if (Request && Request->MatchesFilter(Result))
		{
			// Transport callbacks are guaranteed to be marshaled onto the game thread
			// by IBLETransport itself, so it's safe to broadcast directly here
			Request->OnDeviceDiscovered.Broadcast(Result);
		}
	}
}

void UBLEScannerSubsystem::HandleTransportConnectionComplete(const FString& DeviceId, bool bSuccess)
{
	
}

void UBLEScannerSubsystem::StartScan()
{
	if (!Transport.IsValid() || bIsScanning)
	{
		return;
	}

	bIsScanning = true;
	Transport->StartScan();
}

void UBLEScannerSubsystem::StopScan()
{
	if (!Transport.IsValid() || !bIsScanning)
	{
		return;
	}

	bIsScanning = false;
	Transport->StopScan();
}