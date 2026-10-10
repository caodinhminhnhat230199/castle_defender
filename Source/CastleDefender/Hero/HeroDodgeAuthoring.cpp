#include "Hero/HeroCombatLibrary.h"
#include "Animation/AnimSequence.h"
#if WITH_EDITOR
#include "Animation/Skeleton.h"
#include "Animation/AnimData/IAnimationDataController.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#endif

bool UHeroCombatLibrary::AuthorCustomDodge(UAnimSequence* Output, UAnimSequence* ReferenceIdle, EHeroDodgeDirection Direction, float Duration, float TravelDistance)
{
#if WITH_EDITOR
	if (!Output || !ReferenceIdle || Output == ReferenceIdle || !ReferenceIdle->GetSkeleton()
		|| Output->GetSkeleton() != ReferenceIdle->GetSkeleton() || !FMath::IsFinite(Duration) || Duration <= 0.f
		|| !FMath::IsFinite(TravelDistance) || TravelDistance <= 0.f) { return false; }
	const FReferenceSkeleton& Skeleton = ReferenceIdle->GetSkeleton()->GetReferenceSkeleton();
	const IAnimationDataModel* Model = ReferenceIdle->GetDataModel();
	const int32 BoneCount = Skeleton.GetNum();
	const int32 Frames = FMath::Max(1, FMath::RoundToInt(Duration * 60.f));
	TArray<FTransform> Idle = Skeleton.GetRefBonePose();
	for (int32 Bone = 0; Bone < BoneCount; ++Bone)
	{
		const FName Name = Skeleton.GetBoneName(Bone);
		if (Model->IsValidBoneTrackName(Name)) { Idle[Bone] = Model->EvaluateBoneTrackTransform(Name, FFrameTime(0), EAnimInterpolationType::Linear); }
	}
	const int32 Root = Skeleton.FindBoneIndex(TEXT("root"));
	const int32 Pelvis = Skeleton.FindBoneIndex(TEXT("pelvis"));
	if (Root == INDEX_NONE || Pelvis == INDEX_NONE) { return false; }
	TArray<TArray<FTransform>> Keys;
	Keys.SetNum(BoneCount);
	const bool bRoll = Direction == EHeroDodgeDirection::Forward || Direction == EHeroDodgeDirection::Backward;
	const float Sign = Direction == EHeroDodgeDirection::Backward || Direction == EHeroDodgeDirection::Right ? -1.f : 1.f;
	// Saved mannequin mesh has -90 yaw: local +Y is actor forward, +X is actor left.
	const FVector TravelAxis = bRoll ? FVector(0.f, Sign, 0.f) : FVector(Sign, 0.f, 0.f);
	const FVector RightAxis(-1.f, 0.f, 0.f);
	auto Smooth = [](float Value) { const float T = FMath::Clamp(Value, 0.f, 1.f); return T * T * (3.f - 2.f * T); };
	for (int32 Frame = 0; Frame <= Frames; ++Frame)
	{
		const float T = static_cast<float>(Frame) / Frames;
		const float Envelope = Smooth(T / 0.16f) * (1.f - Smooth((T - 0.72f) / 0.28f));
		TArray<FTransform> Local = Idle;
		TArray<FTransform> Global;
		Global.SetNum(BoneCount);
		auto FK = [&]()
		{
			for (int32 Bone = 0; Bone < BoneCount; ++Bone)
			{
				const int32 Parent = Skeleton.GetParentIndex(Bone);
				Global[Bone] = Parent == INDEX_NONE ? Local[Bone] : Local[Bone] * Global[Parent];
			}
		};
		auto Rotate = [&](FName Name, const FVector& Axis, float Degrees)
		{
			const int32 Bone = Skeleton.FindBoneIndex(Name);
			if (Bone == INDEX_NONE) { return; }
			FK();
			FQuat Rotation = FQuat(Axis, FMath::DegreesToRadians(Degrees)) * Global[Bone].GetRotation();
			const int32 Parent = Skeleton.GetParentIndex(Bone);
			if (Parent != INDEX_NONE) { Rotation = Global[Parent].GetRotation().Inverse() * Rotation; }
			Local[Bone].SetRotation(Rotation.GetNormalized());
		};
		Local[Root].SetTranslation(Idle[Root].GetTranslation() + TravelAxis * TravelDistance * Smooth((T - 0.05f) / 0.84f));
		const float RollAngle = bRoll ? Sign * 360.f * Smooth((T - 0.11f) / 0.72f) : 0.f;
		Rotate(TEXT("pelvis"), bRoll ? RightAxis : FVector(0.f, 1.f, 0.f), bRoll ? RollAngle : -Sign * 24.f * Envelope);
		const FQuat BodyRoll(RightAxis, FMath::DegreesToRadians(RollAngle));
		const FVector BendAxis = BodyRoll.RotateVector(RightAxis);
		Rotate(TEXT("spine_01"), BendAxis, -24.f * Envelope);
		Rotate(TEXT("spine_02"), BendAxis, -14.f * Envelope);
		Rotate(TEXT("head"), BendAxis, -18.f * Envelope);
		for (const TCHAR* Side : { TEXT("l"), TEXT("r") })
		{
			const bool bLead = (Direction == EHeroDodgeDirection::Left) == (FCString::Strcmp(Side, TEXT("l")) == 0);
			Rotate(FName(*FString::Printf(TEXT("thigh_%s"), Side)), BendAxis, -(bRoll ? 95.f : bLead ? 45.f : 25.f) * Envelope);
			Rotate(FName(*FString::Printf(TEXT("calf_%s"), Side)), BendAxis, (bRoll ? 130.f : bLead ? 72.f : 48.f) * Envelope);
			Rotate(FName(*FString::Printf(TEXT("foot_%s"), Side)), BendAxis, -25.f * Envelope);
			Rotate(FName(*FString::Printf(TEXT("upperarm_%s"), Side)), BendAxis, -65.f * Envelope);
			Rotate(FName(*FString::Printf(TEXT("lowerarm_%s"), Side)), BendAxis, -55.f * Envelope);
		}
		Local[Pelvis].AddToTranslation(FVector(0.f, 0.f, -(bRoll ? 62.f : 30.f) * Envelope));
		FK();
		// Floor correction uses body endpoints, excluding IK helper bones.
		float Lowest = TNumericLimits<float>::Max();
		for (const TCHAR* Name : { TEXT("foot_l"), TEXT("foot_r"), TEXT("ball_l"), TEXT("ball_r"), TEXT("hand_l"), TEXT("hand_r"), TEXT("head"), TEXT("spine_03") })
		{
			const int32 Bone = Skeleton.FindBoneIndex(Name);
			if (Bone != INDEX_NONE) { Lowest = FMath::Min(Lowest, static_cast<float>(Global[Bone].GetTranslation().Z)); }
		}
		const float FloorClearance = 12.f * Envelope;
		if (Lowest < FloorClearance) { Local[Pelvis].AddToTranslation(FVector(0.f, 0.f, FloorClearance - Lowest)); }
		for (int32 Bone = 0; Bone < BoneCount; ++Bone) { Keys[Bone].Add(Local[Bone]); }
	}
	Output->Modify();
	IAnimationDataController& Controller = Output->GetController();
	Controller.OpenBracket(FText::FromString(TEXT("Author original directional dodge")), false);
	Controller.RemoveAllBoneTracks(false);
	Controller.SetFrameRate(FFrameRate(60, 1), false);
	Controller.SetNumberOfFrames(FFrameNumber(Frames), false);
	bool bSuccess = true;
	for (int32 Bone = 0; Bone < BoneCount; ++Bone)
	{
		TArray<FVector> Positions, Scales;
		TArray<FQuat> Rotations;
		for (const FTransform& Key : Keys[Bone]) { Positions.Add(Key.GetTranslation()); Rotations.Add(Key.GetRotation()); Scales.Add(Key.GetScale3D()); }
		const FName Name = Skeleton.GetBoneName(Bone);
		bSuccess &= Controller.AddBoneCurve(Name, false) && Controller.SetBoneTrackKeys(Name, Positions, Rotations, Scales, false);
	}
	Controller.CloseBracket(false);
	Output->bEnableRootMotion = true;
	Output->RootMotionRootLock = ERootMotionRootLock::AnimFirstFrame;
	return bSuccess;
#else
	return false;
#endif
}
