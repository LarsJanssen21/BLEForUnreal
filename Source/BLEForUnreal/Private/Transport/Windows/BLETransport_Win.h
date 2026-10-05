#pragma once

#include "IBLETransport.h"
#include "Misc/Optional.h"

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
	virtual void Disconnect(const FString& DeviceId) override;

	virtual void SubscribeToCharacteristic(const FString& DeviceId,
		const FString& ServiceUuid, const FString& CharUuid) override;

	virtual void UnsubscribeFromCharacteristic(const FString& DeviceId,
		const FString& ServiceUuid, const FString& CharUuid) override;

	virtual void ReadCharacteristic(const FString& DeviceId,
		const FString& ServiceUuid, const FString& CharUuid) override;

	virtual void WriteCharacteristic(const FString& DeviceId,
		const FString& ServiceUuid, const FString& CharUuid, FBLECharacteristicData InData) override;

private:
	inline FString FormatDeviceId(uint64_t BluetoothAddress)
	{
		return FString::Printf(TEXT("%012llx"), BluetoothAddress);
	}

	struct FConnectedDeviceEntry
	{
		BluetoothLEDevice Device{ nullptr };
		TMap<FString /*Normalize device UUID entry*/, GattDeviceService> Services;
	};

private:
	FString ComposeCharacteristicCacheKey(FString DeviceId, FString NormalizedCharUuid);

	bool CharacteristicViableCheck(FString& OutCacheKey,
		GattCharacteristic*& OutCharacteristic,
		const FString& DeviceId, const FString& CharUuid
	);

	void DiscoverServicesAndComplete(BluetoothLEDevice Device, const FString& DeviceId);

	void EnableNotifications(const FString& DeviceId, 
		const FString& CharacteristicUuid, GattCharacteristic Characteristic);

	void ExecuteReadRequest(const FString& DeviceId,
		const FString& CharacteristicUuid, GattCharacteristic Characteristic);

	void ExecuteWriteRequest(const FString& DeviceId,
		const FString& CharacteristicUuid, GattCharacteristic Characteristic,
		FBLECharacteristicData InData);

private:
	/*	Callbacks	*/
	void OnAdvertisementReceived(
		BluetoothLEAdvertisementWatcher const& Sender,
		BluetoothLEAdvertisementReceivedEventArgs const& args
	);

private:
	struct FReadRequestEntry
	{
		FString DeviceId;
		FString CharacteristicUuid;
		GattCharacteristic Characteristic;
	};

	struct FWriteRequestEntry
	{
		FString DeviceId;
		FString CharacteristicUuid;
		GattCharacteristic Characteristic;
		FBLECharacteristicData Payload;

	};

	UPROPERTY()
	TMap<FString /*DeviceId*/, FConnectedDeviceEntry> ConnectedDevices;

	TMap<FString /*Cache key*/, GattCharacteristic> CachedCharacteristics;
	TMap<FString /*Cache key*/, winrt::event_token> NotificationTokens;


	FString OpenReadRequest = {}; /*Cache key*/
	TArray<FString /*Cache key*/> QueuedReadRequests = {};
	TMap<FString, FReadRequestEntry> CachekeyToReadRequests;

	TOptional<FWriteRequestEntry> OpenWriteRequest;
	TArray<FWriteRequestEntry> QueuedWriteRequests = {};

	BluetoothLEAdvertisementWatcher AdvertisementWatcher;
	event_token AdvertisementReceivedToken;

	TSet<FString> PendingConnections;

	const uint32_t GattStatusReconnectAttempts = 3;
	const uint32_t GattStatusReconnectDelayMs = 1000;
};