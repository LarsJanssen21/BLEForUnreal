#include "BLETransport_Win.h"

#include "BLEScannerSubsystem.h"

#include "winrt/base.h"
#include "winrt/windows.foundation.h"
#include "winrt/windows.foundation.collections.h"
#include "winrt/windows.devices.bluetooth.h"


BLETransportWindows::BLETransportWindows()
{
	winrt::init_apartment(winrt::apartment_type::multi_threaded);
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

void BLETransportWindows::OnAdvertisementReceived(
	BluetoothLEAdvertisementWatcher const& Sender,
	BluetoothLEAdvertisementReceivedEventArgs const& Args)
{

}