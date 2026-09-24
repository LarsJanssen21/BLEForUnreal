#pragma once

#include "IBLECharacteristicParser.h"

class FHeartRateParser : public IBLECharacteristicParser
{
public:
	virtual FString GetServiceUuid() const override;
	virtual FString GetCharacteristicUuid() const override;

	virtual TArray<FName> GetSupportedMetrics() const override;
	virtual TArray<FName> GetSupportedReadRequests() const override { return{}; }

	virtual TArray<FBLEMetric> ParseNotify(const TArray<uint8>& Data) override;
	virtual TArray<FBLERead> ParseReadRequest(const TArray<uint8>& Data) override { return{}; }
	virtual void Reset() override { }
};