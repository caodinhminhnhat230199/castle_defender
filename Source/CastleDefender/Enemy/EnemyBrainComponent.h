#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatTypes.h"
#include "EnemyBrainComponent.generated.h"

class AEnemyCharacter;
class UAnimMontage;

/** P0 uses Idle, Engage, Attacking, Paused and Dead; Staggered lands with T-ENM-04, route states in P1/P2. */
UENUM(BlueprintType)
enum class EEnemyBrainState : uint8 { Idle, FollowRoute, Engage, Attacking, Staggered, ReturnToRoute, Paused, Dead };

UENUM(BlueprintType)
enum class EEnemyTargetReason : uint8 { LocalAggro, PathObstacle, Objective };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FEnemyBrainStateChangedSignature, UEnemyBrainComponent*, Brain, EEnemyBrainState, OldState, EEnemyBrainState, NewState);

/** Enemy FSM (D-08): decides on a looping timer at the archetype's interval; never ticks. */
UCLASS(ClassGroup = (AI))
class CASTLEDEFENDER_API UEnemyBrainComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UEnemyBrainComponent();

	/** Called by the enemy once its runtime params are set. First decision after a random offset in [0, interval). */
	void StartDecisions();
	/** Terminal: clears the timer and enters Dead. */
	void StopDecisions();
	UFUNCTION(BlueprintCallable, Category = "Enemy|Brain")
	void PauseDecisions();
	UFUNCTION(BlueprintCallable, Category = "Enemy|Brain")
	void ResumeDecisions();

	/** One decision. The timer calls it; tests call it directly. */
	void Decide();

	UFUNCTION(BlueprintPure, Category = "Enemy|Brain")
	EEnemyBrainState GetState() const { return State; }
	UFUNCTION(BlueprintPure, Category = "Enemy|Brain")
	AActor* GetTarget() const { return Target.Get(); }
	EEnemyTargetReason GetTargetReason() const { return TargetReason; }
	int32 GetDecisionCount() const { return DecisionCount; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Brain")
	FEnemyBrainStateChangedSignature OnBrainStateChanged;

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	void SetState(EEnemyBrainState NewState);
	bool IsValidTarget(const AActor* Candidate) const;
	AActor* ScanForTarget() const;
	void ChaseTarget(AActor* Goal);
	/** Starts an attack in range and off cooldown; false leaves the caller chasing. */
	bool TryStartAttack(AActor& Goal);
	void HandleHitWindowBegin();
	void HandleAttackEnded(UAnimMontage* Montage, bool bInterrupted);
	void StopMoving();
	void DrawDebug() const;
	UFUNCTION()
	void HandleDamaged(const FCombatHit& Hit, float NewHealth);

	UPROPERTY()
	TObjectPtr<AEnemyCharacter> Enemy;
	FTimerHandle DecisionTimer;
	EEnemyBrainState State = EEnemyBrainState::Idle;
	TWeakObjectPtr<AActor> Target;
	EEnemyTargetReason TargetReason = EEnemyTargetReason::LocalAggro;
	TWeakObjectPtr<AActor> MoveGoal;
	/** NEW-ENM-3: the last actor that damaged this enemy wins priority ties. Time is kept for debug. */
	TWeakObjectPtr<AActor> LastAttacker;
	double LastAttackedTime = -1.0;
	int32 DecisionCount = 0;
	/** Per attack: game time its cooldown ends. */
	TArray<double> AttackReadyTimes;
	double NextAttackTime = 0.0;
	int32 ActiveAttack = INDEX_NONE;
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;
};
