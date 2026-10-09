#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Enemy/EnemyArchetypeDefinition.h"
#include "Enemy/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Misc/DataValidation.h"

BEGIN_DEFINE_SPEC(FEnemyContentSpec, "CastleDefender.Enemy.Content", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FEnemyContentSpec)

void FEnemyContentSpec::Define()
{
	It("loads the playable melee definition with two valid telegraphed attacks and inherited red presentation", [this]()
	{
		UEnemyArchetypeDefinition* Definition = LoadObject<UEnemyArchetypeDefinition>(nullptr,
			TEXT("/Game/CastleDefender/Enemy/DA_Enemy_Melee"));
		if (!TestNotNull("Playable melee data", Definition)) { return; }
		FString Error;
		TestTrue("Runtime validation", Definition->ValidateDefinition(Error));
		FDataValidationContext Context;
		TestEqual("Authored timing validation", Definition->IsDataValid(Context), EDataValidationResult::Valid);
		if (!TestEqual("One Light and one Heavy", Definition->Attacks.Num(), 2)) { return; }
		TestFalse("Light identity", Definition->Attacks[0].bIsHeavy);
		TestTrue("Heavy identity", Definition->Attacks[1].bIsHeavy);
		TestNotEqual("Distinct clips", Definition->Attacks[0].Montage.Get(), Definition->Attacks[1].Montage.Get());
		UClass* Base = LoadClass<AEnemyCharacter>(nullptr, TEXT("/Game/CastleDefender/Enemy/BP_Enemy_Base.BP_Enemy_Base_C"));
		if (!TestNotNull("Existing enemy presentation", Base) || !Definition->EnemyClass) { return; }
		TestTrue("Reuses base presentation", Definition->EnemyClass->IsChildOf(Base));
		const AEnemyCharacter* Default = Definition->EnemyClass.GetDefaultObject();
		for (int32 Index = 0; Index < Default->GetMesh()->GetNumMaterials(); ++Index)
		{
			UMaterialInterface* Material = Default->GetMesh()->GetMaterial(Index);
			if (!TestNotNull("Authored placeholder material", Material)) { continue; }
			FLinearColor Tint;
			TestTrue("Tint parameter", Material->GetVectorParameterValue(FMaterialParameterInfo(TEXT("Paint Tint")), Tint));
			TestTrue("Enemy red family", Tint.R > Tint.G * 2.f && Tint.R > Tint.B * 2.f);
		}
	});
}
#endif
