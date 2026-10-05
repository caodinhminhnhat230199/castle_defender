#include "Core/GameCheatManager.h"

#include "Combat/CombatStateComponent.h"
#include "Combat/TestDummy.h"
#include "Core/GameLog.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "Player/HeroPlayerController.h"

void UGameCheatManager::SpawnTestDummy(float Distance)
{
#if UE_WITH_CHEAT_MANAGER
	// Exec parsing passes 0 / None for missing arguments, so defaults live here.
	if (Distance <= 0.f)
	{
		Distance = 400.f;
	}
	APlayerController* PC = GetOuterAPlayerController();
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FRotator Facing(0.f, ViewRotation.Yaw, 0.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	const ATestDummy* Dummy = GetWorld()->SpawnActor<ATestDummy>(ViewLocation + Facing.Vector() * Distance, Facing.GetInverse(), Params);
	UE_LOG(LogGameCombat, Log, TEXT("SpawnTestDummy: %s"), Dummy ? *Dummy->GetName() : TEXT("failed"));
#endif
}

void UGameCheatManager::SetTimeDilation(float Value)
{
#if UE_WITH_CHEAT_MANAGER
	Slomo(Value);
#endif
}

void UGameCheatManager::DebugPushMode(const FString& Mode, FName Reason)
{
#if UE_WITH_CHEAT_MANAGER
	const int64 Value = StaticEnum<EPlayerMode>()->GetValueByNameString(Mode);
	AHeroPlayerController* PC = Cast<AHeroPlayerController>(GetOuterAPlayerController());
	if (!PC || Value == INDEX_NONE)
	{
		UE_LOG(LogGamePlayer, Warning, TEXT("DebugPushMode: needs AHeroPlayerController and a valid mode, got '%s'"), *Mode);
		return;
	}
	PC->PushMode(static_cast<EPlayerMode>(Value), Reason.IsNone() ? FName(TEXT("Cheat")) : Reason);
#endif
}

void UGameCheatManager::DebugPopMode(FName Reason)
{
#if UE_WITH_CHEAT_MANAGER
	if (AHeroPlayerController* PC = Cast<AHeroPlayerController>(GetOuterAPlayerController()))
	{
		PC->PopMode(Reason.IsNone() ? FName(TEXT("Cheat")) : Reason);
	}
#endif
}

UCombatStateComponent* UGameCheatManager::FindCrosshairCombatState() const
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetOuterAPlayerController();
	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CheatCrosshair), false, PC->GetPawn());
	TArray<FHitResult> Hits;
	GetWorld()->LineTraceMultiByChannel(Hits, ViewLocation, ViewLocation + ViewRotation.Vector() * 10000.f, ECC_Visibility, Params);
	for (const FHitResult& Hit : Hits)
	{
		if (UCombatStateComponent* State = Hit.GetActor() ? Hit.GetActor()->FindComponentByClass<UCombatStateComponent>() : nullptr)
		{
			return State;
		}
	}
	UE_LOG(LogGameCombat, Warning, TEXT("No actor with a UCombatStateComponent under the crosshair."));
#endif
	return nullptr;
}

void UGameCheatManager::SetPoise(float Value)
{
#if UE_WITH_CHEAT_MANAGER
	if (UCombatStateComponent* State = FindCrosshairCombatState())
	{
		State->SetPoise(Value);
		UE_LOG(LogGameCombat, Log, TEXT("SetPoise %s: %.0f / %.0f"), *GetNameSafe(State->GetOwner()), State->GetCurrentPoise(), State->GetMaxPoise());
	}
#endif
}

void UGameCheatManager::ApplyState(const FString& TagLeaf, float Duration)
{
#if UE_WITH_CHEAT_MANAGER
	const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*(TEXT("State.Combat.") + TagLeaf)), false);
	if (!Tag.IsValid())
	{
		UE_LOG(LogGameCombat, Warning, TEXT("ApplyState: unknown state State.Combat.%s"), *TagLeaf);
		return;
	}
	if (UCombatStateComponent* State = FindCrosshairCombatState())
	{
		State->ApplyState(Tag, Duration, GetOuterAPlayerController()->GetPawn());
	}
#endif
}

void UGameCheatManager::ClearStates()
{
#if UE_WITH_CHEAT_MANAGER
	if (UCombatStateComponent* State = FindCrosshairCombatState())
	{
		State->ClearAllStates();
	}
#endif
}
