#include "UI/GameHUDWidget.h"
#include "UI/HeroVitalsWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Feedback/FeedbackSubsystem.h"

void UGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Feedback = UFeedbackSubsystem::Get(this);
	if (Feedback.IsValid()) { Feedback->OnHUDLayersChanged.AddUniqueDynamic(this, &UGameHUDWidget::HandleLayers); }
	HandleLayers(Feedback.IsValid() ? static_cast<int32>(Feedback->GetHUDLayers()) : 0);
}
void UGameHUDWidget::HandleLayers(int32 Flags)
{
	const EHUDLayer Top = Feedback.IsValid() ? Feedback->GetTopHUDLayer() : EHUDLayer::None;
	HeroVitals->ApplyLayer(Top);
	LockOnLayer->SetVisibility(Top == EHUDLayer::None || Top == EHUDLayer::CommandWheel ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	TArray<UWidget*> Widgets;
	WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* Widget : Widgets)
	{
		if (UNamedSlot* NamedSlot = Cast<UNamedSlot>(Widget))
		{
			// Empty extension points stay collapsed until their owning feature supplies a panel.
			NamedSlot->SetVisibility(NamedSlot->GetContent() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
	}
	ApplyLayers(Flags);
}
void UGameHUDWidget::SetLockOnMarker(UUserWidget* Marker)
{
	LockOnLayer->ClearChildren();
	if (Marker)
	{
		UCanvasPanelSlot* CanvasSlot = LockOnLayer->AddChildToCanvas(Marker);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	}
}
void UGameHUDWidget::NativeDestruct()
{
	if (Feedback.IsValid()) { Feedback->OnHUDLayersChanged.RemoveDynamic(this, &UGameHUDWidget::HandleLayers); }
	Feedback.Reset();
	Super::NativeDestruct();
}
