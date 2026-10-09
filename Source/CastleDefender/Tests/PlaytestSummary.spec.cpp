#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Feedback/PlaytestSummary.h"
#include "Feedback/FeedbackTags.h"
#include "Serialization/JsonReader.h"
#include "Serialization/BufferArchive.h"
#include "Feedback/PlaytestLogSubsystem.h"
#include "Tests/EnemyTestFixture.h"
#include <limits>

namespace
{
	class FFailingPlaytestWriter : public FArchive
	{
	public:
		explicit FFailingPlaytestWriter(int32& InCalls) : Calls(InCalls) {}
		virtual void Serialize(void*, int64) override { ++Calls; SetError(); }
	private:
		int32& Calls;
	};
}

BEGIN_DEFINE_SPEC(FPlaytestSummarySpec, "CastleDefender.Feedback.PlaytestSummary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FPlaytestSummarySpec)

void FPlaytestSummarySpec::Define()
{
	It("keeps the event envelope, emits null for nonfinite numbers and safely escapes generic payloads", [this]()
	{
		FEnemyTestWorld F;
		UPlaytestLogSubsystem* Log = F.World->GetSubsystem<UPlaytestLogSubsystem>();
		Log->bSessionActive = true;
		FBufferArchive* Capture = new FBufferArchive();
		Log->Writer.Reset(Capture);
		const FString Text = TEXT("quote\"line\nslash\\tab\t");
		Log->LogEvent(TEXT("payload"), { {TEXT("x"), 1.f}, {TEXT("t"), -99.f}, {TEXT("nonfinite"), std::numeric_limits<float>::quiet_NaN()} },
			{ {TEXT("s"), Text}, {TEXT("ev"), TEXT("spoof")}, {TEXT("x"), TEXT("wrong type")} });
		const FUTF8ToTCHAR Utf8(reinterpret_cast<const ANSICHAR*>(Capture->GetData()), Capture->Num());
		const FString Line(Utf8.Length(), Utf8.Get());
		const auto Reader = TJsonReaderFactory<>::Create(Line);
		EJsonNotation Notation;
		bool bNonfiniteNull = false;
		TMap<FString, FString> Strings;
		TMap<FString, double> Numbers;
		while (Reader->ReadNext(Notation))
		{
			if (Notation == EJsonNotation::String) { Strings.Add(Reader->GetIdentifier(), Reader->GetValueAsString()); }
			if (Notation == EJsonNotation::Number) { Numbers.Add(Reader->GetIdentifier(), Reader->GetValueAsNumber()); }
			if (Notation == EJsonNotation::Null && Reader->GetIdentifier() == TEXT("nonfinite")) { bNonfiniteNull = true; }
		}
		TestTrue("Valid JSON", Reader->GetErrorMessage().IsEmpty());
		TestEqual("Envelope preserved", Strings.FindRef(TEXT("ev")), FString(TEXT("payload")));
		TestEqual("Clock preserved", Numbers.FindRef(TEXT("t")), F.World->GetTimeSeconds());
		TestEqual("Numeric field wins", Numbers.FindRef(TEXT("x")), 1.0);
		TestFalse("No second x field", Strings.Contains(TEXT("x")));
		TestEqual("Escaped string roundtrip", Strings.FindRef(TEXT("s")), Text);
		TestTrue("Nonfinite is null", bNonfiniteNull);
		Log->bSessionActive = false;
		Log->Writer.Reset(); // The isolated fixture never opens a file or writes a teardown summary.
	});
	It("disables recording after one write failure and leaves later gameplay events harmless", [this]()
	{
		FEnemyTestWorld F;
		UPlaytestLogSubsystem* Log = F.World->GetSubsystem<UPlaytestLogSubsystem>();
		Log->bSessionActive = true;
		int32 Calls = 0;
		Log->Writer.Reset(new FFailingPlaytestWriter(Calls));
		AddExpectedError(TEXT("Playtest telemetry write failed; logging disabled for this session."), EAutomationExpectedErrorFlags::Contains, 1);
		Log->LogEvent(TEXT("first"), {}, {});
		TestTrue("Failure latched", Log->bWriteFailed);
		TestFalse("Writer released", Log->Writer.IsValid());
		TestEqual("One attempted write", Calls, 1);
		Log->LogEvent(TEXT("later"), {}, {});
		TestEqual("No retry or duplicate warning", Calls, 1);
		TestFalse("Recording stays disabled", Log->IsRecording());
	});
	It("aggregates deaths, parries, block breaks and accepted action states without merging unrelated tags", [this]()
	{
		FPlaytestSummary Summary;
		for (int32 Index = 0; Index < 2; ++Index) { Summary.RecordFeedback(FeedbackTags::Hero_Death); }
		for (int32 Index = 0; Index < 3; ++Index) { Summary.RecordFeedback(FeedbackTags::Combat_Parry); }
		Summary.RecordFeedback(FeedbackTags::Combat_BlockBreak);
		Summary.RecordFeedback(FeedbackTags::Enemy_Death);
		Summary.RecordAction(TEXT("Light"));
		Summary.RecordAction(TEXT("Light"));
		Summary.RecordAction(TEXT("Dodge"));
		TestEqual("Hero deaths", Summary.HeroDeaths, 2);
		TestEqual("Parries", Summary.Parries, 3);
		TestEqual("Block breaks", Summary.BlockBreaks, 1);
		TestEqual("Separate enemy count", Summary.Counts.FindRef(FeedbackTags::Enemy_Death.GetTag().ToString()), 1);
		TestEqual("Light entries", Summary.Actions.FindRef(TEXT("Light")), 2);
		TestEqual("Dodge entries", Summary.Actions.FindRef(TEXT("Dodge")), 1);
		const FString Json = Summary.ToJson(120.0);
		const auto Reader = TJsonReaderFactory<>::Create(Json);
		EJsonNotation Notation;
		TMap<FString, double> Numbers;
		FString Result;
		while (Reader->ReadNext(Notation))
		{
			if (Notation == EJsonNotation::Number) { Numbers.Add(Reader->GetIdentifier(), Reader->GetValueAsNumber()); }
			if (Notation == EJsonNotation::String && Reader->GetIdentifier() == TEXT("result")) { Result = Reader->GetValueAsString(); }
		}
		TestTrue("Valid summary JSON", Reader->GetErrorMessage().IsEmpty());
		TestEqual("Session result", Result, FString(TEXT("Session")));
		TestEqual("Duration", Numbers.FindRef(TEXT("duration")), 120.0);
		TestEqual("JSON counts agree", Numbers.FindRef(FeedbackTags::Combat_Parry.GetTag().ToString()), 3.0);
	});
	It("starts each summary fresh and ignores invalid tags and empty action names", [this]()
	{
		FPlaytestSummary Summary;
		Summary.RecordFeedback(FGameplayTag());
		Summary.RecordAction(NAME_None);
		TestTrue("No counts", Summary.Counts.IsEmpty());
		TestTrue("No actions", Summary.Actions.IsEmpty());
		TestEqual("No invented deaths", Summary.HeroDeaths, 0);
		TestTrue("Nonnegative duration", Summary.ToJson(-1.0).Contains(TEXT("\"duration\":0")));
	});
	It("serializes a condensed summary as one valid JSON line with escaped action names", [this]()
	{
		FPlaytestSummary Summary;
		const FName Escaped(TEXT("quote\"line\nslash\\tab\t"));
		Summary.RecordAction(Escaped);
		const FString Line = Summary.ToJson(1.0);
		TestFalse("One physical line", Line.Contains(TEXT("\n")));
		const auto Reader = TJsonReaderFactory<>::Create(Line);
		EJsonNotation Notation;
		bool bFoundEscaped = false;
		while (Reader->ReadNext(Notation))
		{
			if (Notation == EJsonNotation::Number && Reader->GetIdentifier() == Escaped.ToString())
			{
				bFoundEscaped = true;
				TestEqual("Exact escaped key preserved", Reader->GetValueAsNumber(), 1.0);
			}
		}
		TestTrue("JSON parse", Reader->GetErrorMessage().IsEmpty());
		TestTrue("Escaped key present", bFoundEscaped);
	});
}
#endif
