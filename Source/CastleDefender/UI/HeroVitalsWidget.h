#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Combat/CombatTypes.h"
#include "Feedback/FeedbackTypes.h"
#include "Containers/Ticker.h"
#include "TimerManager.h"
#include "HeroVitalsWidget.generated.h"

class AHeroCharacter;
class UHealthComponent;
class UStaminaComponent;
class UProgressBar;
class UFeedbackSubsystem;

/** Delegate-bound view of the possessed Hero. Blueprint owns bar layout and styling. */
UCLASS(Abstract)
class CASTLEDEFENDER_API UHeroVitalsWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void BindHero(AHeroCharacter* Hero);
	void ApplyLayer(EHUDLayer Layer);
	UFUNCTION(BlueprintPure, Category = "HUD")
	AHeroCharacter* GetObservedHero() const;
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetHealthFraction() const { return HealthFraction; }
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetStaminaTargetFraction() const { return StaminaTarget; }
	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsLowHealthLatched() const { return bLowHealthLatched; }
	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsDamageFlashActive() const { return bDamageFlashActive; }
	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetDamageVignetteOpacity() const { return DamageVignetteOpacity; }
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void FlashDamageVignette();
	void SetDamageVignette(UUserWidget* InVignette);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ClampMin = "0.001"))
	float StaminaLerpSpeed = 12.f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ClampMin = "0"))
	float StaminaFlashSeconds = 0.2f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	FLinearColor StaminaFlashColor = FLinearColor(1.f, 0.85f, 0.2f);
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ClampMin = "0", ClampMax = "1"))
	float FocusOpacity = 0.4f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<UUserWidget> DamageVignetteClass;
	UPROPERTY(BlueprintReadOnly, Category = "HUD", meta = (BindWidgetOptional))
	TObjectPtr<UUserWidget> DamageVignette;
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ClampMin = "0.01", ClampMax = "0.3"))
	float DamageFlashSeconds = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "HUD", meta = (ClampMin = "0.1"))
	float LowHealthPulseFrequency = 1.2f;

private:
	UFUNCTION()
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION()
	void HandleDamaged(const FCombatHit& Hit, float NewHealth);
	UFUNCTION()
	void HandleHealed(float Amount, float NewHealth);
	UFUNCTION()
	void HandleDeath(const FCombatHit& Hit);
	UFUNCTION()
	void HandleStaminaChanged(float Current, float Maximum);
	UFUNCTION()
	void HandleStaminaFailed(float Cost);
	UFUNCTION()
	void HandleFeedback(FGameplayTag Tag, const FFeedbackEventContext& Context);
	void FlashStamina();
	void UpdateLowHealthLatch();
	void UpdateVignetteVisuals();
	void RefreshVisibility();
	TWeakObjectPtr<AHeroCharacter> ObservedHero;
	TWeakObjectPtr<UFeedbackSubsystem> Feedback;
	FTSTicker::FDelegateHandle FlashHandle;
	FTSTicker::FDelegateHandle DamageFlashHandle;
	FTimerHandle InitialBindTimer;
	FLinearColor StaminaBaseColor;
	float HealthFraction = 0.f;
	float StaminaTarget = 0.f;
	EHUDLayer CurrentLayer = EHUDLayer::None;
	bool bLowHealthLatched = false;
	bool bDamageFlashActive = false;
	bool bCreatedVignetteInstance = false;
	float DamageVignetteOpacity = 0.f;
	float LowHealthPulseTime = 0.f;
};
