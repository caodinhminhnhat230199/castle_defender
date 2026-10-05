#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GameTuningSettings.h"
#include "Engine/World.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "Tests/FeedbackTestListener.h"

BEGIN_DEFINE_SPEC(FFeedbackThrottleSpec, "CastleDefender.Feedback.Throttle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
	FFeedbackThrottle Throttle;
	UWorld* World = nullptr;
	UFeedbackSubsystem* Feedback = nullptr;
	UFeedbackTestListener* Listener = nullptr;
END_DEFINE_SPEC(FFeedbackThrottleSpec)

void FFeedbackThrottleSpec::Define()
{
	const FGameplayTag Tag = FeedbackTags::Combat_Hit_Light;

	Describe("FFeedbackThrottle (R-UXF-03)", [this, Tag]()
	{
		BeforeEach([this]() { Throttle.Reset(); });

		It("rejects a replay inside the cooldown and accepts one after it", [this, Tag]()
		{
			TestTrue("t=0 plays", Throttle.TryPlay(Tag, nullptr, 0.0, 1.f, false, 0, 0.f));
			TestFalse("t=0.5 rejected", Throttle.TryPlay(Tag, nullptr, 0.5, 1.f, false, 0, 0.f));
			TestTrue("t=1.1 plays", Throttle.TryPlay(Tag, nullptr, 1.1, 1.f, false, 0, 0.f));
		});

		It("keeps a separate cooldown per actor when the row asks for it", [this, Tag]()
		{
			UObject* A = NewObject<UFeedbackTestListener>();
			UObject* B = NewObject<UFeedbackTestListener>();
			TestTrue("Actor A plays", Throttle.TryPlay(Tag, A, 0.0, 1.f, true, 0, 0.f));
			TestTrue("Actor B plays", Throttle.TryPlay(Tag, B, 0.1, 1.f, true, 0, 0.f));
			TestFalse("Actor A again rejected", Throttle.TryPlay(Tag, A, 0.2, 1.f, true, 0, 0.f));
			TestTrue("Global cooldown: A plays", Throttle.TryPlay(Tag, A, 5.0, 1.f, false, 0, 0.f));
			TestFalse("Global cooldown: B shares it", Throttle.TryPlay(Tag, B, 5.1, 1.f, false, 0, 0.f));
		});

		It("caps 20 plays in one frame at the burst limit, then reopens after the window", [this, Tag]()
		{
			int32 Played = 0;
			for (int32 Index = 0; Index < 20; ++Index)
			{
				Played += Throttle.TryPlay(Tag, nullptr, 2.0, 0.f, false, 4, 0.25f) ? 1 : 0;
			}
			TestEqual("Burst limit 4 / 0.25 s", Played, 4);
			TestFalse("Still inside the window", Throttle.TryPlay(Tag, nullptr, 2.2, 0.f, false, 4, 0.25f));
			TestTrue("Window passed", Throttle.TryPlay(Tag, nullptr, 2.26, 0.f, false, 4, 0.25f));
		});
	});

	Describe("UFeedbackSubsystem (AC-UXF-01)", [this, Tag]()
	{
		BeforeEach([this, Tag]()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			Feedback = World->GetSubsystem<UFeedbackSubsystem>();
			Listener = NewObject<UFeedbackTestListener>();
			if (Feedback)
			{
				Feedback->SetFeedbackTable(UFeedbackTestListener::MakeTable({ Tag }));
				Feedback->OnFeedbackPlayed.AddDynamic(Listener, &UFeedbackTestListener::HandlePlayed);
				Feedback->OnHUDLayersChanged.AddDynamic(Listener, &UFeedbackTestListener::HandleLayersChanged);
			}
		});

		AfterEach([this]()
		{
			World->DestroyWorld(false);
			World = nullptr;
			Feedback = nullptr;
		});

		It("exists in game worlds and loads DT_Feedback from Game Tuning", [this]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			UWorld* Fresh = UWorld::CreateWorld(EWorldType::Game, false);
			UFeedbackSubsystem* FreshFeedback = Fresh->GetSubsystem<UFeedbackSubsystem>();
			TestTrue("Table loaded: Hit.Light row plays", FreshFeedback && FreshFeedback->Play(FeedbackTags::Combat_Hit_Light, FFeedbackEventContext()));
			Fresh->DestroyWorld(false);
		});

		It("plays a known tag once and broadcasts the row tag", [this, Tag]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			TestTrue("Played", Feedback->Play(Tag, FFeedbackEventContext()));
			TestEqual("Broadcast count", Listener->PlayedCount, 1);
			TestEqual("Broadcast tag", Listener->LastPlayed, Tag);
		});

		It("warns once for a tag without a row and does not crash", [this]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			AddExpectedError(TEXT("No DT_Feedback row for Feedback.Combat.Parry"), EAutomationExpectedErrorFlags::Contains, 1);
			TestFalse("First play is a no-op", Feedback->Play(FeedbackTags::Combat_Parry, FFeedbackEventContext()));
			TestFalse("Second play is a no-op", Feedback->Play(FeedbackTags::Combat_Parry, FFeedbackEventContext()));
			TestEqual("Nothing broadcast", Listener->PlayedCount, 0);
		});

		It("plays at most the default burst limit for 20 calls in one frame", [this, Tag]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			for (int32 Index = 0; Index < 20; ++Index)
			{
				Feedback->Play(Tag, FFeedbackEventContext());
			}
			TestEqual("Burst limit", Listener->PlayedCount, UGameTuningSettings::Get()->DefaultBurstLimit);
		});

		It("broadcasts a HUD layer change once and derives the tactical display", [this]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			Feedback->SetHUDLayerActive(EHUDLayer::TacticalFocus, true);
			Feedback->SetHUDLayerActive(EHUDLayer::TacticalFocus, true);
			TestEqual("One broadcast", Listener->LayersChangedCount, 1);
			TestEqual("Mask", Listener->LastLayerMask, static_cast<int32>(EHUDLayer::TacticalFocus));
			TestTrue("Tactical display under Focus", Feedback->IsTacticalDisplay());
			Feedback->SetHUDLayerActive(EHUDLayer::Modal, true);
			TestEqual("Modal on top", Feedback->GetTopHUDLayer(), EHUDLayer::Modal);
			TestFalse("Modal hides tactical display", Feedback->IsTacticalDisplay());
			Feedback->SetHUDLayerActive(EHUDLayer::Modal, false);
			Feedback->SetHUDLayerActive(EHUDLayer::TacticalFocus, false);
			TestEqual("Back to Combat", Feedback->GetTopHUDLayer(), EHUDLayer::None);
			TestEqual("Four broadcasts", Listener->LayersChangedCount, 4);
		});

		It("lists declared Feedback tags that have not played", [this, Tag]()
		{
			if (!TestNotNull("Subsystem", Feedback)) { return; }
			TestTrue("Hit.Light unplayed before", Feedback->GetUnplayedDeclaredTags().Contains(Tag));
			Feedback->Play(Tag, FFeedbackEventContext());
			const TArray<FGameplayTag> Unplayed = Feedback->GetUnplayedDeclaredTags();
			TestFalse("Hit.Light played", Unplayed.Contains(Tag));
			TestTrue("Armored variant still unplayed", Unplayed.Contains(FeedbackTags::Combat_Hit_Light_Armored));
			TestFalse("Implicit parent not listed", Unplayed.Contains(FGameplayTag::RequestGameplayTag(TEXT("Feedback.Combat.Hit"))));
		});
	});
}

#endif
