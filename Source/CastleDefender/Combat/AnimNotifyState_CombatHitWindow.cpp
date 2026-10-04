#include "Combat/AnimNotifyState_CombatHitWindow.h"

#include "Combat/MeleeTraceComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_CombatHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UMeleeTraceComponent* TraceComp = MeshComp->GetOwner()->FindComponentByClass<UMeleeTraceComponent>())
		{
			TraceComp->BeginHitWindow();
		}
	}
}

void UAnimNotifyState_CombatHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (UMeleeTraceComponent* TraceComp = MeshComp->GetOwner()->FindComponentByClass<UMeleeTraceComponent>())
		{
			TraceComp->EndHitWindow();
		}
	}
}
