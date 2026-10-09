#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/CombatTypes.h"
#include "Combat/HealthComponent.h"
#include "Components/ActorComponent.h"
#include "Combat/CombatHitInterceptor.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/MeleeTraceComponent.h"
#include "CombatTestListener.generated.h"

/** Test helper: counts combat delegate broadcasts (dynamic delegates need a UFUNCTION target). */
UCLASS(Transient)
class UCombatTestListener : public UObject
{
	GENERATED_BODY()

public:
	int32 DeathCount = 0;
	UHealthComponent* Health = nullptr;
	float NestedDamage = 0.f;
	float NestedApplied = -1.f;
	bool bDeadDuringDamage = false;
	bool bNestedHitSent = false;

	UFUNCTION()
	void HandleDamage(const FCombatHit& Hit, float RemainingHealth)
	{
		if (!bNestedHitSent)
		{
			bNestedHitSent = true;
			bDeadDuringDamage = Health->IsDead();
			FCombatHit NestedHit;
			NestedHit.Damage = NestedDamage;
			NestedApplied = Health->ApplyHit(NestedHit);
		}
	}

	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit)
	{
		++DeathCount;
		if (HeroCombat) { ActionStateAtDeath = HeroCombat->GetActionState(); }
	}
	UHeroCombatComponent* HeroCombat = nullptr;
	EHeroActionState ActionStateAtDeath = EHeroActionState::Idle;
	int32 FeedbackCount = 0;
	FGameplayTag LastFeedback;
	UFUNCTION()
	void HandleFeedback(FGameplayTag Tag, const FCombatHit& Hit) { ++FeedbackCount; LastFeedback = Tag; }

	int32 StaminaChangedCount = 0;
	float LastStaminaCurrent = 0.f;
	float LastStaminaMax = 0.f;
	int32 StaminaSpendFailedCount = 0;
	float LastFailedCost = 0.f;
	int32 StaminaDepletedCount = 0;

	UFUNCTION()
	void HandleStaminaChanged(float Current, float Max)
	{
		++StaminaChangedCount;
		LastStaminaCurrent = Current;
		LastStaminaMax = Max;
	}

	UFUNCTION()
	void HandleStaminaSpendFailed(float Cost)
	{
		++StaminaSpendFailedCount;
		LastFailedCost = Cost;
	}

	UFUNCTION()
	void HandleStaminaDepleted()
	{
		++StaminaDepletedCount;
	}

	int32 CombatResolvedCount = 0;
	FCombatResolutionEvent LastResolutionEvent;

	UFUNCTION()
	void HandleCombatResolved(const FCombatResolutionEvent& Event)
	{
		++CombatResolvedCount;
		LastResolutionEvent = Event;
	}

	int32 BlockBrokenCount = 0;
	int32 ParrySucceededCount = 0;
	AActor* ParryAttacker = nullptr;
	AActor* ReentrantParryTarget = nullptr;
	ECombatHitResult ReentrantParryResult = ECombatHitResult::Ignored;
	UFUNCTION()
	void HandleParrySucceeded(AActor* Attacker)
	{
		++ParrySucceededCount;
		ParryAttacker = Attacker;
		if (ReentrantParryTarget)
		{
			FCombatHit Hit; Hit.Damage = 20.f; Hit.Instigator = Attacker; Hit.SourceLayer = ECombatLayer::Enemy;
			ReentrantParryResult = UCombatLibrary::DeliverHit(ReentrantParryTarget, Hit);
		}
	}
	UMeleeTraceComponent* ReentrantCounterTrace = nullptr;
	UMeleeTraceComponent* TraceToCloseOnState = nullptr;
	bool bReopenTraceOnState = false;
	UFUNCTION()
	void HandleCloseTraceOnState(FGameplayTag State, AActor* Instigator)
	{
		if (TraceToCloseOnState)
		{
			TraceToCloseOnState->EndHitWindow();
			if (bReopenTraceOnState)
			{
				TraceToCloseOnState->GetOwner()->SetActorLocation(FVector(600.f, 0.f, 0.f));
				TraceToCloseOnState->BeginHitWindow();
			}
		}
	}
	AActor* ReentrantCounterTarget = nullptr;
	bool bCounterDamageSeen = false;
	UFUNCTION()
	void HandleCounterDamage(const FCombatHit& Hit, float RemainingHealth)
	{
		bCounterDamageSeen = Hit.bIsParryCounter;
		if (ReentrantCounterTrace && ReentrantCounterTarget) { ReentrantCounterTrace->TryHitTarget(ReentrantCounterTarget); }
	}

	UFUNCTION()
	void HandleBlockBroken(AActor* Attacker) { ++BlockBrokenCount; }

	int32 HitResolvedCount = 0;
	ECombatHitResult LastHitResolvedResult = ECombatHitResult::Ignored;

	UFUNCTION()
	void HandleHitResolved(AActor* Target, ECombatHitResult Result)
	{
		++HitResolvedCount;
		LastHitResolvedResult = Result;
	}
};

/** Mock interceptor for combat resolution tests. */
UCLASS(Transient)
class UMockHitInterceptorComponent : public UActorComponent, public ICombatHitInterceptor
{
	GENERATED_BODY()

public:
	ECombatHitResult ResponseResult = ECombatHitResult::Hit;
	float DamageScale = 1.0f;
	int32 InterceptCount = 0;
	int32 ResolutionNotificationCount = 0;
	FCombatResolutionEvent LastResolution;

	virtual ECombatHitResult InterceptHit(FCombatHit& Hit) override
	{
		++InterceptCount;
		Hit.Damage *= DamageScale;
		return ResponseResult;
	}

	virtual void NotifyCombatResolved(const FCombatResolutionEvent& Event) override
	{
		++ResolutionNotificationCount;
		LastResolution = Event;
	}
};

