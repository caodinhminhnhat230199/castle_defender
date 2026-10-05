#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/CombatStateComponent.h"
#include "Combat/CombatStateModel.h"
#include "Combat/HealthComponent.h"
#include "Combat/TestDummy.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Engine/World.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Tests/CombatStateTestListener.h"
#include "Tests/FeedbackTestListener.h"
#include "Containers/Ticker.h"
#include "TimerManager.h"

BEGIN_DEFINE_SPEC(FCombatStateSpec, "CastleDefender.Combat.States", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FCombatStateModel Model;
	UWorld* World = nullptr;
	ATestDummy* Dummy = nullptr;
	UCombatStateComponent* States = nullptr;
	UCombatStateTestListener* Listener = nullptr;
	UFeedbackTestListener* FeedbackListener = nullptr;
	TSoftObjectPtr<UDataTable> SavedPresentationTable;
END_DEFINE_SPEC(FCombatStateSpec)

void FCombatStateSpec::Define()
{
	const FGameplayTag Staggered = GameTags::State_Combat_Staggered;
	const FGameplayTag Marked = GameTags::State_Combat_Marked;

	Describe("FCombatStateModel (AC-SYN-01..04)", [this, Staggered, Marked]()
	{
		BeforeEach([this]()
		{
			FCombatStateConfig Config;
			Config.MaxPoise = 50.f;
			Config.PoiseRegenDelay = 2.f;
			Config.PoiseRegenRate = 25.f;
			Config.StaggerDuration = 1.5f;
			Model.Init(Config);
		});

		It("regenerates after the delay, breaks into Staggered, ignores poise while Staggered and refills at the end", [this, Staggered]()
		{
			TestFalse("40 at t=0 does not break", Model.ApplyPoiseDamage(40.f, 0.0));
			TestEqual("t=0 poise", Model.GetPoise(0.0), 10.f);
			TestEqual("t=1 still inside the delay", Model.GetPoise(1.0), 10.f);
			TestEqual("t=2.4 regen 0.4 s x 25", Model.GetPoise(2.4), 20.f, 0.01f);

			TestTrue("20 at t=2.4 breaks", Model.ApplyPoiseDamage(20.f, 2.4));
			Model.ApplyState(Staggered, Model.GetConfig().StaggerDuration, nullptr, 2.4);
			TestEqual("Staggered until 3.9", Model.FindState(Staggered) ? Model.FindState(Staggered)->ExpiryTime : 0.0, 3.9, 1e-6);

			TestFalse("30 at t=3.0 ignored", Model.ApplyPoiseDamage(30.f, 3.0));
			TestEqual("Poise reads 0 while Staggered", Model.GetPoise(3.0), 0.f);

			TestEqual("Nothing expires at 3.8", Model.RemoveExpired(3.8).Num(), 0);
			TestEqual("Staggered expires at 3.9", Model.RemoveExpired(3.9).Num(), 1);
			TestFalse("Staggered removed", Model.HasState(Staggered));
			TestEqual("Poise back at max", Model.GetPoise(3.9), 50.f);
		});

		It("refreshes to the longer expiry without stacking", [this, Marked]()
		{
			TestTrue("Added", Model.ApplyState(Marked, 8.f, nullptr, 0.0) == FCombatStateModel::EApplyResult::Added);
			TestTrue("Shorter refresh", Model.ApplyState(Marked, 3.f, nullptr, 1.0) == FCombatStateModel::EApplyResult::Refreshed);
			TestEqual("Expiry stays 8", Model.FindState(Marked)->ExpiryTime, 8.0, 1e-6);
			Model.ApplyState(Marked, 10.f, nullptr, 1.0);
			TestEqual("Longer refresh to 11", Model.FindState(Marked)->ExpiryTime, 11.0, 1e-6);
			TestEqual("One entry", Model.GetStates().Num(), 1);
		});

		It("removes every expired state when the timer fires late and reports the next expiry", [this, Staggered, Marked]()
		{
			Model.ApplyState(Staggered, 1.f, nullptr, 0.0);
			Model.ApplyState(Marked, 2.f, nullptr, 0.0);
			TestEqual("Next expiry", Model.NextExpiry().Get(0.0), 1.0, 1e-6);
			TestEqual("Both expire at 5", Model.RemoveExpired(5.0).Num(), 2);
			TestFalse("No next expiry", Model.NextExpiry().IsSet());
		});

		It("never staggers from poise with MaxPoise 0, but accepts a direct Staggered", [this, Staggered]()
		{
			Model.Init(FCombatStateConfig());
			TestFalse("No break", Model.ApplyPoiseDamage(1000.f, 0.0));
			TestEqual("No poise", Model.GetPoise(0.0), 0.f);
			Model.ApplyState(Staggered, 1.f, nullptr, 0.0);
			TestTrue("Direct Staggered", Model.HasState(Staggered));
		});
	});

	It("has an authored Staggered presentation row and a Staggered default duration in Game Tuning", [this, Staggered]()
	{
		const UGameTuningSettings* Settings = UGameTuningSettings::Get();
		const UDataTable* Table = Settings->CombatStatePresentationTable.LoadSynchronous();
		const FCombatStatePresentationRow* Row = Table
			? Table->FindRow<FCombatStatePresentationRow>(Staggered.GetTagName(), TEXT("Spec"), false) : nullptr;
		TestTrue("Staggered row plays Staggered.Applied", Row && Row->AppliedFeedback == FeedbackTags::State_Staggered_Applied);
		TestTrue("Staggered row plays Staggered.Removed", Row && Row->RemovedFeedback == FeedbackTags::State_Staggered_Removed);
		const float* Default = Settings->StateDefaultDurations.Find(Staggered);
		TestTrue("Staggered fallback 1.5 s", Default && FMath::IsNearlyEqual(*Default, 1.5f));
	});

	Describe("UCombatStateComponent in a game world", [this, Staggered, Marked]()
	{
		BeforeEach([this]()
		{
			// Transient presentation table: the authored one has only Staggered, and a missing Marked row would warn.
			UGameTuningSettings* Settings = GetMutableDefault<UGameTuningSettings>();
			SavedPresentationTable = Settings->CombatStatePresentationTable;
			UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
			Table->RowStruct = FCombatStatePresentationRow::StaticStruct();
			FCombatStatePresentationRow Row;
			Row.StateTag = GameTags::State_Combat_Staggered;
			Row.AppliedFeedback = FeedbackTags::State_Staggered_Applied;
			Row.RemovedFeedback = FeedbackTags::State_Staggered_Removed;
			Table->AddRow(Row.StateTag.GetTagName(), Row);
			FCombatStatePresentationRow MarkedRow;
			MarkedRow.StateTag = GameTags::State_Combat_Marked;
			Table->AddRow(MarkedRow.StateTag.GetTagName(), MarkedRow);
			Settings->CombatStatePresentationTable = Table;

			World = UWorld::CreateWorld(EWorldType::Game, false);
			World->InitializeActorsForPlay(FURL());
			Dummy = World->SpawnActor<ATestDummy>();
			Dummy->DispatchBeginPlay();
			States = Dummy->GetCombatState();
			Listener = NewObject<UCombatStateTestListener>();
			States->OnStateAdded.AddDynamic(Listener, &UCombatStateTestListener::HandleAdded);
			States->OnStateRemoved.AddDynamic(Listener, &UCombatStateTestListener::HandleRemoved);
			FeedbackListener = NewObject<UFeedbackTestListener>();
			if (UFeedbackSubsystem* Feedback = World->GetSubsystem<UFeedbackSubsystem>())
			{
				Feedback->OnFeedbackPlayed.AddDynamic(FeedbackListener, &UFeedbackTestListener::HandlePlayed);
			}
		});

		AfterEach([this]()
		{
			World->DestroyWorld(false);
			World = nullptr;
			GetMutableDefault<UGameTuningSettings>()->CombatStatePresentationTable = SavedPresentationTable;
		});

		It("has Tick disabled and a timer only while a state is active", [this, Marked]()
		{
			TestFalse("Tick never enabled", States->PrimaryComponentTick.bCanEverTick);
			TestFalse("No timer before", States->HasPendingExpiry());
			States->ApplyState(Marked, 8.f, nullptr);
			TestTrue("Timer while active", States->HasPendingExpiry());
			States->RemoveState(Marked);
			TestFalse("No timer after removal", States->HasPendingExpiry());
			TestEqual("Removed once", Listener->RemovedCount, 1);
		});

		LatentIt("expires a state through the game-time timer", [this, Staggered](const FDoneDelegate& Done)
		{
			States->ApplyState(Staggered, 0.2f, nullptr);
			// A CreateWorld world has no engine context, so the test advances game time and ticks only its timers,
			// once per real frame (the timer manager ticks once per frame and activates new timers on the next one).
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, Staggered, Done](float)
			{
				World->TimeSeconds += 0.1;
				World->GetTimerManager().Tick(0.1f);
				if (States->HasState(Staggered) && World->TimeSeconds < 2.0)
				{
					return true;
				}
				TestFalse("Staggered expired", States->HasState(Staggered));
				TestTrue("Not before its duration", World->TimeSeconds >= 0.2 - 0.001);
				TestEqual("Removed once", Listener->RemovedCount, 1);
				TestFalse("Timer cleared", States->HasPendingExpiry());
				TestEqual("Removed feedback played", FeedbackListener->LastPlayed, FGameplayTag(FeedbackTags::State_Staggered_Removed));
				Done.Execute();
				return false;
			}));
		});

		It("breaks the dummy's poise into Staggered once and plays Staggered feedback once", [this, Staggered]()
		{
			AActor* Hero = World->SpawnActor<ATestDummy>();
			TestFalse("40 poise", States->ApplyPoiseDamage(40.f, Hero));
			TestTrue("10 more breaks", States->ApplyPoiseDamage(10.f, Hero));
			TestTrue("Staggered", States->HasState(Staggered));
			TestEqual("Remaining = StaggerDuration", States->GetStateRemaining(Staggered), 1.5f, 0.001f);
			TestEqual("Instigator recorded", States->GetStateInstigator(Staggered), Hero);
			TestFalse("Further poise ignored", States->ApplyPoiseDamage(50.f, Hero));
			TestEqual("OnStateAdded once", Listener->AddedCount, 1);
			TestEqual("Applied feedback once", FeedbackListener->PlayedCount, 1);
			TestEqual("Applied feedback tag", FeedbackListener->LastPlayed, FGameplayTag(FeedbackTags::State_Staggered_Applied));
		});

		It("refreshes without a second OnStateAdded and keeps the latest instigator", [this, Marked]()
		{
			AActor* First = World->SpawnActor<ATestDummy>();
			AActor* Second = World->SpawnActor<ATestDummy>();
			States->ApplyState(Marked, 8.f, First);
			States->ApplyState(Marked, 3.f, Second);
			TestEqual("Added once", Listener->AddedCount, 1);
			TestEqual("Remaining stays 8", States->GetStateRemaining(Marked), 8.f, 0.001f);
			TestEqual("Latest instigator", States->GetStateInstigator(Marked), Second);
		});

		It("clears every state on death with one OnStateRemoved each, then refuses new states", [this, Staggered, Marked]()
		{
			States->ApplyState(Staggered, 2.f, nullptr);
			States->ApplyState(Marked, 8.f, nullptr);
			FCombatHit Kill;
			Kill.Damage = 10000.f;
			Dummy->GetHealth()->ApplyHit(Kill);
			TestEqual("Two removals", Listener->RemovedCount, 2);
			TestEqual("No states", States->GetStates().Num(), 0);
			States->ApplyState(Marked, 8.f, nullptr);
			TestFalse("Dead owner ignores states", States->HasState(Marked));
		});

		It("uses the Game Tuning default for Staggered and refuses a state with no duration", [this, Staggered, Marked]()
		{
			States->ApplyState(Staggered, 0.f, nullptr);
			TestEqual("Default 1.5 s", States->GetStateRemaining(Staggered), 1.5f, 0.001f);
			AddExpectedError(TEXT("has no duration"), EAutomationExpectedErrorFlags::Contains, 1);
			States->ApplyState(Marked, 0.f, nullptr);
			TestFalse("Marked has no default yet", States->HasState(Marked));
		});
	});
}

#endif
