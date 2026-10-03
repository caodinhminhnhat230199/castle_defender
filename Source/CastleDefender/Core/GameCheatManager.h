#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "GameCheatManager.generated.h"

/**
 * Game cheats. Not created in Shipping (UE_WITH_CHEAT_MANAGER), and bodies are compiled out.
 * The engine's God cheat is inherited; UHealthComponent honors it.
 * Features add their cheats here (SpawnEnemy, SpawnSquad, StartWave, DamageCore, KillHero, SkipPhase).
 */
UCLASS()
class CASTLEDEFENDER_API UGameCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** Spawns an ATestDummy in front of the view. */
	UFUNCTION(Exec)
	void SpawnTestDummy(float Distance = 400.f);

	/** Global time dilation for debugging. In gameplay only Tactical Focus changes it (D-20). */
	UFUNCTION(Exec)
	void SetTimeDilation(float Value);

	/** Pushes a player mode by name (Combat, Wheel, Build, Focus, Spirit, Modal). */
	UFUNCTION(Exec)
	void DebugPushMode(const FString& Mode, FName Reason = TEXT("Cheat"));

	UFUNCTION(Exec)
	void DebugPopMode(FName Reason = TEXT("Cheat"));
};
