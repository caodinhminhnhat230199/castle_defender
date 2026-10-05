#include "Core/GameCheatManager.h"

#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatActionTiming.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/TestDummy.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Engine/World.h"
#include "Hero/HeroCharacter.h"
#include "Hero/StaminaComponent.h"
#include "Player/HeroPlayerController.h"
#include "TimerManager.h"

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

	// Measure from the pawn, not the camera: the third-person camera sits a boom length behind the hero,
	// so a camera-relative spawn lands on top of the hero.
	const APawn* Pawn = PC->GetPawn();
	const FVector Origin = Pawn ? Pawn->GetActorLocation() : ViewLocation;
	const FRotator Facing(0.f, ViewRotation.Yaw, 0.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	const ATestDummy* Dummy = GetWorld()->SpawnActor<ATestDummy>(Origin + Facing.Vector() * Distance, Facing.GetInverse(), Params);
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

void UGameCheatManager::ReloadHeroTuning()
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC)
	{
		return;
	}

	if (AHeroCharacter* Hero = Cast<AHeroCharacter>(PC->GetPawn()))
	{
		Hero->ApplyTuning();
		UE_LOG(LogGamePlayer, Log, TEXT("ReloadHeroTuning: applied tuning to hero '%s'"), *Hero->GetName());
	}
	else
	{
		UE_LOG(LogGamePlayer, Warning, TEXT("ReloadHeroTuning: controlled pawn is not an AHeroCharacter"));
	}
#endif
}

void UGameCheatManager::InfiniteStamina()
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC)
	{
		return;
	}

	if (AHeroCharacter* Hero = Cast<AHeroCharacter>(PC->GetPawn()))
	{
		if (UStaminaComponent* Stamina = Hero->GetStaminaComponent())
		{
			const bool bNewState = !Stamina->HasInfiniteStamina();
			Stamina->SetInfiniteStamina(bNewState);
			UE_LOG(LogGamePlayer, Log, TEXT("InfiniteStamina set to %s on '%s'"), bNewState ? TEXT("true") : TEXT("false"), *Hero->GetName());
		}
	}
	else
	{
		UE_LOG(LogGamePlayer, Warning, TEXT("InfiniteStamina: controlled pawn is not an AHeroCharacter"));
	}
#endif
}

void UGameCheatManager::KillHero()
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetOuterAPlayerController();
	if (AHeroCharacter* Hero = PC ? Cast<AHeroCharacter>(PC->GetPawn()) : nullptr)
	{
		FCombatHit Hit;
		Hit.Damage = Hero->GetHealthComponent()->GetCurrentHealth();
		Hit.SourceLayer = ECombatLayer::Environment;
		UCombatLibrary::DeliverHit(Hero, Hit);
	}
#endif
}

void UGameCheatManager::ReportHeroWindows()
{
#if UE_WITH_CHEAT_MANAGER && !UE_BUILD_SHIPPING
	const APlayerController* PC = GetOuterAPlayerController();
	const AHeroCharacter* Hero = PC ? Cast<AHeroCharacter>(PC->GetPawn()) : nullptr;
	const UHeroClassDefinition* Def = Hero ? Hero->GetHeroClassDefinition() : nullptr;
	if (!Def) { UE_LOG(LogGameCombat, Warning, TEXT("ReportHeroWindows: controlled hero definition missing.")); return; }
	auto Report = [](const FString& Action, const UAnimMontage* Montage)
	{
		FCombatActionTiming Timing;
		FString Error;
		const bool bValid = FCombatActionTiming::InspectMontage(Montage, Timing, &Error);
		UE_LOG(LogGameCombat, Log, TEXT("ReportHeroWindows: %s / %s duration %.3f %s %s"),
			*Action, *GetNameSafe(Montage), Timing.TotalDuration, bValid ? TEXT("VALID") : TEXT("INVALID"), *Error);
		if (!Montage) { return; }
		for (const FAnimNotifyEvent& Event : Montage->Notifies)
		{
			if (!Event.NotifyStateClass) { continue; }
			UE_LOG(LogGameCombat, Log, TEXT("  %s window %s [%.3f, %.3f]"), *Action,
				*GetNameSafe(Event.NotifyStateClass), Event.GetTime(), Event.GetTime() + Event.GetDuration());
		}
	};
	for (int32 Index = 0; Index < Def->LightChain.Num(); ++Index) { Report(FString::Printf(TEXT("Light[%d]"), Index), Def->LightChain[Index].Montage); }
	Report(TEXT("Heavy"), Def->Heavy.Montage);
	for (EHeroDodgeDirection Direction : {EHeroDodgeDirection::Forward, EHeroDodgeDirection::Backward, EHeroDodgeDirection::Left, EHeroDodgeDirection::Right})
	{
		Report(UEnum::GetValueAsString(Direction), Def->Dodge.GetMontage(Direction));
	}
	Report(TEXT("HitReact Front"), Def->HitReact.FrontMontage);
	Report(TEXT("HitReact Back"), Def->HitReact.BackMontage);
	Report(TEXT("Death"), Def->HitReact.DeathMontage);
	UE_LOG(LogGameCombat, Log, TEXT("%s"), *Hero->GetCombatComponent()->GetCombatDebugString());
#endif
}

void UGameCheatManager::DebugHitHero(float Damage, float Delay, bool bFromFront)
{
#if UE_WITH_CHEAT_MANAGER
	APlayerController* PC = GetOuterAPlayerController();
	if (!PC)
	{
		return;
	}

	AHeroCharacter* Hero = Cast<AHeroCharacter>(PC->GetPawn());
	if (!Hero)
	{
		UE_LOG(LogGamePlayer, Warning, TEXT("DebugHitHero: controlled pawn is not an AHeroCharacter"));
		return;
	}

	if (Damage <= 0.f)
	{
		Damage = 25.f;
	}

	auto ExecuteHit = [Hero, Damage, bFromFront]()
	{
		if (!IsValid(Hero))
		{
			return;
		}

		const FVector HeroForward = Hero->GetActorForwardVector();
		const FVector AttackerLocation = Hero->GetActorLocation() + (bFromFront ? HeroForward * 200.f : -HeroForward * 200.f);

		FCombatHit Hit;
		Hit.Damage = Damage;
		Hit.PoiseDamage = 10.f;
		Hit.DamageType = GameTags::Damage_Physical;
		Hit.SourceLayer = ECombatLayer::Enemy;
		Hit.HitLocation = AttackerLocation;
		Hit.HitDirection = (Hero->GetActorLocation() - AttackerLocation).GetSafeNormal();

		const ECombatHitResult Result = UCombatLibrary::DeliverHit(Hero, Hit);
		UE_LOG(LogGamePlayer, Log, TEXT("DebugHitHero: delivered %f dmg (FromFront: %d), outcome: %s"),
			Damage, bFromFront ? 1 : 0, *UEnum::GetValueAsString(Result));
	};

	if (Delay > 0.f && Hero->GetWorld())
	{
		FTimerHandle Handle;
		Hero->GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda(ExecuteHit), Delay, false);
	}
	else
	{
		ExecuteHit();
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
