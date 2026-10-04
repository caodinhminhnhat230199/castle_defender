#include "Combat/CombatLibrary.h"

#include "GenericTeamAgentInterface.h"
#include "Combat/CombatHitInterceptor.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"
#include "Core/GameLog.h"
#include "GameFramework/Actor.h"

ECombatHitResult UCombatLibrary::DeliverHit(AActor* Target, const FCombatHit& Hit)
{
	if (!Target || !IsValid(Target))
	{
		return ECombatHitResult::Ignored;
	}

	UHealthComponent* Health = Target->FindComponentByClass<UHealthComponent>();
	if (!Health || Health->IsDead())
	{
		return ECombatHitResult::Ignored;
	}

	// Hostility check: if instigator and target both have assigned teams and are on the same team, ignore hit
	if (Hit.Instigator.IsValid())
	{
		const FGenericTeamId InstigatorTeam = FGenericTeamId::GetTeamIdentifier(Hit.Instigator.Get());
		const FGenericTeamId TargetTeam = FGenericTeamId::GetTeamIdentifier(Target);
		if (InstigatorTeam != FGenericTeamId::NoTeam && TargetTeam != FGenericTeamId::NoTeam && InstigatorTeam == TargetTeam)
		{
			return ECombatHitResult::Ignored;
		}
	}

	// Begin resolution event record
	FCombatResolutionEvent Resolution;
	Resolution.ResolutionId = FGuid::NewGuid();
	Resolution.Instigator = Hit.Instigator;
	Resolution.Target = Target;
	Resolution.Hit = Hit;
	Resolution.Hit.bWasBlocked = false; // Outcome metadata is owned by this resolver, including terminated hits.
	Resolution.HitLocation = Hit.HitLocation.IsNearlyZero() ? Target->GetActorLocation() : Hit.HitLocation;
	Resolution.HitDirection = Hit.HitDirection;
	Resolution.bIsHeavy = Hit.bIsHeavy;

	UCombatStateComponent* CombatState = Target->FindComponentByClass<UCombatStateComponent>();
	const bool bArmorBroken = CombatState && CombatState->HasState(GameTags::State_Combat_ArmorBroken);
	Resolution.bTargetArmored = (Health->GetBaseArmor() > 0.f) && !bArmorBroken;

	FCombatHit WorkingHit = Hit;

	// Interceptor phase (defensive mitigation, parry, dodge i-frames)
	ICombatHitInterceptor* TargetInterceptor = FindHitInterceptor(Target);
	ECombatHitResult InterceptorResult = ECombatHitResult::Hit;
	if (TargetInterceptor)
	{
		InterceptorResult = TargetInterceptor->InterceptHit(WorkingHit);
	}

	// Defensive termination: Evaded, Parried, or Ignored terminate damage pipeline
	if (InterceptorResult == ECombatHitResult::Evaded || InterceptorResult == ECombatHitResult::Parried || InterceptorResult == ECombatHitResult::Ignored)
	{
		Resolution.Result = InterceptorResult;
		Resolution.DamageApplied = 0.f;
		Resolution.PoiseDamageApplied = 0.f;
		DispatchCombatResolution(Resolution);
		return InterceptorResult;
	}

	// Apply damage via HealthComponent
	WorkingHit.bWasBlocked = InterceptorResult == ECombatHitResult::Blocked || InterceptorResult == ECombatHitResult::BlockBroken;
	Resolution.Hit.bWasBlocked = WorkingHit.bWasBlocked;
	const float DamageDealt = Health->ApplyHit(WorkingHit);
	Resolution.DamageApplied = DamageDealt;

	ECombatHitResult FinalResult = InterceptorResult;
	if (Health->IsDead())
	{
		FinalResult = ECombatHitResult::Killed;
	}
	else
	{
		if (InterceptorResult == ECombatHitResult::Hit)
		{
			FinalResult = ECombatHitResult::Hit;
		}

		// If target survived, apply poise damage and status effects
		if (CombatState)
		{
			if (WorkingHit.PoiseDamage > 0.f)
			{
				CombatState->ApplyPoiseDamage(WorkingHit.PoiseDamage, WorkingHit.Instigator.Get());
				Resolution.PoiseDamageApplied = WorkingHit.PoiseDamage;
			}
			for (const FGameplayTag& StateTag : WorkingHit.AppliedStates)
			{
				CombatState->ApplyState(StateTag, 0.f, WorkingHit.Instigator.Get());
			}
		}
	}

	Resolution.Result = FinalResult;
	DispatchCombatResolution(Resolution);

	return FinalResult;
}

bool UCombatLibrary::IsInFrontArc(const AActor* Defender, const FVector& AttackerLocation, float ArcDegrees)
{
	if (!Defender)
	{
		return false;
	}

	if (ArcDegrees <= 0.f)
	{
		return false;
	}

	if (ArcDegrees >= 360.f)
	{
		return true;
	}

	const FVector DefenderForward = Defender->GetActorForwardVector().GetSafeNormal2D();
	if (DefenderForward.IsNearlyZero())
	{
		return false;
	}

	const FVector ToAttacker = (AttackerLocation - Defender->GetActorLocation()).GetSafeNormal2D();
	if (ToAttacker.IsNearlyZero())
	{
		return true;
	}

	const float Dot = FVector::DotProduct(DefenderForward, ToAttacker);
	const float HalfAngleRad = FMath::DegreesToRadians(ArcDegrees * 0.5f);
	const float CosHalfAngle = FMath::Cos(HalfAngleRad);

	return Dot >= (CosHalfAngle - KINDA_SMALL_NUMBER);
}

ICombatHitInterceptor* UCombatLibrary::FindHitInterceptor(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	if (ICombatHitInterceptor* Interceptor = Cast<ICombatHitInterceptor>(Actor))
	{
		return Interceptor;
	}

	for (UActorComponent* Comp : Actor->GetComponents())
	{
		if (ICombatHitInterceptor* CompInterceptor = Cast<ICombatHitInterceptor>(Comp))
		{
			return CompInterceptor;
		}
	}

	for (UActorComponent* Comp : Actor->GetInstanceComponents())
	{
		if (ICombatHitInterceptor* CompInterceptor = Cast<ICombatHitInterceptor>(Comp))
		{
			return CompInterceptor;
		}
	}

	return nullptr;
}

void UCombatLibrary::DispatchCombatResolution(const FCombatResolutionEvent& Event)
{
	ICombatHitInterceptor* InstigatorInterceptor = FindHitInterceptor(Event.Instigator.Get());
	ICombatHitInterceptor* TargetInterceptor = FindHitInterceptor(Event.Target.Get());

	if (InstigatorInterceptor)
	{
		InstigatorInterceptor->NotifyCombatResolved(Event);
	}

	if (TargetInterceptor && TargetInterceptor != InstigatorInterceptor)
	{
		TargetInterceptor->NotifyCombatResolved(Event);
	}
}
