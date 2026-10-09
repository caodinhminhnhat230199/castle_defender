#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enemy/EnemyCharacter.h"
#include "SandboxEnemyRespawner.generated.h"

/** P0 sandbox only. Owns its spawned enemies and independent game-time respawn timers. */
UCLASS()
class CASTLEDEFENDER_API ASandboxEnemyRespawner : public AActor
{
	GENERATED_BODY()
public:
	ASandboxEnemyRespawner();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox")
	TObjectPtr<UEnemyArchetypeDefinition> EnemyDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox", meta = (ClampMin = "1", ClampMax = "3"))
	int32 Count = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox", meta = (ClampMin = "0"))
	float RespawnDelay = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox", meta = (ClampMin = "0"))
	float SpawnRadius = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sandbox")
	bool bEnabled = true;
	UFUNCTION(BlueprintCallable, Category = "Sandbox")
	void SetCount(int32 NewCount);
	UFUNCTION(BlueprintCallable, Category = "Sandbox")
	void SetEnabled(bool bNewEnabled);
	UFUNCTION(BlueprintPure, Category = "Sandbox")
	TArray<AEnemyCharacter*> GetTrackedEnemies() const;
	UFUNCTION(BlueprintPure, Category = "Sandbox")
	int32 GetAliveCount() const { return GetTrackedEnemies().Num(); }
	UFUNCTION(BlueprintPure, Category = "Sandbox")
	int32 GetPendingCount() const;

private:
	struct FSlot
	{
		TWeakObjectPtr<AEnemyCharacter> Enemy;
		FTimerHandle Timer;
	};
	TArray<FSlot> Slots;
	bool bEndingPlay = false;
	void Reconcile();
	void FillSlot(int32 Index);
	void ScheduleSlot(int32 Index);
	void RemoveSlot(int32 Index);
	UFUNCTION()
	void HandleEnemyRemoved(AEnemyCharacter* Enemy, EEnemyRemovedReason Reason);
};
