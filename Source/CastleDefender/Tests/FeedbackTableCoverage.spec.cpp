#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Feedback/FeedbackTypes.h"
#include "Feedback/FeedbackTags.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Combat/CombatStateTypes.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"

BEGIN_DEFINE_SPEC(FFeedbackTableCoverageSpec, "CastleDefender.Feedback.TableCoverage", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FFeedbackTableCoverageSpec)

void FFeedbackTableCoverageSpec::Define()
{
	Describe("Authored Content Coverage", [this]()
	{
		It("authored DT_Feedback covers all 16 native Feedback leaves with valid outputs", [this]()
		{
			const UGameTuningSettings* Settings = UGameTuningSettings::Get();
			TestNotNull("GameTuningSettings", Settings);
			const UDataTable* FeedbackTable = Settings ? Settings->FeedbackTable.LoadSynchronous() : nullptr;
			TestNotNull("DT_Feedback loaded", FeedbackTable);

			const UDataTable* StateTable = Settings ? Settings->CombatStatePresentationTable.LoadSynchronous() : nullptr;

			const FFeedbackAuditResult Result = FFeedbackTableAuditor::Audit(FeedbackTable, StateTable);
			for (const FString& Error : Result.Errors)
			{
				AddError(Error);
			}
			TestTrue("Audit passed with zero errors", Result.bSuccess);
			TestTrue("Audited at least 16 rows", Result.AuditedRowCount >= 16);
			TestTrue("Audited all 16 native leaves", Result.AuditedLeafCount >= 16);
		});

		It("referenced assets in authored DT_Feedback are valid", [this]()
		{
			const UGameTuningSettings* Settings = UGameTuningSettings::Get();
			const UDataTable* FeedbackTable = Settings ? Settings->FeedbackTable.LoadSynchronous() : nullptr;
			if (!TestNotNull("DT_Feedback loaded", FeedbackTable)) { return; }

			FeedbackTable->ForeachRow<FFeedbackRow>(TEXT("AssetCheck"), [this](const FName& RowName, const FFeedbackRow& Row)
			{
				if (Row.Sound)
				{
					TestTrue(FString::Printf(TEXT("Row %s Sound is valid"), *RowName.ToString()), IsValid(Row.Sound.Get()));
				}
				for (const auto& Pair : Row.SurfaceSounds)
				{
					TestTrue(FString::Printf(TEXT("Row %s SurfaceSound %d is valid"), *RowName.ToString(), (int32)Pair.Key), Pair.Value && IsValid(Pair.Value.Get()));
				}
				if (Row.CameraShake)
				{
					TestTrue(FString::Printf(TEXT("Row %s CameraShake inherits from UCameraShakeBase"), *RowName.ToString()), Row.CameraShake->IsChildOf(UCameraShakeBase::StaticClass()));
				}
			});
		});

		It("authored DT_CombatStatePresentation AppliedFeedback tags all map to valid DT_Feedback rows", [this]()
		{
			const UGameTuningSettings* Settings = UGameTuningSettings::Get();
			const UDataTable* FeedbackTable = Settings ? Settings->FeedbackTable.LoadSynchronous() : nullptr;
			const UDataTable* StateTable = Settings ? Settings->CombatStatePresentationTable.LoadSynchronous() : nullptr;
			if (!TestNotNull("DT_Feedback", FeedbackTable) || !TestNotNull("DT_CombatStatePresentation", StateTable)) { return; }

			FFeedbackRowIndex Index;
			Index.Build(FeedbackTable);

			StateTable->ForeachRow<FCombatStatePresentationRow>(TEXT("StateCheck"), [this, &Index](const FName& RowName, const FCombatStatePresentationRow& Row)
			{
				if (Row.AppliedFeedback.IsValid())
				{
					const FFeedbackRow* Found = Index.Find(Row.AppliedFeedback, NAME_None, false);
					TestNotNull(FString::Printf(TEXT("State row %s AppliedFeedback %s exists in DT_Feedback"), *RowName.ToString(), *Row.AppliedFeedback.ToString()), Found);
				}
				if (Row.RemovedFeedback.IsValid())
				{
					const FFeedbackRow* Found = Index.Find(Row.RemovedFeedback, NAME_None, false);
					TestNotNull(FString::Printf(TEXT("State row %s RemovedFeedback %s exists in DT_Feedback"), *RowName.ToString(), *Row.RemovedFeedback.ToString()), Found);
				}
			});
		});
	});

	Describe("Mutation & Detection Validation", [this]()
	{
		It("fails when a required P0 native leaf is removed", [this]()
		{
			const UGameTuningSettings* Settings = UGameTuningSettings::Get();
			const UDataTable* AuthoredTable = Settings ? Settings->FeedbackTable.LoadSynchronous() : nullptr;
			if (!TestNotNull("AuthoredTable", AuthoredTable)) { return; }

			// Clone table omitting Combat_Parry
			UDataTable* MutatedTable = NewObject<UDataTable>(GetTransientPackage());
			MutatedTable->RowStruct = FFeedbackRow::StaticStruct();

			AuthoredTable->ForeachRow<FFeedbackRow>(TEXT("CopyRows"), [MutatedTable](const FName& RowName, const FFeedbackRow& Row)
			{
				if (Row.Tag != FeedbackTags::Combat_Parry)
				{
					MutatedTable->AddRow(RowName, Row);
				}
			});

			const FFeedbackAuditResult Result = FFeedbackTableAuditor::Audit(MutatedTable);
			TestFalse("Audit fails when Combat_Parry is missing", Result.bSuccess);
			const bool bMentionsMissingTag = Result.Errors.ContainsByPredicate([](const FString& S)
			{
				return S.Contains(TEXT("Feedback.Combat.Parry"));
			});
			TestTrue("Error specifically names missing tag", bMentionsMissingTag);
		});

		It("fails when a row has no outputs", [this]()
		{
			UDataTable* MutatedTable = NewObject<UDataTable>(GetTransientPackage());
			MutatedTable->RowStruct = FFeedbackRow::StaticStruct();

			FFeedbackRow EmptyRow;
			EmptyRow.Tag = FeedbackTags::Combat_Parry;
			MutatedTable->AddRow(EmptyRow.Tag.GetTagName(), EmptyRow);

			const FFeedbackAuditResult Result = FFeedbackTableAuditor::Audit(MutatedTable);
			TestFalse("Audit fails for empty row", Result.bSuccess);
			const bool bMentionsNoOutputs = Result.Errors.ContainsByPredicate([](const FString& S)
			{
				return S.Contains(TEXT("no outputs"));
			});
			TestTrue("Error mentions no outputs", bMentionsNoOutputs);
		});

		It("fails when a row has invalid tag or row name mismatch", [this]()
		{
			UDataTable* MutatedTable = NewObject<UDataTable>(GetTransientPackage());
			MutatedTable->RowStruct = FFeedbackRow::StaticStruct();

			FFeedbackRow BadTagRow;
			// Invalid tag (empty)
			MutatedTable->AddRow(TEXT("BadTag"), BadTagRow);

			FFeedbackRow MismatchedRow;
			MismatchedRow.Tag = FeedbackTags::Combat_Parry;
			// Row name differs from tag
			MutatedTable->AddRow(TEXT("CustomName"), MismatchedRow);

			const FFeedbackAuditResult Result = FFeedbackTableAuditor::Audit(MutatedTable);
			TestFalse("Audit fails for invalid tag / mismatch", Result.bSuccess);
			const bool bHasInvalidTagErr = Result.Errors.ContainsByPredicate([](const FString& S)
			{
				return S.Contains(TEXT("invalid Feedback tag"));
			});
			const bool bHasMismatchErr = Result.Errors.ContainsByPredicate([](const FString& S)
			{
				return S.Contains(TEXT("row name must equal tag"));
			});
			TestTrue("Reports invalid tag", bHasInvalidTagErr);
			TestTrue("Reports row name mismatch", bHasMismatchErr);
		});

		It("fails when DT_CombatStatePresentation references a missing feedback tag", [this]()
		{
			const UGameTuningSettings* Settings = UGameTuningSettings::Get();
			const UDataTable* AuthoredFeedback = Settings ? Settings->FeedbackTable.LoadSynchronous() : nullptr;
			if (!TestNotNull("AuthoredFeedback", AuthoredFeedback)) { return; }

			UDataTable* StateTable = NewObject<UDataTable>(GetTransientPackage());
			StateTable->RowStruct = FCombatStatePresentationRow::StaticStruct();

			FCombatStatePresentationRow BadRow;
			BadRow.StateTag = GameTags::State_Combat_Staggered;
			BadRow.AppliedFeedback = GameTags::Zone_RallyPoint;
			StateTable->AddRow(BadRow.StateTag.GetTagName(), BadRow);

			const FFeedbackAuditResult Result = FFeedbackTableAuditor::Audit(AuthoredFeedback, StateTable);
			TestFalse("Audit fails when AppliedFeedback does not exist in DT_Feedback", Result.bSuccess);
			const bool bMentionsMissingApplied = Result.Errors.ContainsByPredicate([](const FString& S)
			{
				return S.Contains(TEXT("Zone.RallyPoint"));
			});
			TestTrue("Error specifically names missing AppliedFeedback tag", bMentionsMissingApplied);
		});
	});
}

#endif
