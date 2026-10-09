#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/EnemyTestFixture.h"
#include "UI/HeroVitalsWidget.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/StaminaComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatLibrary.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Tests/FeedbackTestListener.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"

BEGIN_DEFINE_SPEC(FHeroHUDSpec, "CastleDefender.Feedback.HeroHUD", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroHUDSpec)

void FHeroHUDSpec::Define()
{
	It("synchronizes once after binding a pawn that has not begun play yet", [this]()
	{
		FEnemyTestWorld F;
		UClass* WidgetClass = LoadClass<UHeroVitalsWidget>(nullptr, TEXT("/Game/CastleDefender/UI/WBP_HeroVitals.WBP_HeroVitals_C"));
		if (!TestNotNull("Authored vitals", WidgetClass)) { return; }
		UHeroVitalsWidget* Widget = NewObject<UHeroVitalsWidget>(F.World, WidgetClass);
		Widget->Initialize();
		AHeroCharacter* Hero = F.World->SpawnActor<AHeroCharacter>();
		Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(LoadObject<UHeroClassDefinition>(nullptr,
			TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
		F.World->InitializeActorsForPlay(FURL());
		Widget->BindHero(Hero); // Controller possession may precede Hero BeginPlay during startup.
		Hero->DispatchBeginPlay();
		F.World->GetTimerManager().Tick(0.01f);
		TestEqual("First frame HP ready", Widget->GetHealthFraction(), 1.f);
		TestEqual("First frame stamina ready", Widget->GetStaminaTargetFraction(), 1.f);
		Widget->BindHero(nullptr);
	});

	It("binds authored bars, detaches the old hero and resets values on rebinding", [this]()
	{
		FEnemyTestWorld F;
		UClass* WidgetClass = LoadClass<UHeroVitalsWidget>(nullptr, TEXT("/Game/CastleDefender/UI/WBP_HeroVitals.WBP_HeroVitals_C"));
		if (!TestNotNull("Authored vitals", WidgetClass)) { return; }
		UHeroVitalsWidget* Widget = NewObject<UHeroVitalsWidget>(F.World, WidgetClass);
		TestTrue("Initialized Blueprint tree", Widget->Initialize());
		auto SpawnHero = [&]()
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			AHeroCharacter* Hero = F.World->SpawnActor<AHeroCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(LoadObject<UHeroClassDefinition>(nullptr,
				TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
			Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,
				TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
			Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
			F.World->InitializeActorsForPlay(FURL());
			Hero->DispatchBeginPlay();
			return Hero;
		};
		AHeroCharacter* First = SpawnHero();
		Widget->BindHero(First);
		TestEqual("Full HP", Widget->GetHealthFraction(), 1.f);
		First->GetStaminaComponent()->TrySpend(25.f);
		TestEqual("Delegate target", Widget->GetStaminaTargetFraction(), 0.75f);
		FCombatHit Hit;
		Hit.Damage = 20.f;
		UCombatLibrary::DeliverHit(First, Hit);
		TestEqual("HP updates", Widget->GetHealthFraction(), 0.9f);
		AHeroCharacter* Second = SpawnHero();
		Widget->BindHero(Second);
		TestEqual("New owner", Widget->GetObservedHero(), Second);
		TestEqual("Respawn HP reset", Widget->GetHealthFraction(), 1.f);
		TestEqual("Respawn stamina reset", Widget->GetStaminaTargetFraction(), 1.f);
		UCombatLibrary::DeliverHit(First, Hit);
		First->GetStaminaComponent()->TrySpend(25.f);
		TestEqual("Old HP detached", Widget->GetHealthFraction(), 1.f);
		TestEqual("Old stamina detached", Widget->GetStaminaTargetFraction(), 1.f);
		UCombatLibrary::DeliverHit(Second, Hit);
		TestEqual("New damage observed", Widget->GetHealthFraction(), 0.9f);
		UFeedbackSubsystem* Feedback = F.World->GetSubsystem<UFeedbackSubsystem>();
		Feedback->SetFeedbackTable(UFeedbackTestListener::MakeTable({ FeedbackTags::Hero_StaminaInsufficient }));
		UFeedbackTestListener* Listener = NewObject<UFeedbackTestListener>();
		Feedback->OnFeedbackPlayed.AddDynamic(Listener, &UFeedbackTestListener::HandlePlayed);
		Second->GetStaminaComponent()->TrySpend(Second->GetStaminaComponent()->GetCurrentStamina());
		TestFalse("Cannot spend empty", Second->GetStaminaComponent()->TrySpend(20.f));
		TestEqual("One shared feedback event", Listener->PlayedCount, 1);
		TestEqual("Stamina failure row", Listener->LastPlayed, FeedbackTags::Hero_StaminaInsufficient.GetTag());
		Widget->BindHero(nullptr);
		TestEqual("Unpossessed hidden", Widget->GetVisibility(), ESlateVisibility::Collapsed);
	});

	It("applies vitals visibility without changing gameplay state or input mode", [this]()
	{
		FEnemyTestWorld F;
		UClass* WidgetClass = LoadClass<UHeroVitalsWidget>(nullptr, TEXT("/Game/CastleDefender/UI/WBP_HeroVitals.WBP_HeroVitals_C"));
		if (!TestNotNull("Authored vitals", WidgetClass)) { return; }
		UHeroVitalsWidget* Widget = NewObject<UHeroVitalsWidget>(F.World, WidgetClass);
		Widget->Initialize();
		AHeroCharacter* Hero = F.World->SpawnActor<AHeroCharacter>();
		Widget->BindHero(Hero);
		for (EHUDLayer Layer : { EHUDLayer::None, EHUDLayer::CommandWheel, EHUDLayer::Build, EHUDLayer::TacticalFocus })
		{
			Widget->ApplyLayer(Layer);
			TestEqual("Visible layer", Widget->GetVisibility(), ESlateVisibility::HitTestInvisible);
		}
		TestEqual("Focus dim", Widget->GetRenderOpacity(), 0.4f);
		Widget->ApplyLayer(EHUDLayer::Modal);
		TestEqual("Modal hides", Widget->GetVisibility(), ESlateVisibility::Collapsed);
		Widget->ApplyLayer(EHUDLayer::CommanderSpirit);
		TestEqual("Spirit hides", Widget->GetVisibility(), ESlateVisibility::Collapsed);
		Widget->ApplyLayer(EHUDLayer::None);
		TestEqual("Combat restores", Widget->GetRenderOpacity(), 1.f);
		Widget->BindHero(nullptr);
	});

	It("latches low HP once per crossing, clears above re-arm threshold, and triggers damage flash", [this]()
	{
		FEnemyTestWorld F;
		UClass* WidgetClass = LoadClass<UHeroVitalsWidget>(nullptr, TEXT("/Game/CastleDefender/UI/WBP_HeroVitals.WBP_HeroVitals_C"));
		if (!TestNotNull("Authored vitals", WidgetClass)) { return; }
		UHeroVitalsWidget* Widget = NewObject<UHeroVitalsWidget>(F.World, WidgetClass);
		TestTrue("Initialized Blueprint tree", Widget->Initialize());

		UFeedbackSubsystem* Feedback = F.World->GetSubsystem<UFeedbackSubsystem>();
		Feedback->SetFeedbackTable(UFeedbackTestListener::MakeTable({ FeedbackTags::Hero_LowHealth, FeedbackTags::Hero_Damaged, FeedbackTags::Combat_Hit_Light, FeedbackTags::Combat_Hit_Heavy, FeedbackTags::Hero_Death }));
		UFeedbackTestListener* Listener = NewObject<UFeedbackTestListener>();
		Feedback->OnFeedbackPlayed.AddDynamic(Listener, &UFeedbackTestListener::HandlePlayed);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AHeroCharacter* Hero = F.World->SpawnActor<AHeroCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
		Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(LoadObject<UHeroClassDefinition>(nullptr,
			TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
		Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
		Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
		F.World->InitializeActorsForPlay(FURL());
		Hero->DispatchBeginPlay();
		Hero->GetHealthComponent()->InitializeHealth(100.f, 0.f);

		Widget->BindHero(Hero);
		TestFalse("Initial latch clear", Widget->IsLowHealthLatched());

		// Damage Hero to 25% (100 -> 25)
		FCombatHit Hit1;
		Hit1.Damage = 75.f;
		UCombatLibrary::DeliverHit(Hero, Hit1);

		TestTrue("Low health latched at 25%", Widget->IsLowHealthLatched());
		TestTrue("Damage flash active", Widget->IsDamageFlashActive());
		TestTrue("Vignette opacity active", Widget->GetDamageVignetteOpacity() > 0.f);

		// Damage flash clears after duration
		FTSTicker::GetCoreTicker().Tick(0.3f);
		TestFalse("Damage flash cleared after duration", Widget->IsDamageFlashActive());
		TestTrue("Low health pulse still active", Widget->GetDamageVignetteOpacity() > 0.f);

		// More damage to 15% (25 -> 15)
		FCombatHit Hit2;
		Hit2.Damage = 10.f;
		UCombatLibrary::DeliverHit(Hero, Hit2);

		TestTrue("Remains latched at 15%", Widget->IsLowHealthLatched());
		int32 LowHealthCount = 0;
		for (const FGameplayTag& Tag : Listener->PlayedTags)
		{
			if (Tag == FeedbackTags::Hero_LowHealth) { ++LowHealthCount; }
		}
		TestEqual("Low health played exactly once", LowHealthCount, 1);

		// Heal to 35% (below re-arm threshold of 40%)
		Hero->GetHealthComponent()->Heal(20.f);
		TestTrue("Still latched at 35% below re-arm threshold", Widget->IsLowHealthLatched());

		// Heal to 50% (above re-arm threshold of 40%)
		Hero->GetHealthComponent()->Heal(15.f);
		TestFalse("Latch released above re-arm threshold", Widget->IsLowHealthLatched());

		// Damage back to 20%
		FCombatHit Hit3;
		Hit3.Damage = 30.f;
		UCombatLibrary::DeliverHit(Hero, Hit3);

		TestTrue("Re-latched on second crossing", Widget->IsLowHealthLatched());
		LowHealthCount = 0;
		for (const FGameplayTag& Tag : Listener->PlayedTags)
		{
			if (Tag == FeedbackTags::Hero_LowHealth) { ++LowHealthCount; }
		}
		TestEqual("Low health fired again on second crossing", LowHealthCount, 2);

		// Death clears latch and resets opacity
		FCombatHit DeathHit;
		DeathHit.Damage = 500.f;
		UCombatLibrary::DeliverHit(Hero, DeathHit);
		TestFalse("Latch cleared on death", Widget->IsLowHealthLatched());
		TestEqual("Vignette opacity 0 on death", Widget->GetDamageVignetteOpacity(), 0.f);

		// Rebind/Respawn clears latch
		Widget->BindHero(nullptr);
		TestFalse("Latch cleared on unbind", Widget->IsLowHealthLatched());
		TestEqual("Vignette opacity 0 when unpossessed", Widget->GetDamageVignetteOpacity(), 0.f);
	});
}
#endif
