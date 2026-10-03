#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif
#include "Tests/TestGameDefinition.h"

BEGIN_DEFINE_SPEC(FGameDefinitionSpec, "CastleDefender.Core.GameDefinition", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UTestGameDefinition* Definition = nullptr;
END_DEFINE_SPEC(FGameDefinitionSpec)

void FGameDefinitionSpec::Define()
{
	BeforeEach([this]()
	{
		Definition = NewObject<UTestGameDefinition>(GetTransientPackage(), TEXT("DA_TestDefinition"));
	});

#if WITH_EDITOR // IsDataValid is editor-only
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
