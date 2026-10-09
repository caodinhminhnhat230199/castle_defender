#include "Hero/HeroCombatLibrary.h"
#include "Animation/AnimMontage.h"
#include "Combat/CombatActionTiming.h"
#include "Combat/AnimNotifyState_CombatHitWindow.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#if WITH_EDITOR
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#endif

bool UHeroCombatLibrary::FinalizeStrafeBlendSpace(UBlendSpace* BlendSpace)
{
#if WITH_EDITOR
	if (!BlendSpace || BlendSpace->GetNumberOfBlendSamples() == 0) { return false; }
	BlendSpace->Modify();
	BlendSpace->ResampleData();
	return true;
#else
	return false;
#endif
}

bool UHeroCombatLibrary::AuthorSideDodge(UAnimSequence* SideStep, UAnimSequence* Dash, float DashEndTime, bool bRight)
{
#if WITH_EDITOR
	if (!SideStep || !Dash || SideStep == Dash || SideStep->GetSkeleton() != Dash->GetSkeleton()
		|| DashEndTime <= 0.f || DashEndTime > Dash->GetPlayLength()) { return false; }
	const IAnimationDataModel* Model = SideStep->GetDataModel();
	const IAnimationDataModel* DashModel = Dash->GetDataModel();
	TArray<FTransform> Keys;
	Model->GetBoneTrackTransforms(TEXT("root"), Keys);
	if (Keys.Num() < 2) { return false; }
	const FVector Start = Keys[0].GetTranslation();
	const FTransform DashStart = DashModel->EvaluateBoneTrackTransform(TEXT("root"), FFrameTime(0), EAnimInterpolationType::Linear);
	TArray<FVector> Positions, Scales;
	TArray<FQuat> Rotations;
	for (int32 Index = 0; Index < Keys.Num(); ++Index)
	{
		const double Time = DashEndTime * Index / (Keys.Num() - 1);
		const FTransform DashKey = DashModel->EvaluateBoneTrackTransform(TEXT("root"), DashModel->GetFrameRate().AsFrameTime(Time), EAnimInterpolationType::Linear);
		const FVector Travel = DashKey.GetTranslation() - DashStart.GetTranslation();
		Positions.Add(Start + FRotator(0.f, bRight ? 90.f : -90.f, 0.f).RotateVector(Travel));
		Rotations.Add(Keys[Index].GetRotation());
		Scales.Add(Keys[Index].GetScale3D());
	}
	SideStep->Modify();
	const bool bSuccess = SideStep->GetController().SetBoneTrackKeys(TEXT("root"), Positions, Rotations, Scales, false);
	SideStep->bEnableRootMotion = true;
	SideStep->RootMotionRootLock = ERootMotionRootLock::AnimFirstFrame;
	return bSuccess;
#else
	return false;
#endif
}

bool UHeroCombatLibrary::AddCombatHitWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration)
{
	return FCombatActionTiming::AddHitWindow(Montage, StartTime, Duration);
}

bool UHeroCombatLibrary::EnsureRotationAssistWindow(UAnimMontage* Montage)
{
#if WITH_EDITOR
	FCombatActionTiming Timing;
	if (!FCombatActionTiming::InspectMontage(Montage, Timing) || Timing.HitWindowCount != 1) { return false; }
	if (Timing.bHasRotationAssistWindow) { return true; }
	// The initial assist covers authored startup through the hit; subsequent editor tuning is preserved.
	Montage->Modify();
	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	Event.NotifyStateClass = NewObject<UAnimNotifyState_RotationAssist>(Montage, NAME_None, RF_Transactional);
	Event.Link(Montage, 0.f);
	Event.SetTime(0.f);
	Event.SetDuration(Timing.HitWindowEnd);
	Event.EndLink.Link(Montage, Timing.HitWindowEnd);
	Event.EndLink.SetTime(Timing.HitWindowEnd);
	Event.NotifyName = FName(TEXT("RotationAssist"));
	return true;
#else
	return false;
#endif
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
			Event.NotifyStateClass->IsA<UAnimNotifyState_Invulnerable>() ||
			Event.NotifyStateClass->IsA<UAnimNotifyState_ParryWindow>() ||
			Event.NotifyStateClass->IsA<UAnimNotifyState_RotationAssist>()
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

bool UHeroCombatLibrary::AddParryWindowToMontage(UAnimMontage* Montage, float StartTime, float Duration)
{
#if WITH_EDITOR
	if (!Montage || !FMath::IsFinite(StartTime) || !FMath::IsFinite(Duration) || StartTime < 0.f || Duration <= 0.f || StartTime + Duration >= Montage->GetPlayLength()) { return false; }
	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	Event.NotifyStateClass = NewObject<UAnimNotifyState_ParryWindow>(Montage, NAME_None, RF_Transactional);
	Event.Link(Montage, StartTime); Event.SetTime(StartTime); Event.SetDuration(Duration);
	Event.EndLink.Link(Montage, StartTime + Duration); Event.EndLink.SetTime(StartTime + Duration);
	Event.NotifyName = FName(TEXT("ParryWindow"));
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

bool UHeroCombatLibrary::SetSingleSegmentMontageSource(UAnimMontage* Montage, UAnimSequenceBase* Sequence, float AnimStartTime, float AnimEndTime, float Duration, bool bPlayReversed)
{
#if WITH_EDITOR
	if (!Montage || !Sequence || !Sequence->GetSkeleton() || Duration <= 0.f || AnimStartTime < 0.f
		|| AnimEndTime <= AnimStartTime || AnimEndTime > Sequence->GetPlayLength() + KINDA_SMALL_NUMBER
		|| Montage->SlotAnimTracks.Num() != 1 || Montage->SlotAnimTracks[0].AnimTrack.AnimSegments.Num() != 1) { return false; }
	Montage->Modify();
	Montage->SetSkeleton(Sequence->GetSkeleton());
	FAnimSegment& Segment = Montage->SlotAnimTracks[0].AnimTrack.AnimSegments[0];
	Segment.SetAnimReference(Sequence);
	Segment.AnimStartTime = AnimStartTime;
	Segment.AnimEndTime = AnimEndTime;
	// A negative segment rate plays the clip (and its root motion) backwards.
	Segment.AnimPlayRate = (AnimEndTime - AnimStartTime) / Duration * (bPlayReversed ? -1.f : 1.f);
	Montage->SetCompositeLength(Montage->CalculateSequenceLength());
	return true;
#else
	return false;
#endif
}
