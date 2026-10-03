#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameTuningSettings.generated.h"

/**
 * Global [TUNABLE] values (D-06). Project Settings > Game > Game Tuning; saved to DefaultGame.ini.
 * Features add properties under the categories Combat, Army, Focus, Respawn and Debug.
 * Per-content values belong in definition assets (UGameDefinition), not here.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Game Tuning"))
class CASTLEDEFENDER_API UGameTuningSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UGameTuningSettings* Get() { return GetDefault<UGameTuningSettings>(); }
};
