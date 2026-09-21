#pragma once

/* One decoded value from a characteristic update (e.g. HeartRateBPM (55.0f) */
struct FBLEMetric
{
public:
	FBLEMetric() = default;
	FBLEMetric(FName InMetric, float InValue)
		:MetricName(InMetric)
		,Value(InValue)
	{

	}

	FName MetricName;
	float Value = 0.0f;
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

	/* Decodes one characteristic update. May return multiple metrics */
	virtual TArray<FBLEMetric> Parse(const TArray<uint8>& Data) = 0;

	/*
	 * Clears any state carried between packets.
	 * MUST be called before resubscribing to a characteristic after
	 * unsubscribing. So a resumed subscription doesn't delta against
	 * stale -potentially minutes-old- data.
	 * No-op for stateless parsers (e.g. heart rate)
	 */
	virtual void Reset() = 0;

};