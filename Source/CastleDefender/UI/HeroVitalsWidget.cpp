#include "UI/HeroVitalsWidget.h"
#include "Components/ProgressBar.h"
#include "Hero/HeroCharacter.h"
#include "Hero/StaminaComponent.h"
#include "Combat/HealthComponent.h"
#include "Feedback/FeedbackSubsystem.h"
#include "Feedback/FeedbackTags.h"
#include "GameFramework/PlayerController.h"
#include "Core/GameTuningSettings.h"

AHeroCharacter* UHeroVitalsWidget::GetObservedHero() const { return ObservedHero.Get(); }

void UHeroVitalsWidget::SetDamageVignette(UUserWidget* InVignette)
{
	DamageVignette = InVignette;
	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::FlashDamageVignette()
{
	if (!ObservedHero.IsValid() || !ObservedHero->GetHealthComponent()->IsAlive())
	{
		return;
	}
	FTSTicker::GetCoreTicker().RemoveTicker(DamageFlashHandle);
	bDamageFlashActive = true;
	UpdateVignetteVisuals();
	const TWeakObjectPtr<UHeroVitalsWidget> WeakThis(this);
	DamageFlashHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
	{
		if (UHeroVitalsWidget* Widget = WeakThis.Get())
		{
			Widget->bDamageFlashActive = false;
			Widget->DamageFlashHandle.Reset();
			Widget->UpdateVignetteVisuals();
		}
		return false;
	}), DamageFlashSeconds);
}

void UHeroVitalsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	StaminaBaseColor = StaminaBar ? StaminaBar->GetFillColorAndOpacity() : FLinearColor::White;
	if (APlayerController* Controller = GetOwningPlayer())
	{
		Controller->OnPossessedPawnChanged.AddUniqueDynamic(this, &UHeroVitalsWidget::HandlePawnChanged);
		BindHero(Cast<AHeroCharacter>(Controller->GetPawn()));
		if (!DamageVignette && DamageVignetteClass)
		{
			DamageVignette = CreateWidget<UUserWidget>(Controller, DamageVignetteClass);
			if (DamageVignette)
			{
				DamageVignette->AddToPlayerScreen(-1);
				bCreatedVignetteInstance = true;
			}
		}
	}
	Feedback = UFeedbackSubsystem::Get(this);
	if (Feedback.IsValid()) { Feedback->OnFeedbackPlayed.AddUniqueDynamic(this, &UHeroVitalsWidget::HandleFeedback); }
	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::BindHero(AHeroCharacter* Hero)
{
	if (GetWorld()) { GetWorld()->GetTimerManager().ClearTimer(InitialBindTimer); }
	if (ObservedHero.IsValid())
	{
		ObservedHero->GetHealthComponent()->OnDamaged.RemoveDynamic(this, &UHeroVitalsWidget::HandleDamaged);
		ObservedHero->GetHealthComponent()->OnHealed.RemoveDynamic(this, &UHeroVitalsWidget::HandleHealed);
		ObservedHero->GetHealthComponent()->OnDeath.RemoveDynamic(this, &UHeroVitalsWidget::HandleDeath);
		ObservedHero->GetStaminaComponent()->OnStaminaChanged.RemoveDynamic(this, &UHeroVitalsWidget::HandleStaminaChanged);
		ObservedHero->GetStaminaComponent()->OnStaminaSpendFailed.RemoveDynamic(this, &UHeroVitalsWidget::HandleStaminaFailed);
	}
	FTSTicker::GetCoreTicker().RemoveTicker(FlashHandle);
	FlashHandle.Reset();
	FTSTicker::GetCoreTicker().RemoveTicker(DamageFlashHandle);
	DamageFlashHandle.Reset();
	if (StaminaBar) { StaminaBar->SetFillColorAndOpacity(StaminaBaseColor); }
	bLowHealthLatched = false;
	bDamageFlashActive = false;
	LowHealthPulseTime = 0.f;
	ObservedHero = Hero;
	HealthFraction = Hero && Hero->GetHealthComponent()->GetMaxHealth() > 0.f
		? Hero->GetHealthComponent()->GetCurrentHealth() / Hero->GetHealthComponent()->GetMaxHealth() : 0.f;
	StaminaTarget = Hero ? Hero->GetStaminaComponent()->GetStaminaFraction() : 0.f;
	if (HealthBar) { HealthBar->SetPercent(HealthFraction); }
	if (StaminaBar) { StaminaBar->SetPercent(StaminaTarget); }
	if (Hero)
	{
		Hero->GetHealthComponent()->OnDamaged.AddDynamic(this, &UHeroVitalsWidget::HandleDamaged);
		Hero->GetHealthComponent()->OnHealed.AddDynamic(this, &UHeroVitalsWidget::HandleHealed);
		Hero->GetHealthComponent()->OnDeath.AddDynamic(this, &UHeroVitalsWidget::HandleDeath);
		Hero->GetStaminaComponent()->OnStaminaChanged.AddDynamic(this, &UHeroVitalsWidget::HandleStaminaChanged);
		Hero->GetStaminaComponent()->OnStaminaSpendFailed.AddDynamic(this, &UHeroVitalsWidget::HandleStaminaFailed);
		if (!Feedback.IsValid())
		{
			Feedback = UFeedbackSubsystem::Get(Hero);
			if (Feedback.IsValid()) { Feedback->OnFeedbackPlayed.AddUniqueDynamic(this, &UHeroVitalsWidget::HandleFeedback); }
		}
		if (!Hero->HasActorBegunPlay())
		{
			// Startup possession precedes Hero initialization. Sync once next frame, not by polling HP.
			const TWeakObjectPtr<AHeroCharacter> PendingHero(Hero);
			InitialBindTimer = GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, PendingHero]()
			{
				if (PendingHero.IsValid() && ObservedHero == PendingHero) { BindHero(PendingHero.Get()); }
			}));
		}
		else
		{
			UpdateLowHealthLatch();
		}
	}
	RefreshVisibility();
	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn) { BindHero(Cast<AHeroCharacter>(NewPawn)); }

void UHeroVitalsWidget::HandleDamaged(const FCombatHit& Hit, float NewHealth)
{
	const float Maximum = ObservedHero.IsValid() ? ObservedHero->GetHealthComponent()->GetMaxHealth() : 0.f;
	HealthFraction = Maximum > 0.f ? FMath::Clamp(NewHealth / Maximum, 0.f, 1.f) : 0.f;
	if (HealthBar) { HealthBar->SetPercent(HealthFraction); }
	UpdateLowHealthLatch();
}

void UHeroVitalsWidget::HandleHealed(float Amount, float NewHealth)
{
	const float Maximum = ObservedHero.IsValid() ? ObservedHero->GetHealthComponent()->GetMaxHealth() : 0.f;
	HealthFraction = Maximum > 0.f ? FMath::Clamp(NewHealth / Maximum, 0.f, 1.f) : 0.f;
	if (HealthBar) { HealthBar->SetPercent(HealthFraction); }
	UpdateLowHealthLatch();
}

void UHeroVitalsWidget::HandleDeath(const FCombatHit& Hit)
{
	HealthFraction = 0.f;
	bLowHealthLatched = false;
	bDamageFlashActive = false;
	FTSTicker::GetCoreTicker().RemoveTicker(DamageFlashHandle);
	DamageFlashHandle.Reset();
	if (HealthBar) { HealthBar->SetPercent(0.f); }
	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::HandleStaminaChanged(float Current, float Maximum)
{
	StaminaTarget = Maximum > 0.f ? FMath::Clamp(Current / Maximum, 0.f, 1.f) : 0.f;
}

void UHeroVitalsWidget::HandleStaminaFailed(float Cost) { FlashStamina(); }

void UHeroVitalsWidget::HandleFeedback(FGameplayTag Tag, const FFeedbackEventContext& Context)
{
	if (Tag == FeedbackTags::Hero_StaminaInsufficient && ObservedHero.IsValid() &&
		(Context.Target.Get() == ObservedHero.Get() || Context.Instigator.Get() == ObservedHero.Get()))
	{
		FlashStamina();
	}
	else if (Tag == FeedbackTags::Hero_Damaged && ObservedHero.IsValid() && Context.Target.Get() == ObservedHero.Get())
	{
		FlashDamageVignette();
	}
}

void UHeroVitalsWidget::FlashStamina()
{
	if (!StaminaBar) { return; }
	FTSTicker::GetCoreTicker().RemoveTicker(FlashHandle);
	StaminaBar->SetFillColorAndOpacity(StaminaFlashColor);
	const TWeakObjectPtr<UHeroVitalsWidget> WeakThis(this);
	// One-shot Core ticker runs in real time; UI flash must not stretch with Tactical Focus (D-20).
	FlashHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
	{
		if (UHeroVitalsWidget* Widget = WeakThis.Get())
		{
			if (Widget->StaminaBar) { Widget->StaminaBar->SetFillColorAndOpacity(Widget->StaminaBaseColor); }
			Widget->FlashHandle.Reset();
		}
		return false;
	}), StaminaFlashSeconds);
}

void UHeroVitalsWidget::UpdateLowHealthLatch()
{
	const UGameTuningSettings* Settings = UGameTuningSettings::Get();
	const float LowThreshold = Settings ? Settings->HeroLowHealthThreshold : 0.30f;
	const float RearmThreshold = Settings ? Settings->HeroLowHealthRearmThreshold : 0.40f;

	if (ObservedHero.IsValid() && ObservedHero->GetHealthComponent()->IsAlive() && HealthFraction > 0.f)
	{
		if (HealthFraction <= LowThreshold)
		{
			if (!bLowHealthLatched)
			{
				bLowHealthLatched = true;
				if (!Feedback.IsValid()) { Feedback = UFeedbackSubsystem::Get(this); }
				if (Feedback.IsValid())
				{
					FFeedbackEventContext Context;
					Context.Target = ObservedHero.Get();
					Context.Instigator = ObservedHero.Get();
					Feedback->Play(FeedbackTags::Hero_LowHealth, Context);
				}
			}
		}
		else if (HealthFraction >= RearmThreshold)
		{
			bLowHealthLatched = false;
		}
	}
	else
	{
		bLowHealthLatched = false;
	}

	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::UpdateVignetteVisuals()
{
	const bool bValidVitals = ObservedHero.IsValid() && ObservedHero->GetHealthComponent()->IsAlive() &&
		CurrentLayer != EHUDLayer::Modal && CurrentLayer != EHUDLayer::CommanderSpirit;
	if (!bValidVitals)
	{
		DamageVignetteOpacity = 0.f;
	}
	else if (bDamageFlashActive)
	{
		DamageVignetteOpacity = 1.f;
	}
	else if (bLowHealthLatched)
	{
		const float Sine = (FMath::Sin(LowHealthPulseTime * LowHealthPulseFrequency * 2.f * UE_PI) + 1.f) * 0.5f;
		DamageVignetteOpacity = FMath::Lerp(0.2f, 0.65f, Sine);
	}
	else
	{
		DamageVignetteOpacity = 0.f;
	}

	if (CurrentLayer == EHUDLayer::TacticalFocus)
	{
		DamageVignetteOpacity *= FocusOpacity;
	}

	if (DamageVignette)
	{
		if (DamageVignetteOpacity > 0.001f)
		{
			DamageVignette->SetVisibility(ESlateVisibility::HitTestInvisible);
			DamageVignette->SetRenderOpacity(DamageVignetteOpacity);
		}
		else
		{
			DamageVignette->SetVisibility(ESlateVisibility::Collapsed);
			DamageVignette->SetRenderOpacity(0.f);
		}
	}
}

void UHeroVitalsWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
	Super::NativeTick(Geometry, DeltaTime);
	// R-UXF-13: only interpolate the cached delegate value; never poll gameplay state each frame.
	if (StaminaBar) { StaminaBar->SetPercent(FMath::FInterpTo(StaminaBar->GetPercent(), StaminaTarget, DeltaTime, StaminaLerpSpeed)); }

	if (bLowHealthLatched)
	{
		LowHealthPulseTime += DeltaTime;
		UpdateVignetteVisuals();
	}
}

void UHeroVitalsWidget::ApplyLayer(EHUDLayer Layer) { CurrentLayer = Layer; RefreshVisibility(); }

void UHeroVitalsWidget::RefreshVisibility()
{
	const bool bHidden = CurrentLayer == EHUDLayer::Modal || CurrentLayer == EHUDLayer::CommanderSpirit || !ObservedHero.IsValid();
	SetVisibility(bHidden ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	SetRenderOpacity(CurrentLayer == EHUDLayer::TacticalFocus ? FocusOpacity : 1.f);
	UpdateVignetteVisuals();
}

void UHeroVitalsWidget::NativeDestruct()
{
	if (APlayerController* Controller = GetOwningPlayer()) { Controller->OnPossessedPawnChanged.RemoveDynamic(this, &UHeroVitalsWidget::HandlePawnChanged); }
	if (Feedback.IsValid()) { Feedback->OnFeedbackPlayed.RemoveDynamic(this, &UHeroVitalsWidget::HandleFeedback); }
	Feedback.Reset();
	FTSTicker::GetCoreTicker().RemoveTicker(FlashHandle);
	FlashHandle.Reset();
	FTSTicker::GetCoreTicker().RemoveTicker(DamageFlashHandle);
	DamageFlashHandle.Reset();
	if (bCreatedVignetteInstance && DamageVignette)
	{
		DamageVignette->RemoveFromParent();
	}
	DamageVignette = nullptr;
	BindHero(nullptr);
	Super::NativeDestruct();
}
