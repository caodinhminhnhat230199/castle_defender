#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UCanvasPanel;
class UHeroVitalsWidget;
class UFeedbackSubsystem;

/** Fixed Blueprint slots; observes world HUD layers, owns no gameplay or player mode. */
UCLASS(Abstract)
class CASTLEDEFENDER_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetLockOnMarker(UUserWidget* Marker);
	UFUNCTION(BlueprintPure, Category = "HUD")
	UHeroVitalsWidget* GetHeroVitals() const { return HeroVitals; }
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void ApplyLayers(int32 Flags);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	TObjectPtr<UHeroVitalsWidget> HeroVitals;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	TObjectPtr<UCanvasPanel> LockOnLayer;
private:
	UFUNCTION()
	void HandleLayers(int32 Flags);
	TWeakObjectPtr<UFeedbackSubsystem> Feedback;
};
