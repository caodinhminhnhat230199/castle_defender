#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Feedback/FeedbackTags.h"
#include "Tests/FeedbackTestListener.h"

BEGIN_DEFINE_SPEC(FFeedbackVariantSpec, "CastleDefender.Feedback.Variant", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	UDataTable* Table = nullptr;
	FFeedbackRowIndex Index;
END_DEFINE_SPEC(FFeedbackVariantSpec)

void FFeedbackVariantSpec::Define()
{
	BeforeEach([this]()
	{
		Table = UFeedbackTestListener::MakeTable({ FeedbackTags::Combat_Hit_Light, FeedbackTags::Combat_Hit_Light_Armored,
			FeedbackTags::Combat_Hit_Heavy });
		Index.Build(Table);
	});

	It("picks the Armored row for an armored target", [this]()
	{
		const FFeedbackRow* Row = Index.Find(FeedbackTags::Combat_Hit_Light, NAME_None, true);
		TestTrue("Armored row", Row && Row->Tag == FeedbackTags::Combat_Hit_Light_Armored);
	});

	It("falls back to the base row when the variant row is missing", [this]()
	{
		const FFeedbackRow* Row = Index.Find(FeedbackTags::Combat_Hit_Heavy, NAME_None, true);
		TestTrue("Base Heavy row", Row && Row->Tag == FeedbackTags::Combat_Hit_Heavy);
	});

	It("uses the base row for an unarmored target and an explicit Variant over the armored flag", [this]()
	{
		const FFeedbackRow* Base = Index.Find(FeedbackTags::Combat_Hit_Light, NAME_None, false);
		TestTrue("Base row", Base && Base->Tag == FeedbackTags::Combat_Hit_Light);
		const FFeedbackRow* Named = Index.Find(FeedbackTags::Combat_Hit_Light, TEXT("Armored"), false);
		TestTrue("Named variant", Named && Named->Tag == FeedbackTags::Combat_Hit_Light_Armored);
		const FFeedbackRow* Unknown = Index.Find(FeedbackTags::Combat_Hit_Light, TEXT("Infantry"), true);
		TestTrue("Unknown named variant falls back to base", Unknown && Unknown->Tag == FeedbackTags::Combat_Hit_Light);
	});

	It("returns null for a tag without a row", [this]()
	{
		TestNull("No Parry row", Index.Find(FeedbackTags::Combat_Parry, NAME_None, false));
	});

	It("reports a row whose name differs from its tag", [this]()
	{
		FFeedbackRow Row;
		Row.Tag = FeedbackTags::Combat_Block;
		Table->AddRow(TEXT("Block"), Row);
		const TArray<FString> Warnings = Index.Build(Table);
		TestEqual("One warning", Warnings.Num(), 1);
		TestTrue("Row still indexed", Index.Find(FeedbackTags::Combat_Block, NAME_None, false) != nullptr);
	});
}

#endif
