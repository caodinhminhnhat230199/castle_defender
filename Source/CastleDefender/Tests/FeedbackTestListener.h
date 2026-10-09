#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/DataTable.h"
#include "Feedback/FeedbackTypes.h"
#include "FeedbackTestListener.generated.h"

/** Test helper: counts UFeedbackSubsystem broadcasts (dynamic delegates need a UFUNCTION target). */
UCLASS(Transient)
class UFeedbackTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 PlayedCount = 0;
	FGameplayTag LastPlayed;
	TArray<FGameplayTag> PlayedTags;
	FFeedbackEventContext LastContext;
	int32 LayersChangedCount = 0;
	int32 LastLayerMask = 0;

	UFUNCTION()
	void HandlePlayed(FGameplayTag RowTag, const FFeedbackEventContext& Context) { ++PlayedCount; LastPlayed = RowTag; PlayedTags.Add(RowTag); LastContext = Context; }

	UFUNCTION()
	void HandleLayersChanged(int32 LayerMask) { ++LayersChangedCount; LastLayerMask = LayerMask; }

	/** Transient DT_Feedback stand-in with one empty row per tag (row name = tag). */
	static UDataTable* MakeTable(std::initializer_list<FGameplayTag> Tags, float CooldownSeconds = 0.f, bool bPerActor = false)
	{
		UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
		Table->RowStruct = FFeedbackRow::StaticStruct();
		for (const FGameplayTag& Tag : Tags)
		{
			FFeedbackRow Row;
			Row.Tag = Tag;
			Row.CooldownSeconds = CooldownSeconds;
			Row.bCooldownPerActor = bPerActor;
			Table->AddRow(Tag.GetTagName(), Row);
		}
		return Table;
	}
};
