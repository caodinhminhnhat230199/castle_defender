#include "Combat/CombatActionTiming.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Hero/HeroCombatComponent.h"
#include "Combat/AnimNotifyState_CombatHitWindow.h"

void UAnimNotifyState_RotationAssist::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->OpenRotationAssistWindow();
		}
	}
}

void UAnimNotifyState_RotationAssist::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->CloseRotationAssistWindow();
		}
	}
}

// --- FCombatActionTiming ---

bool FCombatActionTiming::InspectMontage(const UAnimMontage* Montage, FCombatActionTiming& OutTiming, FString* OutError)
{
	if (!Montage)
	{
		if (OutError)
		{
			*OutError = TEXT("Montage is null.");
		}
		return false;
	}

	OutTiming = FCombatActionTiming();
	OutTiming.TotalDuration = Montage->GetPlayLength();
	if (!FMath::IsFinite(OutTiming.TotalDuration) || OutTiming.TotalDuration <= 0.f)
	{
		if (OutError) { *OutError = TEXT("Montage duration must be positive."); }
		return false;
	}

	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		if (NotifyEvent.NotifyStateClass && (NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_CancelWindow>()
			|| NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_CombatHitWindow>()
			|| NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_Invulnerable>()
			|| NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_ParryWindow>()
			|| NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_InterruptResistance>()
			|| NotifyEvent.NotifyStateClass->IsA<UAnimNotifyState_RotationAssist>()))
		{
			const float Start = NotifyEvent.GetTime();
			const float End = Start + NotifyEvent.GetDuration();
			if (!FMath::IsFinite(Start) || !FMath::IsFinite(End) || Start < 0.f
				|| End <= Start || End > OutTiming.TotalDuration + KINDA_SMALL_NUMBER)
			{
				if (OutError) { *OutError = FString::Printf(TEXT("Window %s [%.3f, %.3f] must have positive duration and lie inside montage %s (%.3f)."),
					*GetNameSafe(NotifyEvent.NotifyStateClass), Start, End, *GetNameSafe(Montage), OutTiming.TotalDuration); }
				return false;
			}
		}
		if (const UAnimNotifyState_CancelWindow* CancelNotify = Cast<UAnimNotifyState_CancelWindow>(NotifyEvent.NotifyStateClass))
		{
			if (OutTiming.bHasCancelWindow || CancelNotify->AllowedActions.IsEmpty())
			{
				if (OutError) { *OutError = TEXT("Expected one nonempty cancel window for this action."); }
				return false;
			}
			OutTiming.bHasCancelWindow = true;
			OutTiming.CancelWindowStart = NotifyEvent.GetTime();
			OutTiming.CancelWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
			OutTiming.AllowedCancelActions = CancelNotify->AllowedActions;
		}
		else if (Cast<UAnimNotifyState_CombatHitWindow>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasHitWindow = true;
			OutTiming.HitWindowCount++;
			OutTiming.HitWindowStart = NotifyEvent.GetTime();
			OutTiming.HitWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
		}
		else if (Cast<UAnimNotifyState_Invulnerable>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasInvulnerableWindow = true;
			++OutTiming.InvulnerableWindowCount;
			OutTiming.InvulnerableWindowStart = NotifyEvent.GetTime();
			OutTiming.InvulnerableWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
		}
		else if (Cast<UAnimNotifyState_ParryWindow>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasParryWindow = true;
			++OutTiming.ParryWindowCount;
			OutTiming.ParryWindowStart = NotifyEvent.GetTime();
			OutTiming.ParryWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
		}
		else if (Cast<UAnimNotifyState_RotationAssist>(NotifyEvent.NotifyStateClass))
		{
			if (OutTiming.bHasRotationAssistWindow)
			{
				if (OutError) { *OutError = TEXT("Expected at most one rotation assist window per attack."); }
				return false;
			}
			OutTiming.bHasRotationAssistWindow = true;
			OutTiming.RotationAssistWindowStart = NotifyEvent.GetTime();
			OutTiming.RotationAssistWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
		}
	}

	if (OutTiming.bHasHitWindow && (OutTiming.HitWindowStart <= 0.f
		|| OutTiming.HitWindowEnd >= OutTiming.TotalDuration
		|| (OutTiming.bHasCancelWindow && OutTiming.CancelWindowStart + KINDA_SMALL_NUMBER < OutTiming.HitWindowEnd)))
	{
		if (OutError) { *OutError = TEXT("Attack requires Startup, Active and Recovery; cancellation cannot precede Recovery."); }
		return false;
	}
	if (OutTiming.bHasRotationAssistWindow && !OutTiming.bHasHitWindow)
	{
		if (OutError) { *OutError = TEXT("Rotation assist requires an attack hit window."); }
		return false;
	}
	return true;
}

bool FCombatActionTiming::AddHitWindow(UAnimMontage* Montage, float StartTime, float Duration)
{
	if (!Montage || Duration <= 0.f || StartTime < 0.f)
	{
		return false;
	}

	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	UAnimNotifyState_CombatHitWindow* HitNotify = NewObject<UAnimNotifyState_CombatHitWindow>(Montage, NAME_None, RF_Transactional);
	Event.NotifyStateClass = HitNotify;
	Event.Link(Montage, StartTime);
	Event.SetTime(StartTime);
	Event.SetDuration(Duration);
	Event.EndLink.Link(Montage, StartTime + Duration);
	Event.EndLink.SetTime(StartTime + Duration);
	Event.NotifyName = FName(TEXT("CombatHitWindow"));
	return true;
}

bool FCombatActionTiming::AddCancelWindow(UAnimMontage* Montage, float StartTime, float Duration, const TArray<EHeroAction>& AllowedActions)
{
	if (!Montage || Duration <= 0.f || StartTime < 0.f)
	{
		return false;
	}

	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	UAnimNotifyState_CancelWindow* CancelNotify = NewObject<UAnimNotifyState_CancelWindow>(Montage, NAME_None, RF_Transactional);
	CancelNotify->AllowedActions = AllowedActions;
	Event.NotifyStateClass = CancelNotify;
	Event.Link(Montage, StartTime);
	Event.SetTime(StartTime);
	Event.SetDuration(Duration);
	Event.EndLink.Link(Montage, StartTime + Duration);
	Event.EndLink.SetTime(StartTime + Duration);
	Event.NotifyName = FName(TEXT("CancelWindow"));
	return true;
}

// --- UAnimNotifyState_CancelWindow ---

void UAnimNotifyState_CancelWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->OpenCancelWindow(AllowedActions);
		}
	}
}

void UAnimNotifyState_CancelWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->CloseCancelWindow();
		}
	}
}

FString UAnimNotifyState_CancelWindow::GetNotifyName_Implementation() const
{
	return TEXT("Cancel Window");
}

// --- UAnimNotifyState_Invulnerable ---

void UAnimNotifyState_Invulnerable::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->OpenInvulnerableWindow();
		}
	}
}

void UAnimNotifyState_Invulnerable::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->CloseInvulnerableWindow();
		}
	}
}

FString UAnimNotifyState_Invulnerable::GetNotifyName_Implementation() const
{
	return TEXT("Invulnerable Window");
}

// --- UAnimNotifyState_ParryWindow ---

void UAnimNotifyState_ParryWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->OpenParryWindow();
		}
	}
}

void UAnimNotifyState_ParryWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (UHeroCombatComponent* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>())
		{
			Combat->CloseParryWindow();
		}
	}
}

FString UAnimNotifyState_ParryWindow::GetNotifyName_Implementation() const
{
	return TEXT("Parry Window");
}

void UAnimNotifyState_InterruptResistance::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (auto* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>()) { Combat->OpenInterruptResistanceWindow(); }
	}
}

void UAnimNotifyState_InterruptResistance::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (MeshComp && MeshComp->GetOwner())
	{
		if (auto* Combat = MeshComp->GetOwner()->FindComponentByClass<UHeroCombatComponent>()) { Combat->CloseInterruptResistanceWindow(); }
	}
}
