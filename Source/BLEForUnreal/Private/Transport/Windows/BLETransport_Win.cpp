#include "BLETransport_Win.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "BLETypes.h"
#include "BLEScannerSubsystem.h"
#include "BLEUuid.h"

#include "winrt/base.h"
#include "winrt/windows.foundation.h"
#include "winrt/windows.foundation.collections.h"
#include "winrt/windows.devices.bluetooth.h"
#include "winrt/windows.storage.streams.h"

#include "CoreMinimal.h"


BLETransportWindows::BLETransportWindows()
{

}

BLETransportWindows::~BLETransportWindows()
{
	for (const auto& [DeviceId, DeviceEntry] : ConnectedDevices)
	{
		DeviceEntry.Device.Close();
	}
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
	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (Entry)
	{
		TArray<FString> DiscoveredServiceUuids;
		Entry->Services.GenerateKeyArray(DiscoveredServiceUuids);

		AsyncTask(ENamedThreads::GameThread, [this, DeviceId, 
			DiscoveredServiceUuids = MoveTemp(DiscoveredServiceUuids)]()
			{
				OnConnectComplete.ExecuteIfBound(DeviceId, DiscoveredServiceUuids, true);
			}
		);

		return;
	}

	if (PendingConnections.Contains(DeviceId))
	{
		// Connect request for this device is already in flight.
		// Original request will still fire OnConnectComplete 
		return;
	}

	PendingConnections.Add(DeviceId);

	uint64_t ParsedAddress = FCString::Strtoui64(*DeviceId, nullptr, 16);
	
	BluetoothLEDevice::FromBluetoothAddressAsync(ParsedAddress).Completed([this, DeviceId](IAsyncOperation<BluetoothLEDevice> const& Op, AsyncStatus Status)
		{
			// TODO: Handle AsyncStatus 
			BluetoothLEDevice Device = { nullptr };

			if (Status != AsyncStatus::Completed)
			{
				HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

				Op.Completed([&](auto&&, auto&&)
					{
						SetEvent(Signal);
					}
				);

				WaitForSingleObject(Signal, INFINITE);
			}


			if (Status == AsyncStatus::Completed)
			{
				Device = Op.GetResults();
			}

			if (!Device)
			{
				AsyncTask(ENamedThreads::GameThread, [this, DeviceId]()
					{
						PendingConnections.Remove(DeviceId);
						OnConnectComplete.ExecuteIfBound(DeviceId, {}, false);
					}
				);
				return;
			}

			DiscoverServicesAndComplete(Device, DeviceId);
		});
}

void BLETransportWindows::Disconnect(const FString& DeviceId)
{
	FConnectedDeviceEntry* ConnectedDevice = ConnectedDevices.Find(DeviceId);
	if (ConnectedDevice)
	{
		ConnectedDevice->Device.Close();
	}
}

void BLETransportWindows::SubscribeToCharacteristic(const FString& DeviceId,
	const FString& ServiceUuid, const FString& CharUuid)
{
	const FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharUuid);

	if (GattCharacteristic* Characteristic = CachedCharacteristics.Find(CacheKey))
	{
		EnableNotifications(DeviceId, CharUuid, *Characteristic);
		return;
	}

	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (!Entry)
	{
		return;
	}

	GattDeviceService* Service = Entry->Services.Find(ServiceUuid);
	if (!Service)
	{
		return;
	}

	Service->GetCharacteristicsForUuidAsync(
		winrt::guid(TCHAR_TO_UTF8(*CharUuid)), BluetoothCacheMode::Uncached)
		.Completed(
			[this, DeviceId, ServiceUuid, CharUuid, CacheKey]
			(IAsyncOperation<GattCharacteristicsResult> const& Op, AsyncStatus Status)
			{
				if (Status != AsyncStatus::Completed)
				{
					HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

					Op.Completed([&](auto&&, auto&&)
						{
							SetEvent(Signal);
						}
					);

					WaitForSingleObject(Signal, INFINITE);
				}

				if (Op.GetResults().Status() != GattCommunicationStatus::Success ||
					Op.GetResults().Characteristics().Size() == 0)
				{
					// Characteristics genuinely doesn't exist on this device, nothing to subscribe to
					return;
				}

				GattCharacteristic Characteristic = Op.GetResults().Characteristics().GetAt(0);

				// Potential discarding of value due to OS threading behavior
				// This path might get exectued multiple times if there's multiple requests to SubscribeToMetric or ReadRequest on a as of yet uncached characteristic in a single frame
				// (e.g. on the device connect delegate where multiple metrics might be described to all at once).
				// Mentioned here and assumed this is safe behavior (28/09/2026)
				AsyncTask(ENamedThreads::GameThread, [this, CacheKey, Characteristic](){
					CachedCharacteristics.Add({ CacheKey, Characteristic });
				});

				EnableNotifications(DeviceId, CharUuid, Characteristic);
			}
		);
}

void BLETransportWindows::UnsubscribeFromCharacteristic(const FString& DeviceId,
	const FString& ServiceUuid, const FString& CharUuid)
{
	const FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharUuid);

	GattCharacteristic* Characteristic = CachedCharacteristics.Find(CacheKey);
	if (!Characteristic)
	{
		return;
	}

	if (winrt::event_token* Token = NotificationTokens.Find(CacheKey))
	{
		Characteristic->ValueChanged(*Token);
		NotificationTokens.Remove(CacheKey);
	}

	Characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(
		GattClientCharacteristicConfigurationDescriptorValue::None);
}


void BLETransportWindows::ReadCharacteristic(const FString& DeviceId,
	const FString& ServiceUuid, const FString& CharUuid)
{
	const FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharUuid);

	if (GattCharacteristic* Characteristic = CachedCharacteristics.Find(CacheKey))
	{
		ExecuteReadRequest(DeviceId, CharUuid, *Characteristic);
		return;
	}

	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (!Entry)
	{
		return;
	}

	GattDeviceService* Service = Entry->Services.Find(ServiceUuid);

	Service->GetCharacteristicsForUuidAsync(
		winrt::guid(TCHAR_TO_UTF8(*CharUuid)), BluetoothCacheMode::Uncached)
		.Completed(
			[this, DeviceId, ServiceUuid, CharUuid, CacheKey]
			(IAsyncOperation<GattCharacteristicsResult> const& Op, AsyncStatus Status)
			{
				if (Status != AsyncStatus::Completed)
				{
					HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

					Op.Completed([&](auto&&, auto&&)
						{
							SetEvent(Signal);
						}
					);

					WaitForSingleObject(Signal, INFINITE);
				}

				if (Op.GetResults().Status() != GattCommunicationStatus::Success ||
					Op.GetResults().Characteristics().Size() == 0)
				{
					// Characteristics genuinely doesn't exist on this device, nothing to subscribe to
					return;
				}

				GattCharacteristic Characteristic = Op.GetResults().Characteristics().GetAt(0);

				// Potential discarding of value due to OS threading behavior
				// This path might get exectued multiple times if there's multiple requests to SubscribeToMetric or ReadRequest on a as of yet (at this point in the game's lifecycle)
				// uncached characteristic in a single frame (e.g. on the device connect delegate where multiple metrics might be described to all at once). This is a naive solution, 
				// but seeing that multiple code paths can reach the CachedCharaceristics map which is not thread safe it's a necessary evil.
				// 
				// Mentioned here and assumed this is safe behavior (28/09/2026)
				AsyncTask(ENamedThreads::GameThread, [this, CacheKey, Characteristic]() {
					CachedCharacteristics.Add({ CacheKey, Characteristic });
				});

				ExecuteReadRequest(DeviceId, CharUuid, Characteristic);
			}
		);
}

void BLETransportWindows::WriteCharacteristic(const FString& DeviceId,
	const FString& ServiceUuid, const FString& CharUuid, FBLECharacteristicData InData)
{
	const FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharUuid);

	if (GattCharacteristic* Characteristic = CachedCharacteristics.Find(CacheKey))
	{
		ExecuteWriteRequest(DeviceId, CharUuid, *Characteristic, InData);
		return;
	}

	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (!Entry)
	{
		return;
	}

	GattDeviceService* Service = Entry->Services.Find(ServiceUuid);

	Service->GetCharacteristicsForUuidAsync(
		winrt::guid(TCHAR_TO_UTF8(*CharUuid)), BluetoothCacheMode::Uncached)
		.Completed(
			[this, DeviceId, ServiceUuid, CharUuid, CacheKey, InData]
			(IAsyncOperation<GattCharacteristicsResult> const& Op, AsyncStatus Status)
			{
				if (Status != AsyncStatus::Completed)
				{
					HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

					Op.Completed([&](auto&&, auto&&)
						{
							SetEvent(Signal);
						}
					);

					WaitForSingleObject(Signal, INFINITE);
				}

				GattCommunicationStatus GattStatus = Op.GetResults().Status();

				if (GattStatus != GattCommunicationStatus::Success ||
					Op.GetResults().Characteristics().Size() == 0)
				{
					// Characteristics genuinely doesn't exist on this device, nothing to subscribe to
					return;
				}

				GattCharacteristic Characteristic = Op.GetResults().Characteristics().GetAt(0);

				// Potential discarding of value due to OS threading behavior
				// This path might get exectued multiple times if there's multiple requests to SubscribeToMetric or ReadRequest on a as of yet (at this point in the game's lifecycle)
				// uncached characteristic in a single frame (e.g. on the device connect delegate where multiple metrics might be described to all at once). This is a naive solution, 
				// but seeing that multiple code paths can reach the CachedCharaceristics map which is not thread safe it's a necessary evil.
				// 
				// Mentioned here and assumed this is safe behavior (28/09/2026)
				AsyncTask(ENamedThreads::GameThread, [this, CacheKey, Characteristic]() {
					CachedCharacteristics.Add({ CacheKey, Characteristic });
					}
				);

				ExecuteWriteRequest(DeviceId, CharUuid, Characteristic, InData);
			}
		);
}

FString BLETransportWindows::ComposeCharacteristicCacheKey(
	FString DeviceId, FString NormalizedCharUuid)
{
	return DeviceId + TEXT("|") + NormalizedCharUuid;
}

bool BLETransportWindows::CharacteristicViableCheck(FString& OutCacheKey, 
	GattCharacteristic*& OutCharacteristic, 
	const FString& DeviceId, const FString& CharUuid)
{
	OutCacheKey = ComposeCharacteristicCacheKey(DeviceId, CharUuid);

	if (GattCharacteristic* Characteristic = CachedCharacteristics.Find(OutCacheKey))
	{
		OutCharacteristic = Characteristic;
	}
	else
	{
		return false;
	}

	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (!Entry)
	{
		return false;
	}

	return true;
}

void BLETransportWindows::DiscoverServicesAndComplete(BluetoothLEDevice Device, const FString& DeviceId)
{
	Device.GetGattServicesAsync(BluetoothCacheMode::Uncached).Completed(
		[this, Device, DeviceId](IAsyncOperation<GattDeviceServicesResult> const& Op, AsyncStatus Status)
		{
			if (Status != AsyncStatus::Completed)
			{
				HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

				Op.Completed([&](auto&&, auto&&)
					{
						SetEvent(Signal);
					}
				);
				
				WaitForSingleObject(Signal, INFINITE);
			}

			GattCommunicationStatus GattStatus = Op.GetResults().Status();
			bool bDiscovereySucceeded = GattStatus == GattCommunicationStatus::Success;

			if (!bDiscovereySucceeded)
			{
				if (GattStatus == GattCommunicationStatus::Unreachable)
				{
					for (uint32_t i = 0; i < GattStatusReconnectAttempts; i++)
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(GattStatusReconnectDelayMs));
						if (Op.GetResults().Status() == GattCommunicationStatus::Success)
						{
							bDiscovereySucceeded = true;
							continue;
						}
					}
				}

				if (!bDiscovereySucceeded)
				{
					AsyncTask(ENamedThreads::GameThread, [this, DeviceId]()
						{
							PendingConnections.Remove(DeviceId);
							OnConnectComplete.ExecuteIfBound(DeviceId, {}, false);
						}
					);
					return;
				}
			}

			FConnectedDeviceEntry Entry;
			Entry.Device = Device;

			TArray<FString> DiscoveredServiceUuids;
			for (GattDeviceService const& Service : Op.GetResults().Services())
			{
				const FString NormalizedUuid = BLEUuid::Normalize(winrt::to_hstring(Service.Uuid()).c_str());
				if (!NormalizedUuid.IsEmpty())
				{
					Entry.Services.Add(NormalizedUuid, Service);
					DiscoveredServiceUuids.Add(NormalizedUuid);
				}
			}

			ConnectedDevices.Add(DeviceId, MoveTemp(Entry));

			AsyncTask(ENamedThreads::GameThread, [this, DeviceId, DiscoveredServiceUuids = MoveTemp(DiscoveredServiceUuids)]
				{
					PendingConnections.Remove(DeviceId);
					OnConnectComplete.ExecuteIfBound(DeviceId, DiscoveredServiceUuids, true);
				}
			);
		}
	);
}

void BLETransportWindows::EnableNotifications(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic)
{
	FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharacteristicUuid);

	// Register the byte-level callback BEFORE writing the cccd - some
	// stacks can fire the first notification faster than you'd expect
	// once the descriptor write completes.
	winrt::event_token Token = Characteristic.ValueChanged(
		[this, DeviceId, CharacteristicUuid]
		(GattCharacteristic const&, GattValueChangedEventArgs const& Args)
		{
			auto Reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(Args.CharacteristicValue());
			FBLECharacteristicData Data;
			Data.SetNumUninitialized(Reader.UnconsumedBufferLength());
			Reader.ReadBytes(winrt::array_view<uint8>(Data.GetData(), Data.Num()));

			AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CharacteristicUuid, Data = MoveTemp(Data)]()
				{
					OnCharacteristicUpdated.ExecuteIfBound(DeviceId, CharacteristicUuid, Data);
				}
			);
		}
	);

	NotificationTokens.Add(CacheKey, Token);

	Characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
		GattClientCharacteristicConfigurationDescriptorValue::Notify)
		.Completed(
			[this, DeviceId, CharacteristicUuid]
			(IAsyncOperation<GattCommunicationStatus> const& Op, AsyncStatus Status)
			{
				if (Status != AsyncStatus::Completed)
				{
					HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

					Op.Completed([&](auto&&, auto&&)
						{
							SetEvent(Signal);
						}
					);

					WaitForSingleObject(Signal, INFINITE);
				}

				if (Op.GetResults() != GattCommunicationStatus::Success)
				{
					UE_LOG(LogTemp, Warning, TEXT("Temporary not succesfull"));
				}
			}
		);
}

void BLETransportWindows::ExecuteReadRequest(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic)
{
	FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharacteristicUuid);

	if (QueuedReadRequests.Contains(CacheKey))
	{
		return;
	}

	// Add to queue
	FReadRequestEntry Entry{
		DeviceId,
		CharacteristicUuid,
		Characteristic
	};
	QueuedReadRequests.Add(CacheKey);
	CachekeyToReadRequests.Add({ CacheKey, Entry });

	if (!bActivelyProcessingReadRequest)
	{
		ThreadRecursiveReadRequest(DeviceId, CharacteristicUuid, Characteristic);
	}
}

void BLETransportWindows::ThreadRecursiveReadRequest(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic)
{
	bActivelyProcessingReadRequest = true;

	// Important is that we can only ever support one read at a time, distributing if multiple callers arrive is up to us.
	Characteristic.ReadValueAsync(BluetoothCacheMode::Uncached).Completed(
		[this, DeviceId, CharacteristicUuid](IAsyncOperation<GattReadResult> const& Op, AsyncStatus Status)
		{
			if (Status != AsyncStatus::Completed)
			{
				HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

				Op.Completed([&](auto&&, auto&&)
					{
						SetEvent(Signal);
					}
				);

				WaitForSingleObject(Signal, INFINITE);
			}

			FBLECharacteristicData Data(
				Op.GetResults().Value().data(),
				Op.GetResults().Value().Length()
			);

			AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CharacteristicUuid, Data = MoveTemp(Data)]()
				{
					OnReadRequestCompleted.ExecuteIfBound(DeviceId, CharacteristicUuid, Data);

					FString CacheKey = ComposeCharacteristicCacheKey(DeviceId, CharacteristicUuid);

					// Remove the current read request (this is finished now)
					QueuedReadRequests.RemoveAt(0);
					CachekeyToReadRequests.Remove(CacheKey);

					// Check if the requests are still empty
					if (!QueuedReadRequests.IsEmpty())
					{
						FString QueuedRequestKey = QueuedReadRequests[0];

						FReadRequestEntry Entry = CachekeyToReadRequests[QueuedRequestKey];

						ThreadRecursiveReadRequest(Entry.DeviceId, Entry.CharacteristicUuid, Entry.Characteristic);
					}
					else
					{
						bActivelyProcessingReadRequest = false;
					}
				});
		});
}

void BLETransportWindows::ExecuteWriteRequest(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic,
	FBLECharacteristicData InData)
{
	if (OpenWriteRequest.IsSet())
	{
		// Request in progress, add to queue
		FWriteRequestEntry Entry{
			DeviceId,
			CharacteristicUuid,
			Characteristic,
			InData
		};

		QueuedWriteRequests.Push(Entry);
		return;
	}

	winrt::Windows::Storage::Streams::Buffer buffer(InData.Num());
	memcpy(buffer.data(), InData.GetData(), InData.Num());
	Characteristic.WriteValueWithResultAsync(buffer).Completed(
		[this, DeviceId, CharacteristicUuid](IAsyncOperation<GattWriteResult> Op, AsyncStatus Status) 
		{
			if (Status != AsyncStatus::Completed)
			{
				HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);

				Op.Completed([&](auto&&, auto&&) {
						SetEvent(Signal);
					}
				);

				WaitForSingleObject(Signal, INFINITE);
			}

			bool bSuccess = false;
			if (Op.GetResults().Status() == GattCommunicationStatus::Success)
			{
				bSuccess = true;
			}

			AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CharacteristicUuid, bSuccess]()
				{
					OnWriteRequestCompleted.ExecuteIfBound(DeviceId, CharacteristicUuid, bSuccess);

					if (!QueuedWriteRequests.IsEmpty())
					{
						OpenWriteRequest = QueuedWriteRequests[0];
						QueuedWriteRequests.RemoveAt(0);

						const FWriteRequestEntry& Entry = OpenWriteRequest.GetValue();

						ExecuteWriteRequest(Entry.DeviceId, Entry.CharacteristicUuid,
							Entry.Characteristic, Entry.Payload);
					}
					else
					{
						OpenWriteRequest.Reset();
					}
				}
			);
		}
	);
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
		Result.AdvertisedServices.Add(BLEUuid::Normalize(FString(winrt::to_hstring(uuid).c_str())));
	}

	// Marshal back to the game thread
	AsyncTask(ENamedThreads::GameThread, 
		[this, Result = MoveTemp(Result)]
		{
			OnDeviceFound.ExecuteIfBound(Result);
		}
	);
}