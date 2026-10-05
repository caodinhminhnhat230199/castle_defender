#include "Feedback/FeedbackTypes.h"

namespace
{
	// Per-actor cooldowns of destroyed actors expire on their own; prune only when the map grows.
	constexpr int32 CooldownPruneThreshold = 256;
}

bool FFeedbackThrottle::TryPlay(FGameplayTag Tag, const UObject* ActorKey, double Now,
	float CooldownSeconds, bool bPerActor, int32 BurstLimit, float BurstWindow)
{
	const TPair<FGameplayTag, FObjectKey> Key(Tag, bPerActor ? FObjectKey(ActorKey) : FObjectKey());
	if (const double* End = CooldownEnds.Find(Key); End && Now < *End)
	{
		return false;
	}

	if (BurstLimit > 0)
	{
		TArray<double>& Plays = BurstPlays.FindOrAdd(Tag);
		Plays.RemoveAll([Oldest = Now - BurstWindow](double Time) { return Time <= Oldest; });
		if (Plays.Num() >= BurstLimit)
		{
			return false;
		}
		Plays.Add(Now);
	}

	if (CooldownSeconds > 0.f)
	{
		if (CooldownEnds.Num() > CooldownPruneThreshold)
		{
			for (auto It = CooldownEnds.CreateIterator(); It; ++It)
			{
				if (It.Value() <= Now) { It.RemoveCurrent(); }
			}
		}
		CooldownEnds.Add(Key, Now + CooldownSeconds);
	}
	return true;
}

void FFeedbackThrottle::Reset()
{
	CooldownEnds.Reset();
	BurstPlays.Reset();
}

TArray<FString> FFeedbackRowIndex::Build(const UDataTable* Table)
{
	TArray<FString> Warnings;
	Rows.Reset();
	Variants.Reset();
	if (!Table)
	{
		return Warnings;
	}
	if (Table->GetRowStruct() != FFeedbackRow::StaticStruct())
	{
		Warnings.Add(FString::Printf(TEXT("%s does not use FFeedbackRow rows."), *Table->GetPathName()));
		return Warnings;
	}

	Table->ForeachRow<FFeedbackRow>(TEXT("FFeedbackRowIndex"), [this, &Warnings](const FName& Name, const FFeedbackRow& Row)
	{
		if (!Row.Tag.IsValid())
		{
			Warnings.Add(FString::Printf(TEXT("Row %s has no valid Feedback tag; skipped."), *Name.ToString()));
			return;
		}
		if (Row.Tag.GetTagName() != Name)
		{
			Warnings.Add(FString::Printf(TEXT("Row %s has tag %s; the row name must equal the tag."), *Name.ToString(), *Row.Tag.ToString()));
		}
		Rows.Add(Row.Tag, &Row);

		// Every row is also a variant of its parent: Hit.Light.Armored = (Hit.Light, Armored).
		const FGameplayTag Parent = Row.Tag.RequestDirectParent();
		const FString TagString = Row.Tag.ToString();
		int32 LastDot = INDEX_NONE;
		if (Parent.IsValid() && TagString.FindLastChar(TEXT('.'), LastDot))
		{
			Variants.Add({ Parent, FName(*TagString.Mid(LastDot + 1)) }, &Row);
		}
	});
	return Warnings;
}

const FFeedbackRow* FFeedbackRowIndex::Find(FGameplayTag Tag, FName Variant, bool bTargetArmored) const
{
	static const FName Armored(TEXT("Armored"));
	const FName WantedVariant = !Variant.IsNone() ? Variant : (bTargetArmored ? Armored : NAME_None);
	if (!WantedVariant.IsNone())
	{
		if (const FFeedbackRow* VariantRow = Variants.FindRef({ Tag, WantedVariant }))
		{
			return VariantRow;
		}
	}
	return Rows.FindRef(Tag);
}
