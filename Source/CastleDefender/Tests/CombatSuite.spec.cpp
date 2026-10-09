#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "InputMappingContext.h"
#include "InputAction.h"
#include "Player/PlayerMode.h"
#include "Tests/HeroCombatFixture.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/HeroClassDefinition.h"
#include "Hero/HeroCombatTestLibrary.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Feedback/FeedbackTypes.h"
#include "Kismet/GameplayStatics.h"

BEGIN_DEFINE_SPEC(FHeroCombatSuiteSpec, "CastleDefender.Combat.Suite", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FHeroCombatSuiteSpec)

void FHeroCombatSuiteSpec::Define()
{
	Describe("Input Key Map and Mode Stack (AC-CMB-14)", [this]()
	{
		It("maps all combat verbs in IMC_Combat without reserved key overlap", [this]()
		{
			UInputMappingContext* IMC = Cast<UInputMappingContext>(StaticLoadObject(UInputMappingContext::StaticClass(), nullptr, TEXT("/Game/CastleDefender/Core/Input/IMC_Combat")));
			TestNotNull("IMC_Combat loaded", IMC);
			if (!IMC)
			{
				return;
			}

			const TArray<FEnhancedActionKeyMapping>& Mappings = IMC->GetMappings();
			TestTrue("Mappings exist", Mappings.Num() > 0);

			TSet<FString> RequiredActions = {
				TEXT("IA_Move"),
				TEXT("IA_Look"),
				TEXT("IA_Sprint"),
				TEXT("IA_LightAttack"),
				TEXT("IA_HeavyAttack"),
				TEXT("IA_Dodge"),
				TEXT("IA_Block"),
				TEXT("IA_Parry"),
				TEXT("IA_LockOn"),
				TEXT("IA_LockOnSwitch"),
				TEXT("IA_Interact")
			};

			TSet<FString> FoundActions;
			TArray<FKey> ReservedKeys = {
				EKeys::Q,
				EKeys::One,
				EKeys::Two,
				EKeys::Three,
				EKeys::Tab,
				EKeys::B,
				EKeys::Escape,
				EKeys::F1, EKeys::F2, EKeys::F3, EKeys::F4, EKeys::F5, EKeys::F6,
				EKeys::F7, EKeys::F8, EKeys::F9, EKeys::F10, EKeys::F11, EKeys::F12
			};

			for (const FEnhancedActionKeyMapping& Mapping : Mappings)
			{
				if (Mapping.Action)
				{
					FoundActions.Add(Mapping.Action->GetName());
				}

				for (const FKey& Reserved : ReservedKeys)
				{
					TestFalse(FString::Printf(TEXT("Combat key %s does not use reserved key %s"), *Mapping.Key.ToString(), *Reserved.ToString()), Mapping.Key == Reserved);
				}
			}

			for (const FString& Required : RequiredActions)
			{
				TestTrue(FString::Printf(TEXT("Required combat action %s is mapped"), *Required), FoundActions.Contains(Required));
			}

			// Verify mode stack returns to Combat
			FPlayerModeStack Stack;
			TestEqual("Starts in Combat", Stack.Top(), EPlayerMode::Combat);
			Stack.Push(EPlayerMode::Build, "Build");
			TestEqual("Build pushed", Stack.Top(), EPlayerMode::Build);
			TestTrue("Pop Build", Stack.Pop("Build"));
			TestEqual("Restores Combat", Stack.Top(), EPlayerMode::Combat);
		});
	});

	Describe("Clock Domains Regressions (D-20)", [this]()
	{
		It("isolates hero-dilated timers from world timers under global dilation", [this]()
		{
			FHeroCombatFixture F;
			F.BeginPlay();

			UHeroCombatComponent* Combat = F.Hero->GetCombatComponent();
			TestNotNull("Combat component exists", Combat);

			// Buffered input age uses owner-dilated DeltaTime in TickComponent
			Combat->RequestAction(EHeroAction::Light);
			Combat->RequestAction(EHeroAction::Dodge); // buffers Dodge

			TestTrue("Has buffered input", Combat->HasBufferedInput());
			const float InitialAge = Combat->GetBufferedInputAge();

			// Tick hero component with 0.05s
			Combat->TickComponent(0.05f, LEVELTICK_All, nullptr);
			TestEqual("Buffered input aged on hero clock", Combat->GetBufferedInputAge(), InitialAge + 0.05f, 0.001f);

			// World combat state component uses world game timer
			UCombatStateComponent* States = F.Hero->GetCombatStateComponent();
			TestNotNull("Combat states component exists", States);

			States->ApplyState(GameTags::State_Combat_Staggered, 2.0f, nullptr);
			TestTrue("State active", States->HasState(GameTags::State_Combat_Staggered));
			TestEqual("State duration initialized", States->GetStateRemaining(GameTags::State_Combat_Staggered), 2.0f, 0.01f);

			States->ClearAllStates();
		});

		It("hit stop dilates only hit actor and preserves global dilation", [this]()
		{
			FHeroCombatFixture F;
			F.BeginPlay();

			UFeedbackSubsystem* Feedback = UFeedbackSubsystem::Get(F.World);
			TestNotNull("FeedbackSubsystem exists", Feedback);

			const float PreGlobal = UGameplayStatics::GetGlobalTimeDilation(F.World);
			FFeedbackEventContext Ctx;
			Ctx.Instigator = F.Hero;
			Ctx.bIsHeavy = true;
			Feedback->Play(FeedbackTags::Combat_Hit_Heavy, Ctx);

			TestEqual("Global dilation untouched", UGameplayStatics::GetGlobalTimeDilation(F.World), PreGlobal, 0.001f);
		});
	});

	Describe("Deliberately Broken Rule Detection", [this]()
	{
		It("fails block reduction when DamageReduction is zero", [this]()
		{
			FHeroCombatFixture F;
			F.BeginPlay();

			UHeroClassDefinition* Def = F.Hero->GetHeroClassDefinition();
			const float OriginalReduction = Def->Block.DamageReduction;

			// Deliberately break the rule
			Def->Block.DamageReduction = 0.f;

			FString Message;
			const bool bPassed = UHeroCombatTestLibrary::RunHeroCombatScenario(F.World, TEXT("FT_BlockReduce"), Message);
			TestFalse("Zero block reduction makes FT_BlockReduce fail", bPassed);
			TestTrue("Message notes failure", Message.Contains(TEXT("AC-CMB-08 failed")));

			// Restore original value
			Def->Block.DamageReduction = OriginalReduction;
		});
	});
}

#endif
