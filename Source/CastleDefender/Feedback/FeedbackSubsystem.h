#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Feedback/FeedbackTypes.h"
#include "FeedbackSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFeedbackPlayedSignature, FGameplayTag, RowTag, const FFeedbackEventContext&, Context);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHUDLayersChangedSignature, int32, LayerMask);

/**
 * Single entry point for presentation feedback (D-10, R-UXF-02). Gameplay calls Play(Tag, Context);
 * the row in DT_Feedback decides what plays. Also owns the world-level HUD layer flags (R-UXF-23).
 * Map lifetime: one table index, throttle and played-tag set per game/PIE world.
 */
UCLASS()
class CASTLEDEFENDER_API UFeedbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UFeedbackSubsystem* Get(const UObject* WorldContextObject);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Plays the row for Tag (or its variant from Context). Returns true when the row played;
	 * false for an unknown tag, a missing table, or a cooldown/burst rejection.
	 * Hit stop and camera shake fields are applied from T-UXF-03.
	 */
	UFUNCTION(BlueprintCallable, Category = "Feedback")
	bool Play(UPARAM(meta = (Categories = "Feedback")) FGameplayTag Tag, const FFeedbackEventContext& Context);

	/** Replaces the table loaded from Game Tuning (tests, tools). Clears throttle and played state. */
	void SetFeedbackTable(UDataTable* Table);

	UFUNCTION(BlueprintCallable, Category = "Feedback|HUD")
	void SetHUDLayerActive(EHUDLayer Layer, bool bActive);

	UFUNCTION(BlueprintPure, Category = "Feedback|HUD")
	bool IsHUDLayerActive(EHUDLayer Layer) const { return EnumHasAnyFlags(HUDLayers, Layer); }

	EHUDLayer GetHUDLayers() const { return HUDLayers; }

	/** Highest active layer: Modal > CommanderSpirit > TacticalFocus > Build > CommandWheel. None = Combat. */
	UFUNCTION(BlueprintPure, Category = "Feedback|HUD")
	EHUDLayer GetTopHUDLayer() const;

	/** Markers switch to tactical display under Focus or Commander Spirit, unless a modal is open. */
	UFUNCTION(BlueprintPure, Category = "Feedback|HUD")
	bool IsTacticalDisplay() const;

	/** Explicitly declared Feedback.* tags that no row has played in this world (game.feedback.Coverage). */
	TArray<FGameplayTag> GetUnplayedDeclaredTags() const;

	UPROPERTY(BlueprintAssignable, Category = "Feedback")
	FFeedbackPlayedSignature OnFeedbackPlayed;

	UPROPERTY(BlueprintAssignable, Category = "Feedback|HUD")
	FHUDLayersChangedSignature OnHUDLayersChanged;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void RebuildIndex();
	void WarnOnce(FName Key, const FString& Message);
	void PlayOutputs(const FFeedbackRow& Row, const FFeedbackEventContext& Context) const;
	void ShowRecentTags() const;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> FeedbackTable;

	FFeedbackRowIndex RowIndex;
	FFeedbackThrottle Throttle;
	TSet<FGameplayTag> PlayedTags;
	TSet<FName> WarnedKeys;
	TArray<FGameplayTag> RecentTags;
	EHUDLayer HUDLayers = EHUDLayer::None;
};
