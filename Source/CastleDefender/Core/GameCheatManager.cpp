#include "Core/GameCheatManager.h"

#include "Combat/TestDummy.h"
#include "Core/GameLog.h"
#include "Engine/World.h"
#include "Hero/HeroCharacter.h"
#include "Hero/StaminaComponent.h"
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

