#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#if !UE_BUILD_SHIPPING
#include "Serialization/JsonWriter.h"
#include "Policies/CondensedJsonPrintPolicy.h"
using FPlaytestJsonWriter = TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>;

/** Pure P0 aggregation of the recorded presentation and action-state stream. */
struct CASTLEDEFENDER_API FPlaytestSummary
{
	TMap<FString, int32> Counts;
	TMap<FName, int32> Actions;
	int32 HeroDeaths = 0;
	int32 Parries = 0;
	int32 BlockBreaks = 0;
	void RecordFeedback(FGameplayTag Tag);
	void RecordAction(FName Action);
	void WriteFields(FPlaytestJsonWriter& Json, double Duration) const;
	FString ToJson(double Duration) const;
};
#endif
