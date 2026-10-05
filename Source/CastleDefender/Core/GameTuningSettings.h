#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GameTuningSettings.generated.h"

class UDataTable;

/**
 * Global [TUNABLE] values (D-06). Project Settings > Game > Game Tuning; saved to DefaultGame.ini.
 * Features add properties under the categories Combat, Army, Focus, Respawn, Feedback and Debug.
 * Per-content values belong in definition assets (UGameDefinition), not here.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Tuning"))
class CASTLEDEFENDER_API UGameTuningSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameTuningSettings();

	static const UGameTuningSettings* Get() { return GetDefault<UGameTuningSettings>(); }

	/** R-SYN-08: duration for a state applied without one (no hit duration, not a poise break). 0 or missing = not applied. */
	UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ForceInlineRow, Categories = "State.Combat"))
	TMap<FGameplayTag, float> StateDefaultDurations;

	/** DT_CombatStatePresentation (FCombatStatePresentationRow rows): feedback per state. */
	UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (RequiredAssetDataTags = "RowStructure=/Script/CastleDefender.CombatStatePresentationRow"))
	TSoftObjectPtr<UDataTable> CombatStatePresentationTable;

	/** DT_Feedback (FFeedbackRow rows), loaded by UFeedbackSubsystem at map start (D-10). */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (RequiredAssetDataTags = "RowStructure=/Script/CastleDefender.FeedbackRow"))
	TSoftObjectPtr<UDataTable> FeedbackTable;

	/** R-UXF-03b: plays per tag per window when a row leaves BurstLimit at 0. */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "1"))
	int32 DefaultBurstLimit = 4;

	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0.01", Units = "s"))
	float DefaultBurstWindow = 0.25f;
};
