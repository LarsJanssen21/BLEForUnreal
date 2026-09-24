#pragma once

enum class DataType : uint8 {
	Invalid,
	Float,
	String
};

/* One decoded value from a characteristic update (e.g. HeartRateBPM (55.0f) */
struct FBLEMetric
{
public:
	FBLEMetric() = default;
	FBLEMetric(FName InMetric, float InValue)
		:MetricName(InMetric)
		,Value(InValue)
		,Type(DataType::Float)
	{}

	FName MetricName;
	float Value = 0.0f;
	DataType Type = DataType::Invalid;
};

/* One decoded value from a read request (e.g. DeviceName ("HeartRateMonitor") */
struct FBLERead
{
public:
	FBLERead() = default;
	FBLERead(FName InRead, float InValue)
		:ReadName(InRead)
		,Value(InValue)
		,Type(DataType::Float)
	{ }
	FBLERead(FName InRead, FString InString)
		:ReadName(InRead)
		,String(InString)
		,Type(DataType::String)
	{ }

	FName ReadName;
	float Value = 0.0f;
	FString String = "";
	DataType Type = DataType::Invalid;
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

	/* If characteristic is of type: notify								*/
	/* Decodes one characteristic update. May return multiple metrics	*/
	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) = 0;

	/* If characteristic is of type: read										*/
	/* Decodes one characteristic read request. May return multiple read values */
	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) = 0;

	/*
	 * Clears any state carried between packets.
	 * MUST be called before resubscribing to a characteristic after
	 * unsubscribing. So a resumed subscription doesn't delta against
	 * stale -potentially minutes-old- data.
	 * No-op for stateless parsers (e.g. heart rate)
	 */
	virtual void Reset() = 0;

};