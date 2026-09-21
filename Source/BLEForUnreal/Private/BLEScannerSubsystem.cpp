

#include "BLEScannerSubsystem.h"

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
		Transport->OnCharacteristicUpdated.BindUObject(this, &UBLEScannerSubsystem::HandleTransportCharacteristicUpdated);
	}
}

void UBLEScannerSubsystem::Deinitialize()
{
	for (UBLEScanRequest* Request : ScanRequests)
	{
		UnregisterScanRequest(Request);
	}

	StopScan();

	Transport.Reset();

	Super::Deinitialize();
}

UBLEScanRequest* UBLEScannerSubsystem::StartFilteredScan(EBLEDeviceCategory Category)
{
	UBLEScanRequest* Request = NewObject<UBLEScanRequest>(this);
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

void UBLEScannerSubsystem::HandleTransportConnectionComplete(const FString& DeviceId, 
	const TArray<FString>& DiscoveredServiceUuids,
	bool bSuccess)
{
	UBLEDevice* Device = nullptr;

	if (bSuccess)
	{
		Device = NewObject<UBLEDevice>(this);
		Device->Initialize(DeviceId, Transport.Get(), DiscoveredServiceUuids);
		ConnectedDevices.Add(Device);
	}

	OnDeviceConnected.Broadcast(Device, bSuccess);
}

void UBLEScannerSubsystem::HandleTransportCharacteristicUpdated(const FString& DeviceId,
	const FString& CharacteristicUuid,
	const FBLECharacteristicData& Data)
{
	// OnCharacteristicUpdated is transport-wide — every connected device's
	// notifications arrive through this ONE bound callback, regardless of
	// which physical peripheral they came from. Find the UBLEDevice this
	// packet actually belongs to before handing the bytes off.
	for (UBLEDevice* Device : ConnectedDevices)
	{
		if (Device && Device->GetDeviceId() == DeviceId)
		{
			Device->HandleCharacteristicData(CharacteristicUuid, Data);
		}
	}
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