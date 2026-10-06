#include "Enemy/EnemyBrainComponent.h"
#include "Enemy/EnemyCharacter.h"
#include "Enemy/EnemyTargeting.h"
#include "AIController.h"
#include "Combat/HealthComponent.h"
#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Hero/HeroCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"
#include "VisualLogger/VisualLogger.h"

UEnemyBrainComponent::UEnemyBrainComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEnemyBrainComponent::StartDecisions()
{
	Enemy = Cast<AEnemyCharacter>(GetOwner());
	if (!Enemy || State == EEnemyBrainState::Dead) { return; }
	Enemy->GetHealthComponent()->OnDamaged.AddUniqueDynamic(this, &UEnemyBrainComponent::HandleDamaged);
	const float Interval = Enemy->GetRuntimeParams().DecisionInterval;
	// Random first fire spreads enemies spawned in the same frame across the interval.
	GetWorld()->GetTimerManager().SetTimer(DecisionTimer, this, &UEnemyBrainComponent::Decide, Interval, true, FMath::FRandRange(0.f, Interval));
}

void UEnemyBrainComponent::StopDecisions()
{
	if (UWorld* World = GetWorld()) { World->GetTimerManager().ClearTimer(DecisionTimer); }
	Target = nullptr;
	MoveGoal = nullptr;
	SetState(EEnemyBrainState::Dead);
}

void UEnemyBrainComponent::PauseDecisions()
{
	if (State == EEnemyBrainState::Dead) { return; }
	StopMoving();
	SetState(EEnemyBrainState::Paused);
}

void UEnemyBrainComponent::ResumeDecisions()
{
	// Re-evaluates on the next decision; the target is kept.
	if (State == EEnemyBrainState::Paused) { SetState(EEnemyBrainState::Idle); }
}

void UEnemyBrainComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UWorld* World = GetWorld()) { World->GetTimerManager().ClearTimer(DecisionTimer); }
	Super::EndPlay(Reason);
}

void UEnemyBrainComponent::Decide()
{
	if (!Enemy) { return; }
	++DecisionCount;
	if (State == EEnemyBrainState::Attacking || State == EEnemyBrainState::Staggered
		|| State == EEnemyBrainState::Paused || State == EEnemyBrainState::Dead)
	{
		return; // These states exit on events, not on the decision tick.
	}
	AActor* Current = Target.Get();
	if (Current && !IsValidTarget(Current)) { Current = nullptr; }
	if (!Current)
	{
		// Scan only without a valid combat target (R-ENM-10).
		Current = ScanForTarget();
		Target = Current;
		TargetReason = EEnemyTargetReason::LocalAggro;
	}
	if (Current)
	{
		SetState(EEnemyBrainState::Engage);
		ChaseTarget(Current);
	}
	else
	{
		StopMoving();
		SetState(EEnemyBrainState::Idle);
	}
	DrawDebug();
}

void UEnemyBrainComponent::SetState(EEnemyBrainState NewState)
{
	if (State == NewState) { return; }
	const EEnemyBrainState OldState = State;
	State = NewState;
	UE_VLOG(GetOwner(), LogGameAI, Log, TEXT("State %s -> %s"), *UEnum::GetDisplayValueAsText(OldState).ToString(), *UEnum::GetDisplayValueAsText(NewState).ToString());
	OnBrainStateChanged.Broadcast(this, OldState, NewState);
}

bool UEnemyBrainComponent::IsValidTarget(const AActor* Candidate) const
{
	if (!IsValid(Candidate) || Candidate->IsActorBeingDestroyed() || !AreHostile(GetOwner(), Candidate)) { return false; }
	const UHealthComponent* Health = Candidate->FindComponentByClass<UHealthComponent>();
	return Health && !Health->IsDead();
}

AActor* UEnemyBrainComponent::ScanForTarget() const
{
	const FEnemyRuntimeParams& Params = Enemy->GetRuntimeParams();
	const FVector Origin = Enemy->GetActorLocation();
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Query(SCENE_QUERY_STAT(EnemyAggroScan), false, Enemy);
	GetWorld()->OverlapMultiByObjectType(Overlaps, Origin, FQuat::Identity, FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Params.LocalAggroRadius), Query);

	TArray<FEnemyTargetCandidate, TInlineAllocator<8>> Candidates;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		if (!IsValidTarget(Actor) || Candidates.ContainsByPredicate([Actor](const FEnemyTargetCandidate& C) { return C.Actor == Actor; }))
		{
			continue;
		}
		FEnemyTargetCandidate& Candidate = Candidates.AddDefaulted_GetRef();
		Candidate.Actor = Actor;
		// Soldiers are the only other hostile pawns in the plan; SQD may classify them by unit tags later.
		Candidate.Kind = Actor->IsA<AHeroCharacter>() ? EEnemyTargetKind::Hero : EEnemyTargetKind::Soldier;
		Candidate.DistanceSq = FVector::DistSquared(Origin, Actor->GetActorLocation());
		Candidate.bRecentAttacker = LastAttacker.Get() == Actor;
	}
	const int32 Best = EnemyTargeting::PickTarget(Candidates, Params.TargetPriority);
	return Best == INDEX_NONE ? nullptr : Candidates[Best].Actor;
}

void UEnemyBrainComponent::ChaseTarget(AActor* Goal)
{
	AAIController* AI = Cast<AAIController>(Enemy->GetController());
	if (!AI) { return; }
	// A running MoveToActor follows its goal (the engine re-paths when the goal moves > 100 cm),
	// so re-issue only for a new goal or after the previous move ended.
	if (MoveGoal.Get() == Goal && AI->GetMoveStatus() != EPathFollowingStatus::Idle) { return; }
	float AcceptanceRadius = 0.f;
	for (const FEnemyAttackDefinition& Attack : Enemy->GetRuntimeParams().Attacks) { AcceptanceRadius = FMath::Max(AcceptanceRadius, Attack.Range); }
	MoveGoal = Goal;
	AI->MoveToActor(Goal, AcceptanceRadius);
}

void UEnemyBrainComponent::StopMoving()
{
	if (!MoveGoal.IsValid() && !MoveGoal.IsStale()) { return; }
	MoveGoal = nullptr;
	if (AAIController* AI = Enemy ? Cast<AAIController>(Enemy->GetController()) : nullptr) { AI->StopMovement(); }
}

void UEnemyBrainComponent::HandleDamaged(const FCombatHit& Hit, float /*NewHealth*/)
{
	if (AActor* Attacker = Hit.Instigator.Get())
	{
		LastAttacker = Attacker;
		LastAttackedTime = GetWorld()->GetTimeSeconds();
	}
}

void UEnemyBrainComponent::DrawDebug() const
{
#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
	if (GameDebug::CVarEnemy.GetValueOnGameThread() <= 0) { return; }
	const FEnemyRuntimeParams& Params = Enemy->GetRuntimeParams();
	const float Lifetime = Params.DecisionInterval; // Redrawn every decision; no Tick.
	const FVector Origin = Enemy->GetActorLocation();
	const AActor* Goal = Target.Get();
	DrawDebugString(GetWorld(), Origin + FVector(0.f, 0.f, 120.f),
		FString::Printf(TEXT("%s%s%s"), *UEnum::GetDisplayValueAsText(State).ToString(), Goal ? TEXT(" -> ") : TEXT(""), Goal ? *Goal->GetName() : TEXT("")),
		nullptr, FColor::White, Lifetime);
	DrawDebugCircle(GetWorld(), Origin, Params.LocalAggroRadius, 32, FColor::Orange, false, Lifetime, 0, 2.f, FVector::XAxisVector, FVector::YAxisVector, false);
	if (Goal) { DrawDebugLine(GetWorld(), Origin, Goal->GetActorLocation(), FColor::Red, false, Lifetime, 0, 2.f); }
#endif
}
