#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Tests/HeroCombatFixture.h"
#include "Tests/CombatTestListener.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"
#include "Combat/CombatLibrary.h"
#include "Combat/HealthComponent.h"
#include "Hero/HeroCombatLibrary.h"

BEGIN_DEFINE_SPEC(FDodgeSpec, "CastleDefender.Combat.Hero.Dodge", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FDodgeSpec)

void FDodgeSpec::Define()
{
    It("selects backward without input and facing-relative cardinal clips while locked", [this]()
    {
        TestEqual("No input", FHeroDodgeData::SelectDirection(FVector::ZeroVector, FVector::ForwardVector, false), EHeroDodgeDirection::Backward);
        TestEqual("Free camera uses forward clip after turning", FHeroDodgeData::SelectDirection(FVector::RightVector, FVector::ForwardVector, false), EHeroDodgeDirection::Forward);
        TestEqual("Locked left", FHeroDodgeData::SelectDirection(-FVector::RightVector, FVector::ForwardVector, true), EHeroDodgeDirection::Left);
        TestEqual("Locked back", FHeroDodgeData::SelectDirection(-FVector::ForwardVector, FVector::ForwardVector, true), EHeroDodgeDirection::Backward);
        TestEqual("Locked right with rotated facing", FHeroDodgeData::SelectDirection(-FVector::ForwardVector, FVector::RightVector, true), EHeroDodgeDirection::Right);
    });

    It("spends cost once and restores prior root-motion scale after montage completion", [this]()
    {
        FHeroCombatFixture Fixture;
        auto* Combat = Fixture.Hero->GetCombatComponent();
        Fixture.Hero->GetStaminaComponent()->InitializeFromConfig(Fixture.Hero->GetHeroClassDefinition()->Stamina);
        Fixture.Hero->SetAnimRootMotionTranslationScale(0.75f);
        Fixture.Hero->GetHeroClassDefinition()->Dodge.RootMotionScale = 1.5f;
        TestTrue("Dodge accepted", Combat->RequestAction(EHeroAction::Dodge));
        TestEqual("Cost spent", Fixture.Hero->GetStaminaComponent()->GetCurrentStamina(), 80.f);
        TestEqual("Scale applied", Fixture.Hero->GetAnimRootMotionTranslationScale(), 1.5f);
        TestEqual("Backward clip", Combat->GetLastDodgeDirection(), EHeroDodgeDirection::Backward);
        TestFalse("Dodge cannot cancel itself", Combat->RequestAction(EHeroAction::Dodge));
        TestEqual("Repeated press does not spend again", Fixture.Hero->GetStaminaComponent()->GetCurrentStamina(), 80.f);
        Combat->ClearBuffer();
        Fixture.Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
        Fixture.Hero->GetMesh()->TickAnimation(0.01f, false);
        Fixture.Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
        TestEqual("Completion returns Idle", Combat->GetActionState(), EHeroActionState::Idle);
        TestEqual("Previous scale restored", Fixture.Hero->GetAnimRootMotionTranslationScale(), 0.75f);
    });

    It("emits an evade resolution without health damage then allows damage after i-frames", [this]()
    {
        FHeroCombatFixture Fixture;
        auto* Combat = Fixture.Hero->GetCombatComponent();
        auto* Health = Fixture.Hero->GetHealthComponent();
        Health->InitializeHealth(200.f, 0.f);
        auto* Listener = NewObject<UCombatTestListener>();
        Combat->OnCombatResolved.AddDynamic(Listener, &UCombatTestListener::HandleCombatResolved);
        Combat->RequestAction(EHeroAction::Dodge);
        Combat->OpenInvulnerableWindow();
        FCombatHit Hit;
        Hit.Damage = 20.f;
        TestEqual("I-frames evade", UCombatLibrary::DeliverHit(Fixture.Hero, Hit), ECombatHitResult::Evaded);
        TestEqual("HP unchanged", Health->GetCurrentHealth(), 200.f);
        TestEqual("One resolution", Listener->CombatResolvedCount, 1);
        Combat->CloseInvulnerableWindow();
        TestEqual("Outside i-frames hit", UCombatLibrary::DeliverHit(Fixture.Hero, Hit), ECombatHitResult::Hit);
        TestEqual("Damage applied", Health->GetCurrentHealth(), 180.f);
        TestEqual("Second resolution", Listener->CombatResolvedCount, 2);
    });

    It("rejects insufficient stamina without buffering or spending", [this]()
    {
        FHeroCombatFixture Fixture;
        auto* Stamina = Fixture.Hero->GetStaminaComponent();
        Stamina->TrySpend(85.f);
        auto* Combat = Fixture.Hero->GetCombatComponent();
        auto* Listener = NewObject<UCombatTestListener>();
        Stamina->OnStaminaSpendFailed.AddDynamic(Listener, &UCombatTestListener::HandleStaminaSpendFailed);
        TestFalse("Cost exceeds stamina", Combat->RequestAction(EHeroAction::Dodge));
        TestEqual("HP-independent rejection", Combat->GetActionState(), EHeroActionState::Idle);
        TestEqual("Stamina retained", Stamina->GetCurrentStamina(), 15.f);
        TestEqual("Failure cue producer", Listener->StaminaSpendFailedCount, 1);
        TestFalse("Not buffered", Combat->HasBufferedInput());
    });

    It("rejects missing or duplicate i-frame windows before spending", [this]()
    {
        FHeroCombatFixture Fixture;
        auto* Def = Fixture.Hero->GetHeroClassDefinition();
        auto* Montage = DuplicateObject<UAnimMontage>(Def->Dodge.BackwardMontage, Def);
        Def->Dodge.BackwardMontage = Montage;
        UHeroCombatLibrary::AddInvulnerableWindowToMontage(Montage, 0.15f, 0.1f);
        FString Error;
        TestFalse("Duplicate i-frames fail validation", Def->ValidateDodge(EHeroDodgeDirection::Backward, Error));
        AddExpectedError(TEXT("refused: Dodge requires"), EAutomationExpectedErrorFlags::Contains, 1);
        TestFalse("Invalid dodge refused", Fixture.Hero->GetCombatComponent()->RequestAction(EHeroAction::Dodge));
        TestEqual("Stamina not spent", Fixture.Hero->GetStaminaComponent()->GetCurrentStamina(), 100.f);
    });

    It("consumes a buffered dodge at recovery but drops expired input on the hero clock", [this]()
    {
        FHeroCombatFixture Fixture;
        auto* Combat = Fixture.Hero->GetCombatComponent();
        Combat->RequestAction(EHeroAction::Light);
        TestFalse("Early dodge buffered", Combat->RequestAction(EHeroAction::Dodge));
        Combat->TickComponent(0.05f, LEVELTICK_All, nullptr);
        Combat->OpenCancelWindow({ EHeroAction::Dodge });
        TestEqual("Buffered dodge starts", Combat->GetActionState(), EHeroActionState::Dodge);
        TestFalse("Clock disabled after consuming", Combat->IsComponentTickEnabled());
        Combat->RequestAction(EHeroAction::Dodge);
        Fixture.Hero->CustomTimeDilation = 0.1f;
        Combat->TickComponent(0.02f, LEVELTICK_All, nullptr);
        TestTrue("Hero-dilated 0.02 seconds retains buffer", Combat->HasBufferedInput());
        TestEqual("No double dilation", Combat->GetBufferedInputAge(), 0.02f);
        Combat->TickComponent(0.21f, LEVELTICK_All, nullptr);
        TestFalse("Expired press dropped", Combat->HasBufferedInput());
        TestFalse("No inactive clock tick", Combat->IsComponentTickEnabled());
    });
}
#endif
