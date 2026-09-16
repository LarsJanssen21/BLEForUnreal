#pragma once

#include "IBLETransport.h"

#pragma warning(push)
#pragma warning(disable: 4265)
#include "winrt/windows.devices.bluetooth.advertisement.h"
#pragma warning(pop)

#include "winrt/windows.devices.bluetooth.genericattributeprofile.h"

using namespace winrt;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;

class BLETransportWindows : public IBLETransport
{
public:
	BLETransportWindows();
	virtual ~BLETransportWindows() override;

	virtual void StartScan() override;
	virtual void StopScan() override;

	virtual void ConnectToDevice(const FString& DeviceId) override;

private:
	inline FString FormatDeviceId(uint64_t BluetoothAddress)
	{
		return FString::Printf(TEXT("%012llx"), BluetoothAddress);
	}

	inline uint64_t ParseDeviceIdString(const FString& DeviceId)
	{
		return FCString::Strtoui64(*DeviceId, nullptr, 16);
	}

	struct FConnectedDeviceEntry
	{
		BluetoothLEDevice Device{ nullptr };
		TMap<FString /*Normalize device UUID entry*/, GattDeviceService> Services;
	};

private:
	void DiscoverServicesAndComplete(BluetoothLEDevice Device, const FString& DeviceId);

private:
	/*	Callbacks	*/
	void OnAdvertisementReceived(
		BluetoothLEAdvertisementWatcher const& Sender,
		BluetoothLEAdvertisementReceivedEventArgs const& args
	);

private:
	BluetoothLEAdvertisementWatcher AdvertisementWatcher;
	event_token AdvertisementReceivedToken;

	TMap<FString /*DeviceId*/, FConnectedDeviceEntry> ConnectedDevices;
};