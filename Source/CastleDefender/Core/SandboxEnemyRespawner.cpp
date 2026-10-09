#include "Core/SandboxEnemyRespawner.h"
#include "Core/GameLog.h"
#include "Combat/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

ASandboxEnemyRespawner::ASandboxEnemyRespawner()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SpawnOrigin")));
}

void ASandboxEnemyRespawner::BeginPlay()
{
	Super::BeginPlay();
	Count = FMath::Clamp(Count, 1, 3);
	FString Error;
	if (!EnemyDefinition || !EnemyDefinition->ValidateDefinition(Error) ||
		!FMath::IsFinite(RespawnDelay) || RespawnDelay < 0.f ||
		!FMath::IsFinite(SpawnRadius) || SpawnRadius < 0.f)
	{
		UE_LOG(LogGameCombat, Error, TEXT("%s: invalid sandbox respawner data: %s"), *GetName(), *Error);
		bEnabled = false;
	}
	Reconcile();
}

void ASandboxEnemyRespawner::SetCount(int32 NewCount)
{
	Count = FMath::Clamp(NewCount, 1, 3);
	if (HasActorBegunPlay()) { Reconcile(); }
}

void ASandboxEnemyRespawner::SetEnabled(bool bNewEnabled)
{
	bEnabled = bNewEnabled;
	if (HasActorBegunPlay()) { Reconcile(); }
}

void ASandboxEnemyRespawner::Reconcile()
{
	if (bEndingPlay) { return; }
	const int32 Desired = bEnabled ? Count : 0;
	while (Slots.Num() > Desired)
	{
		RemoveSlot(Slots.Num() - 1);
		Slots.Pop();
	}
	const int32 Previous = Slots.Num();
	Slots.SetNum(Desired);
	for (int32 Index = Previous; Index < Desired; ++Index) { FillSlot(Index); }
}

void ASandboxEnemyRespawner::FillSlot(int32 Index)
{
	if (bEndingPlay || !bEnabled || !Slots.IsValidIndex(Index) || Slots[Index].Enemy.IsValid() || !EnemyDefinition) { return; }
	const AEnemyCharacter* DefaultEnemy = EnemyDefinition->EnemyClass.GetDefaultObject();
	if (!DefaultEnemy) { return; }
	FVector Location = GetActorLocation();
	if (SpawnRadius > 0.f)
	{
		UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		ANavigationData* NavData = Navigation ? Navigation->GetNavDataForProps(DefaultEnemy->GetNavAgentPropertiesRef(), Location) : nullptr;
		FNavLocation Point;
		if (!NavData || !Navigation->GetRandomPointInNavigableRadius(Location, SpawnRadius, Point, NavData))
		{
			UE_LOG(LogGameCombat, Log, TEXT("%s slot=%d waiting for navigable spawn at %.3f"), *GetName(), Index, GetWorld()->GetTimeSeconds());
			ScheduleSlot(Index); // Navigation may still be loading; keep this capacity pending.
			return;
		}
		Location = Point.Location;
	}
	Location.Z += DefaultEnemy->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FTransform Transform(GetActorRotation(), Location);
	AEnemyCharacter* Enemy = GetWorld()->SpawnActorDeferred<AEnemyCharacter>(EnemyDefinition->EnemyClass, Transform,
		this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Enemy) { ScheduleSlot(Index); return; }
	Slots[Index].Enemy = Enemy;
	Enemy->OnEnemyRemoved.AddDynamic(this, &ASandboxEnemyRespawner::HandleEnemyRemoved);
	Enemy->InitFromSpawn(EnemyDefinition, FEnemySpawnParams());
	Enemy->FinishSpawning(Transform);
	UE_LOG(LogGameCombat, Log, TEXT("%s slot=%d spawned %s at %.3f"), *GetName(), Index, *Enemy->GetName(), GetWorld()->GetTimeSeconds());
}

void ASandboxEnemyRespawner::ScheduleSlot(int32 Index)
{
	if (bEndingPlay || !bEnabled || !Slots.IsValidIndex(Index)) { return; }
	FTimerDelegate Callback = FTimerDelegate::CreateUObject(this, &ASandboxEnemyRespawner::FillSlot, Index);
	if (RespawnDelay == 0.f) { Slots[Index].Timer = GetWorldTimerManager().SetTimerForNextTick(Callback); }
	else { GetWorldTimerManager().SetTimer(Slots[Index].Timer, Callback, RespawnDelay, false); }
}

void ASandboxEnemyRespawner::HandleEnemyRemoved(AEnemyCharacter* Enemy, EEnemyRemovedReason Reason)
{
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index].Enemy.Get(true) == Enemy)
		{
			UE_LOG(LogGameCombat, Log, TEXT("%s slot=%d removed %s reason=%d at %.3f"), *GetName(), Index, *Enemy->GetName(), static_cast<int32>(Reason), GetWorld()->GetTimeSeconds());
			Enemy->OnEnemyRemoved.RemoveDynamic(this, &ASandboxEnemyRespawner::HandleEnemyRemoved);
			Slots[Index].Enemy.Reset();
			ScheduleSlot(Index);
			return;
		}
	}
}

void ASandboxEnemyRespawner::RemoveSlot(int32 Index)
{
	GetWorldTimerManager().ClearTimer(Slots[Index].Timer);
	AEnemyCharacter* Enemy = Slots[Index].Enemy.Get();
	Slots[Index].Enemy.Reset(); // Detach before lifecycle callbacks.
	if (Enemy)
	{
		Enemy->OnEnemyRemoved.RemoveDynamic(this, &ASandboxEnemyRespawner::HandleEnemyRemoved);
		Enemy->Despawn();
	}
}

TArray<AEnemyCharacter*> ASandboxEnemyRespawner::GetTrackedEnemies() const
{
	TArray<AEnemyCharacter*> Result;
	for (const FSlot& Slot : Slots)
	{
		AEnemyCharacter* Enemy = Slot.Enemy.Get();
		if (Enemy && !Enemy->HasReportedRemoval() && !Enemy->GetHealthComponent()->IsDead()) { Result.Add(Enemy); }
	}
	return Result;
}

int32 ASandboxEnemyRespawner::GetPendingCount() const
{
	int32 Result = 0;
	for (const FSlot& Slot : Slots) { Result += GetWorldTimerManager().TimerExists(Slot.Timer) ? 1 : 0; }
	return Result;
}

void ASandboxEnemyRespawner::EndPlay(const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;
	for (int32 Index = Slots.Num() - 1; Index >= 0; --Index) { RemoveSlot(Index); }
	Slots.Empty();
	Super::EndPlay(Reason);
}
