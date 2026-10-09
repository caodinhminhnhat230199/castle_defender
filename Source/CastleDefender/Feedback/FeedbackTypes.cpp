#include "Feedback/FeedbackTypes.h"
#include "Feedback/FeedbackTags.h"
#include "Combat/CombatStateTypes.h"
#include "Camera/CameraShakeBase.h"

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

FFeedbackAuditResult FFeedbackTableAuditor::Audit(const UDataTable* FeedbackTable, const UDataTable* StatePresentationTable)
{
	FFeedbackAuditResult Result;
	if (!FeedbackTable)
	{
		Result.bSuccess = false;
		Result.Errors.Add(TEXT("DT_Feedback table is null."));
		return Result;
	}

	if (FeedbackTable->GetRowStruct() != FFeedbackRow::StaticStruct())
	{
		Result.bSuccess = false;
		Result.Errors.Add(FString::Printf(TEXT("%s does not use FFeedbackRow rows."), *FeedbackTable->GetPathName()));
		return Result;
	}

	TSet<FGameplayTag> TableTags;
	FeedbackTable->ForeachRow<FFeedbackRow>(TEXT("FeedbackTableAuditor"), [&Result, &TableTags](const FName& RowName, const FFeedbackRow& Row)
	{
		Result.AuditedRowCount++;
		if (!Row.Tag.IsValid())
		{
			Result.Errors.Add(FString::Printf(TEXT("Row %s has invalid Feedback tag."), *RowName.ToString()));
			return;
		}

		if (Row.Tag.GetTagName() != RowName)
		{
			Result.Errors.Add(FString::Printf(TEXT("Row %s has tag %s; row name must equal tag."), *RowName.ToString(), *Row.Tag.ToString()));
		}

		TableTags.Add(Row.Tag);

		const bool bHasOutput = (Row.Sound != nullptr)
			|| (Row.SurfaceSounds.Num() > 0)
			|| (Row.Niagara != nullptr)
			|| (Row.CameraShake != nullptr)
			|| (Row.HitStopSeconds > 0.f)
			|| (!Row.ToastText.IsEmpty());

		if (!bHasOutput)
		{
			Result.Errors.Add(FString::Printf(TEXT("Row %s has no outputs (requires sound, surface sound, niagara, camera shake, hit stop, or toast)."), *RowName.ToString()));
		}

		for (const auto& Pair : Row.SurfaceSounds)
		{
			if (!Pair.Value)
			{
				Result.Warnings.Add(FString::Printf(TEXT("Row %s has null surface sound for surface %d."), *RowName.ToString(), (int32)Pair.Key));
			}
		}

		if (Row.CameraShake && !Row.CameraShake->IsChildOf(UCameraShakeBase::StaticClass()))
		{
			Result.Errors.Add(FString::Printf(TEXT("Row %s CameraShake does not inherit from UCameraShakeBase."), *RowName.ToString()));
		}
	});

	// All 16 P0 native leaves (FeedbackTags)
	const TArray<FGameplayTag> NativeLeaves = {
		FeedbackTags::Combat_Hit_Light,
		FeedbackTags::Combat_Hit_Light_Armored,
		FeedbackTags::Combat_Hit_Heavy,
		FeedbackTags::Combat_Hit_Heavy_Armored,
		FeedbackTags::Combat_Block,
		FeedbackTags::Combat_BlockBreak,
		FeedbackTags::Combat_Parry,
		FeedbackTags::Hero_Damaged,
		FeedbackTags::Hero_Death,
		FeedbackTags::Hero_StaminaInsufficient,
		FeedbackTags::Hero_LowHealth,
		FeedbackTags::Enemy_Telegraph,
		FeedbackTags::Enemy_Telegraph_Heavy,
		FeedbackTags::Enemy_Death,
		FeedbackTags::State_Staggered_Applied,
		FeedbackTags::State_Staggered_Removed,
	};

	for (const FGameplayTag& Tag : NativeLeaves)
	{
		Result.AuditedLeafCount++;
		if (!TableTags.Contains(Tag))
		{
			Result.Errors.Add(FString::Printf(TEXT("Required native leaf feedback tag %s has no row in DT_Feedback."), *Tag.ToString()));
		}
	}

	if (StatePresentationTable)
	{
		if (StatePresentationTable->GetRowStruct() != FCombatStatePresentationRow::StaticStruct())
		{
			Result.Errors.Add(FString::Printf(TEXT("%s does not use FCombatStatePresentationRow rows."), *StatePresentationTable->GetPathName()));
		}
		else
		{
			StatePresentationTable->ForeachRow<FCombatStatePresentationRow>(TEXT("FeedbackTableAuditorState"), [&Result, &TableTags](const FName& RowName, const FCombatStatePresentationRow& Row)
			{
				if (!Row.AppliedFeedback.IsValid())
				{
					Result.Errors.Add(FString::Printf(TEXT("State presentation row %s has invalid AppliedFeedback tag."), *RowName.ToString()));
				}
				else if (!TableTags.Contains(Row.AppliedFeedback))
				{
					Result.Errors.Add(FString::Printf(TEXT("State presentation row %s requires AppliedFeedback tag %s, but no row exists in DT_Feedback."), *RowName.ToString(), *Row.AppliedFeedback.ToString()));
				}
				if (Row.RemovedFeedback.IsValid() && !TableTags.Contains(Row.RemovedFeedback))
				{
					Result.Errors.Add(FString::Printf(TEXT("State presentation row %s requires RemovedFeedback tag %s, but no row exists in DT_Feedback."), *RowName.ToString(), *Row.RemovedFeedback.ToString()));
				}
			});
		}
	}

	Result.bSuccess = (Result.Errors.Num() == 0);
	return Result;
}
