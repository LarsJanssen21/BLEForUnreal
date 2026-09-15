#include "BLETransport_Win.h"

#include "BLETypes.h"
#include "BLEScannerSubsystem.h"

#include "winrt/base.h"
#include "winrt/windows.foundation.h"
#include "winrt/windows.foundation.collections.h"
#include "winrt/windows.devices.bluetooth.h"

#include "CoreMinimal.h"

BLETransportWindows::BLETransportWindows()
{

}

BLETransportWindows::~BLETransportWindows()
{

}

void BLETransportWindows::StartScan()
{
	AdvertisementWatcher = BluetoothLEAdvertisementWatcher();
	AdvertisementWatcher.ScanningMode(BluetoothLEScanningMode::Active);

	AdvertisementReceivedToken = AdvertisementWatcher.Received([this](BluetoothLEAdvertisementWatcher const& Sender, BluetoothLEAdvertisementReceivedEventArgs const& Args)
		{
			OnAdvertisementReceived(Sender, Args);
		});

	AdvertisementWatcher.Start();
}

void BLETransportWindows::StopScan()
{
	AdvertisementWatcher.Stop();
}

void BLETransportWindows::ConnectToDevice(const FString& DeviceId)
{
	uint64_t ParsedAddress = FCString::Strtoui64(*DeviceId, nullptr, 16);
	
	BluetoothLEDevice::FromBluetoothAddressAsync(ParsedAddress).Completed([this](auto const& Sender, AsyncStatus Status)
		{
			// TODO: Handle AsyncStatus 
			BluetoothLEDevice Device = Sender.GetResults();
			OnConnectionComplete(Device);
		});
}

void BLETransportWindows::OnAdvertisementReceived(
	BluetoothLEAdvertisementWatcher const& Sender,
	BluetoothLEAdvertisementReceivedEventArgs const& Args)
{
	FBLEScanResult Result;
	Result.DeviceId = FormatDeviceId(Args.BluetoothAddress());
	Result.DeviceLocalName = winrt::to_hstring(Args.Advertisement().LocalName()).c_str();

	for (const winrt::guid& uuid : Args.Advertisement().ServiceUuids())
	{
		Result.AdvertisedServices.Add(FString(winrt::to_hstring(uuid).c_str()));
	}

	// Marshal back to the game thread
	AsyncTask(ENamedThreads::GameThread, 
		[this, Result = MoveTemp(Result)]
		{
			OnDeviceFound.ExecuteIfBound(Result);
		}
	);
}

void BLETransportWindows::OnConnectionComplete(BluetoothLEDevice device)
{
	FString DeviceId = {};
	bool bSuccess = false;

	DeviceId = FormatDeviceId(device.BluetoothAddress());

	for (const auto& service : device.GattServices())
	{
		for (const auto& characteristic : service.GetAllCharacteristics())
		{
			FString str = DeviceId;
			str += winrt::to_hstring(service.Uuid()).c_str();
			str += winrt::to_hstring(characteristic.Uuid()).c_str();

			CharacteristicsRegistry.Add({ str, characteristic });
		}
	}
	
	// Marshal back to the game thread
	AsyncTask(ENamedThreads::GameThread,
		[this, DeviceId = MoveTemp(DeviceId), bSuccess = MoveTemp(bSuccess)]
		{
			OnTransportConnectComplete.ExecuteIfBound(DeviceId, bSuccess);
		});
}