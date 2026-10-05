#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
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
	static const UGameTuningSettings* Get() { return GetDefault<UGameTuningSettings>(); }

	/** DT_Feedback (FFeedbackRow rows), loaded by UFeedbackSubsystem at map start (D-10). */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (RequiredAssetDataTags = "RowStructure=/Script/CastleDefender.FeedbackRow"))
	TSoftObjectPtr<UDataTable> FeedbackTable;

	/** R-UXF-03b: plays per tag per window when a row leaves BurstLimit at 0. */
	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "1"))
	int32 DefaultBurstLimit = 4;

	UPROPERTY(Config, EditAnywhere, Category = "Feedback", meta = (ClampMin = "0.01", Units = "s"))
	float DefaultBurstWindow = 0.25f;
};
