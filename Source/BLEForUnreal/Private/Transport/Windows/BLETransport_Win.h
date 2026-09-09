#pragma once

#include "IBLETransport.h"

#pragma warning(push)
#pragma warning(disable: 4265)
#include "winrt/windows.devices.bluetooth.advertisement.h"
#pragma warning(pop)

using namespace winrt;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

class BLETransportWindows : public IBLETransport
{
public:
	BLETransportWindows();
	virtual ~BLETransportWindows() override;

	virtual void StartScan() override;
	virtual void StopScan() override;

private:
	void OnAdvertisementReceived(
		BluetoothLEAdvertisementWatcher const& Sender,
		BluetoothLEAdvertisementReceivedEventArgs const& args
	);

private:
	BluetoothLEAdvertisementWatcher AdvertisementWatcher;
	event_token AdvertisementReceivedToken;
};