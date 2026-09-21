#pragma once

#include "IBLECharacteristicParser.h"

class FHeartRateParser : public IBLECharacteristicParser
{
public:
	virtual FString GetServiceUuid() const override;
	virtual FString GetCharacteristicUuid() const override;
	virtual TArray<FName> GetSupportedMetrics() const override;
	virtual TArray<FBLEMetric> Parse(const TArray<uint8>& Data) override;
	virtual void Reset() override;
};