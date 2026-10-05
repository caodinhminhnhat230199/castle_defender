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
	/** Spawns an ATestDummy in front of the view (default 400 cm). */
	UFUNCTION(Exec)
	void SpawnTestDummy(float Distance);

	/** Global time dilation for debugging. In gameplay only Tactical Focus changes it (D-20). */
	UFUNCTION(Exec)
	void SetTimeDilation(float Value);

	/** Pushes a player mode by name (Combat, Wheel, Build, Focus, Spirit, Modal). Reason defaults to Cheat. */
	UFUNCTION(Exec)
	void DebugPushMode(const FString& Mode, FName Reason);

	UFUNCTION(Exec)
	void DebugPopMode(FName Reason);

	/** Sets current poise on the crosshair target (T-SYN-01). */
	UFUNCTION(Exec)
	void SetPoise(float Value);

	/** Applies State.Combat.<TagLeaf> on the crosshair target; Duration 0 = Game Tuning default. */
	UFUNCTION(Exec)
	void ApplyState(const FString& TagLeaf, float Duration);

	/** Clears every combat state on the crosshair target. */
	UFUNCTION(Exec)
	void ClearStates();

private:
	/** First actor with a UCombatStateComponent under the crosshair (lock-on target once T-CMB-10 lands). */
	class UCombatStateComponent* FindCrosshairCombatState() const;
};
