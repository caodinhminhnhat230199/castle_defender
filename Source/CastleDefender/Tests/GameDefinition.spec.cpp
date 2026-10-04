#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "Core/GameTuningSettings.h"
#include "Tests/TestGameDefinition.h"
#include "Engine/AssetManager.h"

BEGIN_DEFINE_SPEC(FGameDefinitionSpec, "CastleDefender.Core.GameDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UTestGameDefinition* Definition = nullptr;
END_DEFINE_SPEC(FGameDefinitionSpec)

void FGameDefinitionSpec::Define()
{
	BeforeEach([this]()
	{
		Definition = NewObject<UTestGameDefinition>(GetTransientPackage(), TEXT("DA_TestDefinition"));
	});

	It("registers Game Tuning under the Project Settings Game category", [this]()
	{
		const UGameTuningSettings* Settings = GetDefault<UGameTuningSettings>();
		if (TestNotNull("Settings default object", Settings))
		{
			TestEqual("Category", Settings->GetCategoryName(), FName(TEXT("Game")));
			TestEqual("Section label", Settings->GetSectionText().ToString(), FString(TEXT("Game Tuning")));
		}
	});

#if WITH_EDITOR // IsDataValid is editor-only
	It("discovers and validates the registered Foundation asset", [this]()
	{
		UAssetManager& Manager = UAssetManager::Get();
		const FPrimaryAssetId Expected(TEXT("TestGameDefinition"), TEXT("DA_FoundationSmoke"));
		TArray<FPrimaryAssetId> Ids;
		Manager.GetPrimaryAssetIdList(Expected.PrimaryAssetType, Ids);
		TestTrue("Registered type discovers the example", Ids.Contains(Expected));
		const FSoftObjectPath Path = Manager.GetPrimaryAssetPath(Expected);
		TestEqual("Scanned asset path", Path.GetAssetPathString(), FString(TEXT("/Game/CastleDefender/Maps/Test/Definitions/DA_FoundationSmoke.DA_FoundationSmoke")));
		UTestGameDefinition* Asset = Cast<UTestGameDefinition>(Path.TryLoad());
		if (TestNotNull("Discovered asset loads", Asset))
		{
			FDataValidationContext Context;
			TestTrue("Content is valid", Asset->IsDataValid(Context) == EDataValidationResult::Valid);
		}
	});

	It("fails validation without a DisplayName", [this]()
	{
		FDataValidationContext Context;
		TestTrue("Invalid", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		TestEqual("Errors", static_cast<int32>(Context.GetNumErrors()), 1);
	});

	It("passes validation with a DisplayName", [this]()
	{
		Definition->DisplayName = NSLOCTEXT("Test", "Name", "Test");
		FDataValidationContext Context;
		TestTrue("Valid", Definition->IsDataValid(Context) == EDataValidationResult::Valid);
	});
#endif

	It("uses the native class name without U as Primary Asset Type", [this]()
	{
		const FPrimaryAssetId Id = Definition->GetPrimaryAssetId();
		TestEqual("Type", Id.PrimaryAssetType.GetName(), FName(TEXT("TestGameDefinition")));
		TestEqual("Name", Id.PrimaryAssetName, FName(TEXT("DA_TestDefinition")));
	});

	AfterEach([this]()
	{
		Definition = nullptr;
	});
}

#endif
