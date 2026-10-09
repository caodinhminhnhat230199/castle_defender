#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LockOnMarkerWidget.generated.h"

class UWidgetTree;

/** Projection-only observer. WBP_LockOnMarker owns appearance; controller owns viewport lifetime. */
UCLASS(Abstract)
class CASTLEDEFENDER_API ULockOnMarkerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** Editor Python bridge: WidgetTree.RootWidget is not exposed by UE 5.8 Python. */
	UFUNCTION(BlueprintCallable, Category = "Marker|Authoring")
	static bool SetEditorRoot(UWidgetTree* Tree, UWidget* Root);
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Marker")
	FVector2D MarkerSize = FVector2D(24.f, 24.f);
};
