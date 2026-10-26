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
			DiscoveredServiceUuids = MoveTemp(DiscoveredServiceUuids), SupportedCharUuids = Entry->SupportedCharacteristics]()
			{
				OnConnectComplete.ExecuteIfBound(
					DeviceId, DiscoveredServiceUuids, SupportedCharUuids, EBLEDeviceConnectionReturnCode::Success);
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
				if (Status == AsyncStatus::Error)
				{
					winrt::hresult_error error(Op.ErrorCode());
					UE_LOG(LogTemp, Warning, TEXT("Error code %s"), error.message().c_str());
				}
			}
			else
			{
				Device = Op.GetResults();
			}

			if (!Device)
			{
				AsyncTask(ENamedThreads::GameThread, [this, DeviceId]()
					{
						PendingConnections.Remove(DeviceId);
						OnConnectComplete.ExecuteIfBound(DeviceId, {}, {}, EBLEDeviceConnectionReturnCode::Unreachable);
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
		for (FString CacheKey : ConnectedDevice->ActiveCharacteristics)
		{
			CloseCharacteristic(CacheKey);
		}
		ConnectedDevice->ActiveCharacteristics.Empty();

		BluetoothLEDevice Device = ConnectedDevice->Device;
		ConnectedDevices.Remove(DeviceId);

		Device.Close();

		OnDeviceDisconnect.ExecuteIfBound(DeviceId);
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
				AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CacheKey, Characteristic](){
					AddCharacteristicToCache(DeviceId, CacheKey, Characteristic);
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
				AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CacheKey, Characteristic]() {
					AddCharacteristicToCache(DeviceId, CacheKey, Characteristic);
				});

				ExecuteReadRequest(DeviceId, CharUuid, Characteristic);
			}
		);
}

void BLETransportWindows::WriteCharacteristic(const FString& DeviceId,
	const FString& ServiceUuid, const FString& CharUuid, FBLECharacteristicData InData, 
	bool bSupportsIndicate)
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
			[this, DeviceId, ServiceUuid, CharUuid, CacheKey, InData, bSupportsIndicate]
			(IAsyncOperation<GattCharacteristicsResult> const& Op, AsyncStatus Status)
			{
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
				AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CacheKey, Characteristic]() {
						AddCharacteristicToCache(DeviceId, CacheKey, Characteristic);
					});

				if (bSupportsIndicate)
				{
					// Indication response
					winrt::event_token Token = Characteristic.ValueChanged(
						[this, DeviceId, CharUuid](GattCharacteristic const&, GattValueChangedEventArgs const& Args)
						{
							TArray<uint8> ResponsePacketBuffer(
								Args.CharacteristicValue().data(),
								Args.CharacteristicValue().Length()
							);

							AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CharUuid, ResponsePacketBuffer]()
								{
									OnWriteIndicationReceived.ExecuteIfBound(DeviceId, CharUuid, ResponsePacketBuffer);
								});
						});

					NotificationTokens.Add(CacheKey, Token);

					Characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
						GattClientCharacteristicConfigurationDescriptorValue::Indicate).Completed(
							[this, DeviceId, CharUuid, InData, Characteristic](IAsyncOperation<GattCommunicationStatus> const& Op, AsyncStatus Status)
							{
								ExecuteWriteRequest(DeviceId, CharUuid, Characteristic, InData);
							});
				}
				else
				{
					ExecuteWriteRequest(DeviceId, CharUuid, Characteristic, InData);
				}
			}
		);
}

FString BLETransportWindows::ComposeCharacteristicCacheKey(
	FString DeviceId, FString NormalizedCharUuid)
{
	return DeviceId + TEXT("|") + NormalizedCharUuid;
}

void BLETransportWindows::AddCharacteristicToCache(const FString& DeviceId, const FString& CacheKey, GattCharacteristic Characteristic)
{
	CachedCharacteristics.Add({ CacheKey, Characteristic });
	FConnectedDeviceEntry* Entry = ConnectedDevices.Find(DeviceId);
	if (Entry)
	{
		Entry->ActiveCharacteristics.Add(CacheKey);
	}
}

void BLETransportWindows::CloseCharacteristic(FString CacheKey)
{
	if (GattCharacteristic* Characteristic = CachedCharacteristics.Find(CacheKey))
	{
		if (static_cast<uint32_t>((Characteristic->CharacteristicProperties()) & (GattCharacteristicProperties::Notify | GattCharacteristicProperties::Indicate)))
		{
			Characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue::None);
		}

		if (winrt::event_token* Token = NotificationTokens.Find(CacheKey))
		{
			Characteristic->ValueChanged(*Token);
			NotificationTokens.Remove(CacheKey);
		}

		CachedCharacteristics.Remove(CacheKey);
	}
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
			GattCommunicationStatus GattStatus = Op.GetResults().Status();
			bool bDiscovereySucceeded = GattStatus == GattCommunicationStatus::Success;

			if(GattStatus != GattCommunicationStatus::Success)
			{
				EBLEDeviceConnectionReturnCode ReturnCode = EBLEDeviceConnectionReturnCode::Unreachable;

				switch (GattStatus)
				{
					case GattCommunicationStatus::AccessDenied:
					{
						ReturnCode = EBLEDeviceConnectionReturnCode::AccessDenied;
						break;
					}
					case GattCommunicationStatus::ProtocolError:
					{
						ReturnCode = EBLEDeviceConnectionReturnCode::ProtocolError;
						break;
					}
					case GattCommunicationStatus::Unreachable:
					{
						ReturnCode = EBLEDeviceConnectionReturnCode::Unreachable;
						break;
					}
				}

				AsyncTask(ENamedThreads::GameThread, [this, DeviceId, ReturnCode]()
					{
						PendingConnections.Remove(DeviceId);
						OnConnectComplete.ExecuteIfBound(DeviceId, {}, {}, ReturnCode);
					});
				return;
			}

			FConnectedDeviceEntry Entry;
			Entry.Device = Device;

			TArray<FString> DiscoveredServiceUuids;
			TArray<FString> DiscoveredCharacteristicUuids;
			for (GattDeviceService const& Service : Op.GetResults().Services())
			{
				const FString NormalizedUuid = BLEUuid::Normalize(winrt::to_hstring(Service.Uuid()).c_str());
				if (!NormalizedUuid.IsEmpty())
				{
					Entry.Services.Add(NormalizedUuid, Service);
					DiscoveredServiceUuids.Add(NormalizedUuid);
				}

				HANDLE Signal = CreateEvent(nullptr, true, false, nullptr);
				Service.GetCharacteristicsAsync(BluetoothCacheMode::Uncached).Completed(
					[Signal, &DiscoveredCharacteristicUuids](const IAsyncOperation<GattCharacteristicsResult>& Op, AsyncStatus Status)
					{
						if (Status != AsyncStatus::Completed)
						{
							SetEvent(Signal);
							return;
						}

						GattCharacteristicsResult Result = Op.GetResults();
						for (const GattCharacteristic& Characteristic : Result.Characteristics())
						{
							DiscoveredCharacteristicUuids.Add(BLEUuid::Normalize(winrt::to_hstring(Characteristic.Uuid()).c_str()));
						}

						SetEvent(Signal);
					});

				WaitForSingleObject(Signal, INFINITE);

				CloseHandle(Signal);

			}
			Entry.SupportedCharacteristics.Append(DiscoveredCharacteristicUuids);

			ConnectedDevices.Add(DeviceId, MoveTemp(Entry));

			AsyncTask(ENamedThreads::GameThread, 
				[this, DeviceId, DiscoveredServiceUuids = MoveTemp(DiscoveredServiceUuids),
				DiscoveredCharacteristicUuids = MoveTemp(DiscoveredCharacteristicUuids)]
				{
					PendingConnections.Remove(DeviceId);
					OnConnectComplete.ExecuteIfBound(DeviceId, DiscoveredServiceUuids, DiscoveredCharacteristicUuids, EBLEDeviceConnectionReturnCode::Success);
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

	if (!bActivelyProcessingReadRequests)
	{
		bActivelyProcessingReadRequests = true;

		ThreadRecursiveReadRequest(DeviceId, CharacteristicUuid, Characteristic);
	}
}

void BLETransportWindows::ThreadRecursiveReadRequest(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic)
{

	// Important is that we can only ever support one read at a time, distributing if multiple callers arrive is up to us.
	Characteristic.ReadValueAsync(BluetoothCacheMode::Uncached).Completed(
		[this, DeviceId, CharacteristicUuid](IAsyncOperation<GattReadResult> const& Op, AsyncStatus Status)
		{
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
						bActivelyProcessingReadRequests = false;
					}
				});
		});
}

void BLETransportWindows::ExecuteWriteRequest(const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic,
	FBLECharacteristicData InData)
{
	// Request in progress, add to queue
	FWriteRequestEntry Entry{
		DeviceId,
		CharacteristicUuid,
		Characteristic,
		InData
	};

	QueuedWriteRequests.Push(Entry);

	if (!bActivelyProcessingWriteRequests)
	{
		bActivelyProcessingWriteRequests = true;

		ThreadRecursiveWriteRequest(DeviceId, CharacteristicUuid, Characteristic,
			InData
		);
	}
}

void BLETransportWindows::ThreadRecursiveWriteRequest(
	const FString& DeviceId,
	const FString& CharacteristicUuid, GattCharacteristic Characteristic,
	const FBLECharacteristicData& InData)
{

	winrt::Windows::Storage::Streams::Buffer buffer(InData.Num());
	buffer.Length(InData.Num());
	memcpy(buffer.data(), InData.GetData(), InData.Num());

	Characteristic.WriteValueWithResultAsync(buffer, GattWriteOption::WriteWithResponse).Completed(
		[this, DeviceId, CharacteristicUuid](IAsyncOperation<GattWriteResult> Op, AsyncStatus Status)
		{

			bool bSuccess = false;
			if (Op.GetResults().Status() == GattCommunicationStatus::Success)
			{
				bSuccess = true;
			}

			AsyncTask(ENamedThreads::GameThread, [this, DeviceId, CharacteristicUuid, bSuccess]()
				{
					OnWriteRequestCompleted.ExecuteIfBound(DeviceId, CharacteristicUuid, bSuccess);

					QueuedWriteRequests.RemoveAt(0);

					if (!QueuedWriteRequests.IsEmpty())
					{
						const FWriteRequestEntry& Entry = QueuedWriteRequests[0];
						ThreadRecursiveWriteRequest(
							Entry.DeviceId,
							Entry.CharacteristicUuid,
							Entry.Characteristic,
							Entry.BytePacket);
					}
					else
					{
						bActivelyProcessingWriteRequests = false;
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

	int16_t RawSignalStrength = Args.RawSignalStrengthInDBm();
	if (RawSignalStrength > -40)
	{
		Result.SignalStrength = EBLEScanResultSignalStrength::Excellent;
	}
	else if (RawSignalStrength <= -40 && RawSignalStrength >= -70)
	{
		Result.SignalStrength = EBLEScanResultSignalStrength::Good;
	}
	else
	{
		Result.SignalStrength = EBLEScanResultSignalStrength::Weak;
	}

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