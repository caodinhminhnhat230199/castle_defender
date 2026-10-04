#include "Combat/CombatActionTiming.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Hero/HeroCombatComponent.h"

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

	for (const FAnimNotifyEvent& NotifyEvent : Montage->Notifies)
	{
		if (const UAnimNotifyState_CancelWindow* CancelNotify = Cast<UAnimNotifyState_CancelWindow>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasCancelWindow = true;
			OutTiming.CancelWindowStart = NotifyEvent.GetTime();
			OutTiming.CancelWindowEnd = NotifyEvent.GetTime() + NotifyEvent.GetDuration();
			OutTiming.AllowedCancelActions = CancelNotify->AllowedActions;
		}
		else if (Cast<UAnimNotifyState_Invulnerable>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasInvulnerableWindow = true;
		}
		else if (Cast<UAnimNotifyState_ParryWindow>(NotifyEvent.NotifyStateClass))
		{
			OutTiming.bHasParryWindow = true;
		}
	}

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
