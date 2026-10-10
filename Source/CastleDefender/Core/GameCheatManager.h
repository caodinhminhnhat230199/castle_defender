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
	/** Changes the enabled sandbox preset's owned enemy capacity (1-3). */
	UFUNCTION(Exec)
	void SetSandboxEnemyCount(int32 Count);
	/** Fixed telemetry smoke payload {x:1, s:a}; Event defaults to test. */
	UFUNCTION(Exec)
	void LogPlaytestEvent(FName Event);
	/** P0 debug spawn: archetype asset name and count (default DA_Enemy_Melee, 1). */
	UFUNCTION(Exec, BlueprintCallable, Category = "Debug|Enemy")
	void SpawnEnemy(FName Archetype, int32 Count);

	/** Spawns an ATestDummy in front of the pawn, along the view yaw (default 400 cm). */
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

	/** Reloads hero tuning from its class definition asset. */
	UFUNCTION(Exec)
	void ReloadHeroTuning();

	/** Toggles infinite stamina cheat on the controlled hero. */
	UFUNCTION(Exec)
	void InfiniteStamina();
	UFUNCTION(Exec)
	void KillHero();
	UFUNCTION(Exec)
	void ReportHeroWindows();
	UFUNCTION(Exec)
	void HealHero(float Amount = 40.f);

	/** Dispatches a synthetic FCombatHit against the controlled hero via DeliverHit. */
	UFUNCTION(Exec)
	void DebugHitHero(float Damage = 25.f, float Delay = 0.f, bool bFromFront = true);
	/** Simulated Army hit on the crosshair target, through the shared resolver (T-SYN-02). */
	UFUNCTION(Exec)
	void DebugHitTarget(float Damage = 20.f, float Poise = 0.f);
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
	/** Locked combat target first, otherwise crosshair target (pawn capsules respect world occlusion). */
	class UCombatStateComponent* FindCrosshairCombatState() const;
};
