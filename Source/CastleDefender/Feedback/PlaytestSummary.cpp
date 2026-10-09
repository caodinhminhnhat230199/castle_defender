#include "Feedback/PlaytestSummary.h"
#if !UE_BUILD_SHIPPING
#include "Feedback/FeedbackTags.h"

void FPlaytestSummary::RecordFeedback(FGameplayTag Tag)
{
	if (!Tag.IsValid()) { return; }
	++Counts.FindOrAdd(Tag.ToString());
	HeroDeaths += Tag == FeedbackTags::Hero_Death ? 1 : 0;
	Parries += Tag == FeedbackTags::Combat_Parry ? 1 : 0;
	BlockBreaks += Tag == FeedbackTags::Combat_BlockBreak ? 1 : 0;
}
void FPlaytestSummary::RecordAction(FName Action) { if (!Action.IsNone()) { ++Actions.FindOrAdd(Action); } }
void FPlaytestSummary::WriteFields(FPlaytestJsonWriter& Json, double Duration) const
{
	Json.WriteValue(TEXT("duration"), FMath::IsFinite(Duration) ? FMath::Max(0.0, Duration) : 0.0);
	Json.WriteValue(TEXT("result"), TEXT("Session"));
	Json.WriteValue(TEXT("hero_deaths"), HeroDeaths);
	Json.WriteValue(TEXT("parries"), Parries);
	Json.WriteValue(TEXT("block_breaks"), BlockBreaks);
	Json.WriteObjectStart(TEXT("counts"));
	for (const auto& Pair : Counts) { Json.WriteValue(Pair.Key, Pair.Value); }
	Json.WriteObjectEnd();
	Json.WriteObjectStart(TEXT("actions"));
	for (const auto& Pair : Actions) { Json.WriteValue(Pair.Key.ToString(), Pair.Value); }
	Json.WriteObjectEnd();
}
FString FPlaytestSummary::ToJson(double Duration) const
{
	FString Line;
	const auto Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Line);
	Writer->WriteObjectStart();
	WriteFields(*Writer, Duration);
	Writer->WriteObjectEnd();
	Writer->Close();
	return Line;
}
#endif
