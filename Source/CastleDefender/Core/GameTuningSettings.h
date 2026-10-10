#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GameTuningSettings.generated.h"

class UDataTable;

/**
 * Global [TUNABLE] values (D-06). Project Settings > Game > Game Tuning; saved to DefaultGame.ini.
 * Features add properties under the categories Combat, Enemy, Army, Focus, Respawn, Feedback and Debug.
 * Per-content values belong in definition assets (UGameDefinition), not here.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Tuning"))
class CASTLEDEFENDER_API UGameTuningSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameTuningSettings();

	static const UGameTuningSettings* Get() { return GetDefault<UGameTuningSettings>(); }

	/** R-SQD-01: prototype limit; registry lives on the controller across Hero respawns. */
	UPROPERTY(Config, EditAnywhere, Category = "Army", meta = (ClampMin = "1"))
	int32 MaxActiveSquads = 3;

	/** R-SYN-08: duration for a state applied without one (no hit duration, not a poise break). 0 or missing = not applied. */
	UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ForceInlineRow, Categories = "State.Combat"))
	TMap<FGameplayTag, float> StateDefaultDurations;

	/** R-SYN-13: remaining fraction of BaseArmor during Armor Broken. */
	UPROPERTY(Config, EditAnywhere, Category = "Combat", meta = (ClampMin = "0", ClampMax = "1"))
	float ArmorBrokenArmorMultiplier = 0.25f;

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

	/** UXF approved global shake scale; telemetry records the actual session value. */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0"))
	float CameraShakeScale = 1.f;
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float HitStopDilation = 0.05f;
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0", Units = "s"))
	float MaxHitStopSeconds = 0.15f;

	/** R-UXF-03c, AC-UXF-09: ratio of MaxHealth below which low health feedback triggers. */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float HeroLowHealthThreshold = 0.30f;

	/** Ratio of MaxHealth above which low health latch is re-armed and pulse stops. */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0", ClampMax = "1"))
	float HeroLowHealthRearmThreshold = 0.40f;

	/** R-ENM-05: shortest allowed wind-up (attack start → first hit window). Enemy definitions warn below it. User default 2026-10-06. */
	UPROPERTY(Config, EditAnywhere, Category = "Enemy", meta = (ClampMin = "0", Units = "s"))
	float MinEnemyTelegraphTime = 0.4f;
};
