#include "Hero/HeroCombatLibrary.h"
#include "Animation/AnimMontage.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/AnimNotifyState_CombatHitWindow.h"

bool UHeroCombatLibrary::AddCombatHitWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration)
{
	return FCombatActionTiming::AddHitWindow(Montage, StartTime, Duration);
}

bool UHeroCombatLibrary::AddCancelWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration, const TArray<EHeroAction>& AllowedActions)
{
	return FCombatActionTiming::AddCancelWindow(Montage, StartTime, Duration, AllowedActions);
}

void UHeroCombatLibrary::ClearCombatNotifiesFromMontage(UAnimMontage* Montage)
{
	if (!Montage)
	{
		return;
	}

	Montage->Notifies.RemoveAll([](const FAnimNotifyEvent& Event)
	{
		return Event.NotifyStateClass && (
			Event.NotifyStateClass->IsA<UAnimNotifyState_CombatHitWindow>() ||
			Event.NotifyStateClass->IsA<UAnimNotifyState_CancelWindow>() ||
			Event.NotifyStateClass->IsA<UAnimNotifyState_Invulnerable>()
		);
	});
}

bool UHeroCombatLibrary::AddInvulnerableWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration)
{
#if WITH_EDITOR
	if (!Montage || StartTime < 0.f || Duration <= 0.f || StartTime + Duration > Montage->GetPlayLength()) { return false; }
	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	Event.NotifyStateClass = NewObject<UAnimNotifyState_Invulnerable>(Montage, NAME_None, RF_Transactional);
	Event.Link(Montage, StartTime);
	Event.SetTime(StartTime);
	Event.SetDuration(Duration);
	Event.EndLink.Link(Montage, StartTime + Duration);
	Event.EndLink.SetTime(StartTime + Duration);
	Event.NotifyName = FName(TEXT("Invulnerable"));
	return true;
#else
	return false;
#endif
}

bool UHeroCombatLibrary::SetSingleSegmentMontageDuration(UAnimMontage* Montage, float Duration)
{
#if WITH_EDITOR
	if (!Montage || Duration <= 0.f || Montage->SlotAnimTracks.Num() != 1
		|| Montage->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() != 1) { return false; }
	FAnimSegment& Segment = Montage->SlotAnimTracks[0].AnimTrack.AnimSegments[0];
	Segment.AnimPlayRate = (Segment.AnimEndTime - Segment.AnimStartTime) / Duration;
	Montage->SetCompositeLength(Montage->CalculateSequenceLength());
	return true;
#else
	return false;
#endif
}
