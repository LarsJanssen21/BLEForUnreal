#pragma once

#include "CoreMinimal.h"


namespace BLEUuid {


	const FString HeartRateServiceUUID = FString(TEXT("180d"));


	inline FString ExpandShortUuid(uint16 ServiceUuid)
	{
		return FString::Printf(TEXT("{0000%04X-0000-1000-8000-00805F9B34FB}"), ServiceUuid);
	}

	inline FString ExpandShortUuidString(const FString& ServiceUuid)
	{
		return FString::Printf(TEXT("{0000%s-0000-1000-8000-00805F9B34FB}"), *ServiceUuid.ToLower());
	}

	inline FString Normalize(const FString& RawUuid)
	{
		FString Trimmed = RawUuid.TrimStartAndEnd().Replace(TEXT("{"), TEXT("")).Replace(TEXT("}"), TEXT(""));

		if (Trimmed.Len() == 4)
		{
			Trimmed = ExpandShortUuidString(Trimmed);
		}

		FGuid ParsedGuid;
		if (!FGuid::ParseExact(Trimmed, EGuidFormats::DigitsWithHyphens, ParsedGuid))
		{
			return FString();
		}

		return Trimmed.ToLower();
	}

	inline bool AreEqual(const FString& A, const FString& B)
	{
		const FString NormA = Normalize(A);
		const FString NormB = Normalize(B);
		return !NormA.IsEmpty() && NormA == NormB;
	}
}