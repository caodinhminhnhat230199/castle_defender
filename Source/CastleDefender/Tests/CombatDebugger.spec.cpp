#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "Tests/HeroCombatFixture.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/MeleeTraceComponent.h"

BEGIN_DEFINE_SPEC(FCombatDebuggerSpec, "CastleDefender.Combat.Debugger", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FCombatDebuggerSpec)

void FCombatDebuggerSpec::Define()
{
    It("reads actual montage position without changing windows, buffer or stamina", [this]()
    {
        FHeroCombatFixture F;
        auto* Combat = F.Hero->GetCombatComponent();
        Combat->RequestAction(EHeroAction::Dodge);
        Combat->RequestAction(EHeroAction::Dodge);
        Combat->TickComponent(0.05f, LEVELTICK_All, nullptr);
        const UAnimMontage* Montage = F.Hero->GetHeroClassDefinition()->Dodge.BackwardMontage;
        F.Hero->GetMesh()->GetAnimInstance()->Montage_SetPosition(Montage, 0.2f);
        Combat->OpenInvulnerableWindow();
        const FString Active = Combat->GetCombatDebugString();
        TestTrue("Position read from montage", Active.Contains(TEXT("0.200")));
        TestTrue("Phase derived from authored interval", Active.Contains(TEXT("(Active)")));
        TestTrue("Window unchanged", Combat->IsInInvulnerableWindow());
        TestTrue("Buffer retained", Combat->HasBufferedInput());
        TestEqual("Buffer age unchanged", Combat->GetBufferedInputAge(), 0.05f);
        Combat->ClearBuffer();
        F.Hero->GetMesh()->GetAnimInstance()->Montage_Stop(0.f);
        F.Hero->GetMesh()->TickAnimation(0.01f, false);
        F.Hero->GetMesh()->GetAnimInstance()->DispatchQueuedAnimEvents();
        TestTrue("No stale phase after interruption", Combat->GetCombatDebugString().Contains(TEXT("(Inactive)")));
        TestFalse("Closed iframe remains closed", Combat->IsInInvulnerableWindow());
        TestFalse("Readout does not enable component clock", Combat->IsComponentTickEnabled());
    });
}
#endif
