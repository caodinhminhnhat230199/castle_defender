#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "Hero/HeroClassDefinition.h"
#include "Engine/AssetManager.h"

BEGIN_DEFINE_SPEC(FHeroClassDefinitionSpec, "CastleDefender.Combat.HeroClassDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UHeroClassDefinition* Definition = nullptr;
END_DEFINE_SPEC(FHeroClassDefinitionSpec)

void FHeroClassDefinitionSpec::Define()
{
	BeforeEach([this]()
	{
		Definition = NewObject<UHeroClassDefinition>(GetTransientPackage(), TEXT("DA_TestHeroClassDefinition"));
		if (const UHeroClassDefinition* Authored = LoadObject<UHeroClassDefinition>(nullptr,
			TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")))
		{
			Definition->LightChain = Authored->LightChain;
			Definition->Heavy = Authored->Heavy;
			Definition->Dodge = Authored->Dodge;
			Definition->HitReact = Authored->HitReact;
			Definition->Block = Authored->Block;
		}
	});

	It("uses HeroClassDefinition as Primary Asset Type", [this]()
	{
		const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
		TestEqual("Type", Id.PrimaryAssetType.GetName(), FName(TEXT("HeroClassDefinition")));
		TestEqual("Name", Id.PrimaryAssetName, FName(TEXT("DA_TestHeroClassDefinition")));
	});

#if WITH_EDITOR
	It("passes validation with valid defaults", [this]()
	{
		FDataValidationContext Context;
		TestTrue("Valid defaults", Definition->IsDataValid(Context) == EDataValidationResult::Valid);
		TestEqual("No errors", static_cast<int32>(Context.GetNumErrors()), 0);
	});

	It("fails validation when MaxHealth is zero or negative", [this]()
	{
		Definition->MaxHealth = 0.f;
		FDataValidationContext Context;
		TestTrue("Invalid", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		TestTrue("Has errors", Context.GetNumErrors() > 0);
	});

	It("fails validation when the block arc is zero or a block montage is missing", [this]()
	{
		Definition->Block.ArcDegrees = 0.f;
		FDataValidationContext ArcContext;
		TestTrue("Zero arc invalid", Definition->IsDataValid(ArcContext) == EDataValidationResult::Invalid);
		Definition->Block.ArcDegrees = 140.f;
		Definition->Block.BlockBreakMontage = nullptr;
		FDataValidationContext MontageContext;
		TestTrue("Missing break montage invalid", Definition->IsDataValid(MontageContext) == EDataValidationResult::Invalid);
	});

	It("fails validation when SprintSpeed is less than or equal to JogSpeed", [this]()
	{
		Definition->Movement.SprintSpeed = Definition->Movement.JogSpeed;
		FDataValidationContext Context;
		TestTrue("Invalid", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		TestTrue("Has errors", Context.GetNumErrors() > 0);
	});

	It("fails validation when JogSpeed is zero or negative", [this]()
	{
		Definition->Movement.JogSpeed = 0.f;
		FDataValidationContext Context;
		TestTrue("Invalid", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		TestTrue("Has errors", Context.GetNumErrors() > 0);
	});

	It("fails validation when DisplayName is empty", [this]()
	{
		Definition->DisplayName = FText::GetEmpty();
		FDataValidationContext Context;
		TestTrue("Invalid", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		TestTrue("Has errors", Context.GetNumErrors() > 0);
	});

	It("discovers and validates DA_HeroClass_Warlord through AssetManager", [this]()
	{
		UAssetManager& Manager = UAssetManager::Get();
		const FPrimaryAssetId Expected(TEXT("HeroClassDefinition"), TEXT("DA_HeroClass_Warlord"));
		TArray<FPrimaryAssetId> Ids;
		Manager.GetPrimaryAssetIdList(Expected.PrimaryAssetType, Ids);
		TestTrue("AssetManager discovered DA_HeroClass_Warlord", Ids.Contains(Expected));

		const FSoftObjectPath Path = Manager.GetPrimaryAssetPath(Expected);
		UHeroClassDefinition* Asset = Cast<UHeroClassDefinition>(Path.TryLoad());
		if (TestNotNull("DA_HeroClass_Warlord loads", Asset))
		{
			FDataValidationContext Context;
			TestTrue("DA_HeroClass_Warlord passes data validation", Asset->IsDataValid(Context) == EDataValidationResult::Valid);
			TestTrue("MaxHealth is positive", Asset->MaxHealth > 0.f);
			TestTrue("SprintSpeed exceeds JogSpeed", Asset->Movement.SprintSpeed > Asset->Movement.JogSpeed);
		}
	});
#endif

	AfterEach([this]()
	{
		Definition = nullptr;
	});
}

#endif
