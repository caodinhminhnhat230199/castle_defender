#include "UI/LockOnMarkerWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Hero/HeroCharacter.h"
#include "Hero/LockOnComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanelSlot.h"

bool ULockOnMarkerWidget::SetEditorRoot(UWidgetTree* Tree, UWidget* Root)
{
#if WITH_EDITOR
	if (!Tree || !Root || Root->GetOuter() != Tree) { return false; }
	Tree->Modify();
	Tree->RootWidget = Root;
	return true;
#else
	return false;
#endif
}

void ULockOnMarkerWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	SetDesiredSizeInViewport(MarkerSize);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void ULockOnMarkerWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	// Tick only while mounted for a lock: UI projection follows the rendered camera each frame.
	const AHeroCharacter* Hero = Cast<AHeroCharacter>(GetOwningPlayerPawn());
	FVector2D Screen = FVector2D::ZeroVector;
	const bool bVisible = Hero && Hero->GetLockOnComponent()->GetLockOnTarget()
		&& UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Hero->GetLockOnComponent()->GetTargetLocation(), Screen, true);
	// Opacity keeps projection alive if the target temporarily moves behind the camera.
	SetRenderOpacity(bVisible ? 1.f : 0.f);
	if (bVisible)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
		{
			CanvasSlot->SetPosition(Screen);
			CanvasSlot->SetSize(MarkerSize);
		}
		else { SetPositionInViewport(Screen, false); } // Widget projection already removes DPI scale.
	}
}
