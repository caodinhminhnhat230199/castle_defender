#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/CombatLibrary.h"
#include "Combat/CombatTypes.h"
#include "Combat/TestDummy.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Core/GameTags.h"
#include "Tests/CombatTestListener.h"

BEGIN_DEFINE_SPEC(FCombatResolutionSpec, "CastleDefender.Combat.Resolution", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	ATestDummy* Attacker = nullptr;
	ATestDummy* Defender = nullptr;
END_DEFINE_SPEC(FCombatResolutionSpec)

void FCombatResolutionSpec::Define()
{
	BeforeEach([this]()
	{
		Attacker = NewObject<ATestDummy>();
		Attacker->SetGenericTeamId(FGenericTeamId(Team_Player));
		Attacker->SetActorLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
		if (UHealthComponent* AttHealth = Attacker->FindComponentByClass<UHealthComponent>())
		{
			AttHealth->InitializeHealth(100.f, 0.f);
		}

		Defender = NewObject<ATestDummy>();
		Defender->SetGenericTeamId(FGenericTeamId(Team_Enemy));
		Defender->SetActorLocationAndRotation(FVector(100.f, 0.f, 0.f), FRotator::ZeroRotator);
		if (UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>())
		{
			DefHealth->InitializeHealth(100.f, 0.f);
		}
	});

	Describe("DeliverHit Core Gate (AC-CMB-13)", [this]()
	{
		It("ignores hits on null target or dead target", [this]()
		{
			FCombatHit Hit;
			Hit.Damage = 25.f;
			Hit.Instigator = Attacker;

			TestEqual("Null target ignored", UCombatLibrary::DeliverHit(nullptr, Hit), ECombatHitResult::Ignored);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestNotNull("Defender health valid", DefHealth);

			// Kill defender
			FCombatHit LethalHit;
			LethalHit.Damage = DefHealth->GetMaxHealth();
			LethalHit.Instigator = Attacker;
			TestEqual("Lethal hit kills target", UCombatLibrary::DeliverHit(Defender, LethalHit), ECombatHitResult::Killed);
			TestTrue("Defender dead", DefHealth->IsDead());

			// Subsequent hit is ignored
			TestEqual("Dead target ignored", UCombatLibrary::DeliverHit(Defender, Hit), ECombatHitResult::Ignored);
		});

		It("ignores hits between non-hostile actors", [this]()
		{
			// Set defender to same team as attacker
			Defender->SetGenericTeamId(FGenericTeamId(Team_Player));

			FCombatHit Hit;
			Hit.Damage = 30.f;
			Hit.Instigator = Attacker;

			TestEqual("Friendly hit ignored", UCombatLibrary::DeliverHit(Defender, Hit), ECombatHitResult::Ignored);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender health untouched", DefHealth->GetCurrentHealth(), DefHealth->GetMaxHealth());
		});

		It("applies damage and reports Hit or Killed when hostile", [this]()
		{
			FCombatHit NormalHit;
			NormalHit.Damage = 35.f;
			NormalHit.Instigator = Attacker;

			TestEqual("Damage outcome is Hit", UCombatLibrary::DeliverHit(Defender, NormalHit), ECombatHitResult::Hit);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender health reduced", DefHealth->GetCurrentHealth(), 65.f);

			FCombatHit FinishingHit;
			FinishingHit.Damage = 70.f;
			FinishingHit.Instigator = Attacker;

			TestEqual("Lethal outcome is Killed", UCombatLibrary::DeliverHit(Defender, FinishingHit), ECombatHitResult::Killed);
			TestTrue("Defender is dead", DefHealth->IsDead());
		});

		It("applies poise damage and status effects to surviving targets", [this]()
		{
			UCombatStateComponent* StateComp = NewObject<UCombatStateComponent>(Defender);
			Defender->AddInstanceComponent(StateComp);

			FCombatHit StatusHit;
			StatusHit.Damage = 20.f;
			StatusHit.PoiseDamage = 15.f;
			StatusHit.Instigator = Attacker;
			StatusHit.AppliedStates.AddTag(GameTags::State_Combat_ArmorBroken);

			TestEqual("Hit outcome", UCombatLibrary::DeliverHit(Defender, StatusHit), ECombatHitResult::Hit);
			TestTrue("ArmorBroken state applied", StateComp->HasState(GameTags::State_Combat_ArmorBroken));
		});
	});

	Describe("Defensive Interception (ICombatHitInterceptor)", [this]()
	{
		It("respects Evaded and prevents damage", [this]()
		{
			UMockHitInterceptorComponent* Interceptor = NewObject<UMockHitInterceptorComponent>(Defender);
			Defender->AddInstanceComponent(Interceptor);
			Interceptor->ResponseResult = ECombatHitResult::Evaded;

			FCombatHit Hit;
			Hit.Damage = 50.f;
			Hit.Instigator = Attacker;

			TestEqual("Hit result is Evaded", UCombatLibrary::DeliverHit(Defender, Hit), ECombatHitResult::Evaded);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender health unchanged", DefHealth->GetCurrentHealth(), DefHealth->GetMaxHealth());
			TestEqual("Interceptor was invoked", Interceptor->InterceptCount, 1);
			TestEqual("Resolution notified", Interceptor->ResolutionNotificationCount, 1);
			TestEqual("Damage applied in resolution is 0", Interceptor->LastResolution.DamageApplied, 0.f);
		});

		It("respects Parried and prevents damage", [this]()
		{
			UMockHitInterceptorComponent* Interceptor = NewObject<UMockHitInterceptorComponent>(Defender);
			Defender->AddInstanceComponent(Interceptor);
			Interceptor->ResponseResult = ECombatHitResult::Parried;

			FCombatHit Hit;
			Hit.Damage = 50.f;
			Hit.Instigator = Attacker;

			TestEqual("Hit result is Parried", UCombatLibrary::DeliverHit(Defender, Hit), ECombatHitResult::Parried);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender health unchanged", DefHealth->GetCurrentHealth(), DefHealth->GetMaxHealth());
			TestEqual("Interceptor was invoked", Interceptor->InterceptCount, 1);
			TestEqual("Resolution notified", Interceptor->ResolutionNotificationCount, 1);
			TestEqual("Damage applied in resolution is 0", Interceptor->LastResolution.DamageApplied, 0.f);
		});

		It("respects Blocked and applies scaled damage", [this]()
		{
			UMockHitInterceptorComponent* Interceptor = NewObject<UMockHitInterceptorComponent>(Defender);
			Defender->AddInstanceComponent(Interceptor);
			Interceptor->ResponseResult = ECombatHitResult::Blocked;
			Interceptor->DamageScale = 0.4f; // 60% damage reduction

			FCombatHit Hit;
			Hit.Damage = 50.f;
			Hit.Instigator = Attacker;

			TestEqual("Hit result is Blocked", UCombatLibrary::DeliverHit(Defender, Hit), ECombatHitResult::Blocked);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender took reduced damage", DefHealth->GetCurrentHealth(), 80.f);
			TestEqual("Resolution notified with scaled damage", Interceptor->LastResolution.DamageApplied, 20.f);
		});
	});

	Describe("One-Hit-Per-Target-Per-Swing (AC-CMB-01)", [this]()
	{
		It("hits each target at most once per active window and resets on new window", [this]()
		{
			UMeleeTraceComponent* TraceComp = NewObject<UMeleeTraceComponent>(Attacker);
			Attacker->AddInstanceComponent(TraceComp);

			FCombatHit SwingHit;
			SwingHit.Damage = 25.f;
			SwingHit.Instigator = Attacker;
			TraceComp->SetPendingAttack(SwingHit);

			UCombatTestListener* Listener = NewObject<UCombatTestListener>();
			TraceComp->OnHitResolved.AddDynamic(Listener, &UCombatTestListener::HandleHitResolved);

			// Activate swing window
			TraceComp->BeginHitWindow();
			TestTrue("Hit window active", TraceComp->IsHitWindowActive());

			// Overlap candidate 1
			const bool bHit1 = TraceComp->TryHitTarget(Defender, FVector(100.f, 0.f, 0.f));
			TestTrue("Defender hit on first overlap", bHit1);

			UHealthComponent* DefHealth = Defender->FindComponentByClass<UHealthComponent>();
			TestEqual("Defender hit once (75 HP)", DefHealth->GetCurrentHealth(), 75.f);
			TestEqual("Hit callback fired once", Listener->HitResolvedCount, 1);
			TestEqual("AlreadyHit contains defender", TraceComp->GetAlreadyHitActors().Num(), 1);

			// Overlap candidate 2 (same swing window)
			const bool bHit2 = TraceComp->TryHitTarget(Defender, FVector(100.f, 0.f, 0.f));
			TestFalse("Defender rejected on second overlap in same window", bHit2);
			TestEqual("Defender still hit only once (still 75 HP)", DefHealth->GetCurrentHealth(), 75.f);
			TestEqual("Hit callback not repeated", Listener->HitResolvedCount, 1);

			// Overlap candidate 3 (same swing window)
			const bool bHit3 = TraceComp->TryHitTarget(Defender, FVector(100.f, 0.f, 0.f));
			TestFalse("Defender rejected on third overlap in same window", bHit3);
			TestEqual("Defender still 75 HP", DefHealth->GetCurrentHealth(), 75.f);
			TestEqual("Hit callback not repeated", Listener->HitResolvedCount, 1);

			// End swing window
			TraceComp->EndHitWindow();
			TestFalse("Hit window closed", TraceComp->IsHitWindowActive());

			// Next swing window
			TraceComp->BeginHitWindow();
			TestEqual("AlreadyHit set cleared on new window", TraceComp->GetAlreadyHitActors().Num(), 0);

			const bool bHitNext = TraceComp->TryHitTarget(Defender, FVector(100.f, 0.f, 0.f));
			TestTrue("Defender hit on next swing window", bHitNext);
			TestEqual("Defender hit a second time on next swing (50 HP)", DefHealth->GetCurrentHealth(), 50.f);
			TestEqual("Hit callback fired for second swing", Listener->HitResolvedCount, 2);
		});
	});

	AfterEach([this]()
	{
		Attacker = nullptr;
		Defender = nullptr;
	});
}

#endif
