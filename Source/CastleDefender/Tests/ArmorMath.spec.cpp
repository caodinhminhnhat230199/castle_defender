#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Combat/CombatLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/TestDummy.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Engine/World.h"
#include "Tests/CombatStateTestListener.h"

BEGIN_DEFINE_SPEC(FArmorMathSpec, "CastleDefender.Combat.Armor", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FArmorMathSpec)

void FArmorMathSpec::Define()
{
	It("reduces damage at 0, 0.5 and 0.9 armor, with and without Armor Broken", [this]()
	{
		for (const float Armor : { 0.f, 0.5f, 0.9f })
		{
			TestEqual("Normal armor", UHealthComponent::ComputeDamageAfterArmor(20.f, Armor, false, 0.25f), 20.f * (1.f - Armor), 0.001f);
			TestEqual("Broken armor", UHealthComponent::ComputeDamageAfterArmor(20.f, Armor, true, 0.25f), 20.f * (1.f - Armor * 0.25f), 0.001f);
		}
		TestEqual("Negative damage cannot heal", UHealthComponent::ComputeDamageAfterArmor(-20.f, 0.5f, true, 0.25f), 0.f);
		TestEqual("Armor capped", UHealthComponent::ComputeDamageAfterArmor(20.f, 2.f, false, 1.f), 2.f, 0.001f);
		TestEqual("Multiplier cannot amplify armor", UHealthComponent::ComputeDamageAfterArmor(20.f, 0.5f, true, 2.f), 10.f);
	});

	It("uses full armor on the applying Heavy, then benefits Hero, Army and Tower hits", [this]()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		World->InitializeActorsForPlay(FURL());
		ATestDummy* Target = World->SpawnActor<ATestDummy>();
		Target->DispatchBeginPlay();
		Target->GetHealth()->InitializeHealth(1000.f, 0.5f);
		FCombatHit Hit;
		Hit.Damage = 20.f;
		Hit.bIsHeavy = true;
		Hit.SourceLayer = ECombatLayer::Hero;
		Hit.AppliedStates.AddTag(GameTags::State_Combat_ArmorBroken);
		UCombatLibrary::DeliverHit(Target, Hit);
		TestEqual("Applying hit uses full armor", Target->GetHealth()->GetCurrentHealth(), 990.f);
		TestTrue("Shared state applied", Target->GetCombatState()->HasState(GameTags::State_Combat_ArmorBroken));
		TestEqual("Default duration from data", Target->GetCombatState()->GetStateRemaining(GameTags::State_Combat_ArmorBroken),
			UGameTuningSettings::Get()->StateDefaultDurations.FindChecked(GameTags::State_Combat_ArmorBroken), 0.001f);
		Hit.AppliedStates.Reset();
		Hit.bIsHeavy = false;
		for (const ECombatLayer Layer : { ECombatLayer::Hero, ECombatLayer::Army, ECombatLayer::Tower })
		{
			Hit.SourceLayer = Layer;
			const float Before = Target->GetHealth()->GetCurrentHealth();
			UCombatLibrary::DeliverHit(Target, Hit);
			TestEqual("Every layer benefits", Before - Target->GetHealth()->GetCurrentHealth(), 17.5f);
		}
		Target->GetCombatState()->RemoveState(GameTags::State_Combat_ArmorBroken);
		const float Before = Target->GetHealth()->GetCurrentHealth();
		UCombatLibrary::DeliverHit(Target, Hit);
		TestEqual("Original armor restored", Before - Target->GetHealth()->GetCurrentHealth(), 10.f);
		World->DestroyWorld(false);
	});

	It("refuses Armor Broken on unarmored targets without a state-added event", [this]()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
		World->InitializeActorsForPlay(FURL());
		ATestDummy* Target = World->SpawnActor<ATestDummy>();
		Target->DispatchBeginPlay();
		UCombatStateTestListener* Listener = NewObject<UCombatStateTestListener>();
		Target->GetCombatState()->OnStateAdded.AddDynamic(Listener, &UCombatStateTestListener::HandleAdded);
		FCombatHit Hit;
		Hit.Damage = 20.f;
		Hit.AppliedStates.AddTag(GameTags::State_Combat_ArmorBroken);
		UCombatLibrary::DeliverHit(Target, Hit);
		Target->GetCombatState()->ApplyState(GameTags::State_Combat_ArmorBroken, 6.f, nullptr);
		TestEqual("Full damage", Target->GetHealth()->GetCurrentHealth(), 80.f);
		TestFalse("No state", Target->GetCombatState()->HasState(GameTags::State_Combat_ArmorBroken));
		TestEqual("No state cue producer", Listener->AddedCount, 0);
		TestFalse("No expiry timer", Target->GetCombatState()->HasPendingExpiry());
		World->DestroyWorld(false);
	});
}
#endif
