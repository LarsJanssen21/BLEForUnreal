#pragma once

#include "IBLECharacteristicParser.generated.h"

#define READ_ONLY_CHARACTERISTIC_IMPLEMENTATION(ServiceUuid, CharUuid, ...) \
virtual FString GetServiceUuid() const override { return ServiceUuid; } \
virtual FString GetCharacteristicUuid() const override { return CharUuid; } \
virtual TArray<FName> GetSupportedMetrics() const override { return {}; } \
virtual TArray<FName> GetSupportedReadRequests() const override { return { __VA_ARGS__ }; } \
virtual TArray<FName> GetSupportedWriteSubmits() const override { return {}; } \
virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override { return {}; } \
virtual TArray<FBLEWrite> ParseWriteRequest(const TArray<uint8>& Data) override { return {}; } \
virtual TArray<uint8> PrepareWriteRequestBuffer(FName WriteName, uint32_t Value) { return {}; } \
virtual bool SupportsIndicate() const override { return false; }

#define METRIC_ONLY_CHARACTERISTIC_IMPLEMENTATION(ServiceUuid, CharUuid, ...) \
virtual FString GetServiceUuid() const override { return ServiceUuid; } \
virtual FString GetCharacteristicUuid() const override { return CharUuid; } \
virtual TArray<FName> GetSupportedMetrics() const override { return {__VA_ARGS__}; } \
virtual TArray<FName> GetSupportedReadRequests() const override { return {}; } \
virtual TArray<FName> GetSupportedWriteSubmits() const override { return {}; } \
virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override { return {}; } \
virtual TArray<FBLEWrite> ParseWriteRequest(const TArray<uint8>& Data) override { return {}; } \
virtual TArray<uint8> PrepareWriteRequestBuffer(FName WriteName, uint32_t Value) { return {}; } \
virtual bool SupportsIndicate() const override { return false; }

#define WRITE_ONLY_CHARACTERISTIC_IMPLEMENTATION(ServiceUuid, CharUuid, ...) \
virtual FString GetServiceUuid() const override { return ServiceUuid; } \
virtual FString GetCharacteristicUuid() const override { return CharUuid; } \
virtual TArray<FName> GetSupportedMetrics() const override { return {}; } \
virtual TArray<FName> GetSupportedReadRequests() const override { return {}; } \
virtual TArray<FName> GetSupportedWriteSubmits() const override { return { __VA_ARGS__}; } \
virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override { return {}; } \
virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override { return {}; }

UENUM(BlueprintType)
enum class CharacteristicDataType : uint8 {
	Invalid,
	Float,
	Int,
	String
};

/* One decoded value from a characteristic update (e.g. HeartRateBPM (55.0f) */
USTRUCT(BlueprintType)
struct FBLEMetric
{
	GENERATED_BODY()

	FBLEMetric() = default;
	FBLEMetric(FName InMetric, float InValue)
		:MetricName(InMetric)
		,Value(InValue)
		,Type(CharacteristicDataType::Float)
	{}
	FBLEMetric(FName InMetric, const FString& InValue)
		:MetricName(InMetric)
		, StringValue(InValue)
		, Type(CharacteristicDataType::String)
	{
	}

	FName MetricName;
	UPROPERTY(BlueprintReadOnly)
	float Value = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	FString StringValue = "";
	UPROPERTY(BlueprintReadOnly)
	CharacteristicDataType Type = CharacteristicDataType::Invalid;
};

/* One decoded value from a read request (e.g. DeviceName ("HeartRateMonitor") */
USTRUCT(BlueprintType)
struct FBLERead
{
	GENERATED_BODY()

	FBLERead() = default;
	FBLERead(FName InRead, float InValue)
		:ReadName(InRead)
		,Value(InValue)
		,Type(CharacteristicDataType::Float)
	{ }
	FBLERead(FName InRead, FString InString)
		:ReadName(InRead)
		,String(InString)
		,Type(CharacteristicDataType::String)
	{ }

	FName ReadName;
	UPROPERTY(BlueprintReadOnly)
	float Value = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	FString String = "";
	UPROPERTY(BlueprintReadOnly)
	CharacteristicDataType Type = CharacteristicDataType::Invalid;
};

USTRUCT(BlueprintType)
struct FBLEWrite
{
	GENERATED_BODY()

	FBLEWrite() = default;
	FBLEWrite(FName InWrite, int32 InValue)
		:WriteName(InWrite)
		,Value(InValue)
		,Type(CharacteristicDataType::Int)
	{ }

	FName WriteName;
	UPROPERTY(BlueprintReadOnly)
	int32 Value;
	UPROPERTY(BlueprintReadOnly)
	CharacteristicDataType Type = CharacteristicDataType::Invalid;
};

/*
 * Decodes one raw byte payload of ONE GATT characteristic into one or more named metrics
 * One implementation per characteristic
 * Lifetime: 
 *	Constructed once per UBLEDevice, for the life of the connection
 *	regardless of Subscribe/Unsubscribe cycles (see Reset())
*/
class IBLECharacteristicParser
{
public:
	virtual ~IBLECharacteristicParser() = default;

	/* Canonical BLEUuid::Normalize'd service characteristics this parser handles*/
	virtual FString GetServiceUuid() const = 0;
	virtual FString GetCharacteristicUuid() const = 0;

	/*
	 * Every metric name this parser can produce, known statically - 
	 * lets UBLEDevice answer GetAvailableMetrics() without subscribing to anything yet.
	*/
	virtual TArray<FName> GetSupportedMetrics() const = 0;

	/*
	 * Every read request this parser can accept, known statically -
	*/
	virtual TArray<FName> GetSupportedReadRequests() const = 0;
	/*
	 * Every write submit this parser can accept, known statically -
	*/
	virtual TArray<FName> GetSupportedWriteSubmits() const = 0;

	/* If characteristic is of type: notify								*/
	/* Decodes one characteristic update. May return multiple metrics	*/
	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) = 0;

	/* If characteristic is of type: read										*/
	/* Decodes one characteristic read request. May return multiple read values */
	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) = 0;

	/* If characteristic is of type: write										*/
	/* Decodes one characteristic write request. May return multiple write values */
	virtual TArray<FBLEWrite> ParseWriteRequest(const TArray<uint8>& Data) = 0;

	/*
	 * Clears any state carried between packets.
	 * MUST be called before resubscribing to a characteristic after
	 * unsubscribing. So a resumed subscription doesn't delta against
	 * stale -potentially minutes-old- data.
	 * No-op for stateless parsers (e.g. heart rate)
	 */
	virtual void Reset() = 0;

	/*	
	 * Prepares data packet for write request	
	*/
	virtual TArray<uint8> PrepareWriteRequestBuffer(FName WriteName, uint32 Value) = 0;
	virtual bool SupportsIndicate() const = 0;
};