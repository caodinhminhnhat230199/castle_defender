#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Combat/TestDummy.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/AnimNotifyState_CombatHitWindow.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroCombatLibrary.h"

BEGIN_DEFINE_SPEC(FAttackAssistSpec, "CastleDefender.Combat.Hero.AttackAssist", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FAttackAssistSpec)

void FAttackAssistSpec::Define()
{
	It("rejects targets outside the original intent cone or distance and invalid tuning", [this]()
	{
		FHeroAttackAssistData Data;
		TestTrue("Inside", Data.IsEligible(FVector(300.f, 100.f, 0.f), 0.f));
		TestFalse("Outside cone", Data.IsEligible(FVector(100.f, 300.f, 0.f), 0.f));
		TestFalse("Outside range", Data.IsEligible(FVector(401.f, 0.f, 0.f), 0.f));
		TestFalse("No direction", Data.IsEligible(FVector::ZeroVector, 0.f));
		Data.MaxAngle = -1.f;
		TestFalse("Invalid angle", Data.IsValid());
		Data.MaxAngle = 35.f;
		Data.RotationRate = 0.f;
		TestFalse("Disabled", Data.IsEligible(FVector(100.f, 0.f, 0.f), 0.f));
	});

	It("bounds speed and total yaw around the original intent including wraparound and actor hit stop", [this]()
	{
		FHeroAttackAssistData Data;
		Data.RotationRate = 10.f;
		TestEqual("Rate limit", Data.StepYaw(0.f, 30.f, 0.f, 0.1f), 1.f, 0.001f);
		TestEqual("Dilated rate", Data.StepYaw(0.f, 30.f, 0.f, 0.01f), 0.1f, 0.001f);
		TestEqual("Paused", Data.StepYaw(0.f, 30.f, 0.f, 0.f), 0.f);
		TestEqual("Total cap", Data.StepYaw(34.f, 90.f, 0.f, 5.f), 35.f, 0.001f);
		TestEqual("Wrap", Data.StepYaw(179.f, -170.f, 179.f, 0.1f), 180.f, 0.001f);
	});

	It("preserves authored assist timing and rejects duplicate or non-attack windows", [this]()
	{
		FHeroCombatFixture Fixture;
		UAnimMontage* Montage = DuplicateObject<UAnimMontage>(Fixture.Hero->GetHeroClassDefinition()->Heavy.Montage, GetTransientPackage());
		TestTrue("Add missing window", UHeroCombatLibrary::EnsureRotationAssistWindow(Montage));
		const int32 Count = Montage->Notifies.Num();
		TestTrue("Idempotent", UHeroCombatLibrary::EnsureRotationAssistWindow(Montage));
		TestEqual("No duplicate", Montage->Notifies.Num(), Count);
		FCombatActionTiming Timing;
		TestTrue("Valid", FCombatActionTiming::InspectMontage(Montage, Timing));
		TestTrue("Assist present", Timing.bHasRotationAssistWindow);
		UAnimMontage* NonAttack = DuplicateObject<UAnimMontage>(Montage, GetTransientPackage());
		NonAttack->Notifies.RemoveAll([](const FAnimNotifyEvent& Notify)
		{
			return Notify.NotifyStateClass && Notify.NotifyStateClass->IsA<UAnimNotifyState_CombatHitWindow>();
		});
		TestFalse("Assist on non-attack rejected", FCombatActionTiming::InspectMontage(NonAttack, Timing));
		for (const FAnimNotifyEvent& Notify : Montage->Notifies)
		{
			if (Notify.NotifyStateClass && Notify.NotifyStateClass->IsA<UAnimNotifyState_RotationAssist>())
			{
				const FAnimNotifyEvent Duplicate = Notify;
				Montage->Notifies.Add(Duplicate);
				break;
			}
		}
		TestFalse("Duplicate rejected", FCombatActionTiming::InspectMontage(Montage, Timing));
	});

	It("selects a live hostile and rotates without translation only while the window is open", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		ATestDummy* Dummy = Fixture.World->SpawnActor<ATestDummy>(FVector(250.f, 75.f, 0.f), FRotator::ZeroRotator);
		Dummy->DispatchBeginPlay();
		auto* Combat = Fixture.Hero->GetCombatComponent();
		Fixture.Hero->GetHeroClassDefinition()->AttackAssist.RotationRate = 10.f;
		TestTrue("Heavy starts", Combat->RequestAction(EHeroAction::Heavy));
		const FVector Location = Fixture.Hero->GetActorLocation();
		Combat->OpenRotationAssistWindow();
		TestEqual("Selected", Combat->GetAssistTarget(), static_cast<AActor*>(Dummy));
		Combat->TickComponent(0.1f, LEVELTICK_All, nullptr);
		TestEqual("Bounded turn", Fixture.Hero->GetActorRotation().Yaw, 1.0, 0.001);
		TestTrue("No pull", Fixture.Hero->GetActorLocation().Equals(Location));
		Combat->ForceCloseAllWindows();
		TestNull("Closed target", Combat->GetAssistTarget());
		Combat->TickComponent(0.1f, LEVELTICK_All, nullptr);
		TestEqual("No turn outside window", Fixture.Hero->GetActorRotation().Yaw, 1.0, 0.001);
	});

	It("rejects friendly and dead candidates and drops a target that leaves range", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		ATestDummy* Dummy = Fixture.World->SpawnActor<ATestDummy>(FVector(250.f, 75.f, 0.f), FRotator::ZeroRotator);
		Dummy->DispatchBeginPlay();
		auto* Combat = Fixture.Hero->GetCombatComponent();
		TestTrue("Heavy starts", Combat->RequestAction(EHeroAction::Heavy));
		Dummy->SetGenericTeamId(Fixture.Hero->GetGenericTeamId());
		Combat->OpenRotationAssistWindow();
		TestNull("Ally rejected", Combat->GetAssistTarget());
		Dummy->SetGenericTeamId(FGenericTeamId(1));
		Combat->OpenRotationAssistWindow();
		TestEqual("Hostile", Combat->GetAssistTarget(), static_cast<AActor*>(Dummy));
		Dummy->SetActorLocation(FVector(1000.f, 0.f, 0.f));
		Combat->TickComponent(0.1f, LEVELTICK_All, nullptr);
		TestNull("Range revalidated", Combat->GetAssistTarget());
		Dummy->SetActorLocation(FVector(250.f, 75.f, 0.f));
		Dummy->ApplyDebugHit(Dummy->GetHealth()->GetCurrentHealth());
		Combat->OpenRotationAssistWindow();
		TestNull("Dead rejected", Combat->GetAssistTarget());
	});

	It("closing assist preserves buffer expiry and shared Staggered closes both", [this]()
	{
		FHeroCombatFixture Fixture;
		Fixture.BeginPlay();
		auto* Combat = Fixture.Hero->GetCombatComponent();
		TestTrue("Heavy starts", Combat->RequestAction(EHeroAction::Heavy));
		Combat->OpenRotationAssistWindow();
		Combat->RequestAction(EHeroAction::Light); // committed Heavy buffers the request
		TestTrue("Buffered", Combat->HasBufferedInput());
		Combat->CloseRotationAssistWindow();
		TestTrue("Buffer clock still enabled", Combat->IsComponentTickEnabled());
		Combat->TickComponent(Fixture.Hero->GetHeroClassDefinition()->Input.InputBufferTime + 0.01f, LEVELTICK_All, nullptr);
		TestFalse("Expired", Combat->HasBufferedInput());
		Combat->OpenRotationAssistWindow();
		Fixture.Hero->GetCombatStateComponent()->ApplyState(GameTags::State_Combat_Staggered, 1.f, Fixture.Hero);
		TestNull("Stagger closes assist", Combat->GetAssistTarget());
		TestFalse("No work remains", Combat->IsComponentTickEnabled());
	});
}
#endif
