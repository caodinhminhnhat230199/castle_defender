#include "Combat/CombatStateComponent.h"

#include "Combat/HealthComponent.h"
#include "Core/GameDebug.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Debug/DebugDrawService.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Feedback/FeedbackSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "UObject/UObjectIterator.h"
#include "VisualLogger/VisualLogger.h"

namespace
{
#if !UE_BUILD_SHIPPING
	/** game.debug.CombatStates: poise and states above every unit in the viewed world (debug only, per frame by nature). */
	void DrawCombatStates(UCanvas* Canvas, APlayerController* Viewer)
	{
		if (GameDebug::CVarCombatStates.GetValueOnGameThread() == 0 || !Canvas || !Viewer)
		{
			return;
		}
		const UWorld* World = Viewer->GetWorld();
		for (TObjectIterator<UCombatStateComponent> It; It; ++It)
		{
			const AActor* Owner = It->GetOwner();
			if (!Owner || It->GetWorld() != World)
			{
				continue;
			}
			const FVector Screen = Canvas->Project(Owner->GetActorLocation() + FVector(0.f, 0.f, 160.f));
			if (Screen.Z <= 0.f)
			{
				continue;
			}
			FString Text = FString::Printf(TEXT("Poise %.0f / %.0f"), It->GetCurrentPoise(), It->GetMaxPoise());
			for (const FActiveCombatState& State : It->GetStates())
			{
				Text += FString::Printf(TEXT("\n%s %.1fs"), *State.StateTag.ToString(), It->GetStateRemaining(State.StateTag));
			}
			Canvas->SetDrawColor(FColor::Yellow);
			Canvas->DrawText(GEngine->GetSmallFont(), Text, Screen.X, Screen.Y);
		}
	}
#endif

	/** DT_CombatStatePresentation row for a state; warns once per tag when missing (state still works). */
	const FCombatStatePresentationRow* FindPresentationRow(FGameplayTag State)
	{
		const UDataTable* Table = UGameTuningSettings::Get()->CombatStatePresentationTable.LoadSynchronous();
		const FCombatStatePresentationRow* Row = Table
			? Table->FindRow<FCombatStatePresentationRow>(State.GetTagName(), TEXT("CombatStatePresentation"), false) : nullptr;
		if (!Row)
		{
			static TSet<FGameplayTag> Warned;
			bool bAlreadyWarned = false;
			Warned.Add(State, &bAlreadyWarned);
			if (!bAlreadyWarned)
			{
				UE_LOG(LogGameCombat, Warning, TEXT("No DT_CombatStatePresentation row for %s; no state feedback."), *State.ToString());
			}
		}
		return Row;
	}
}

UCombatStateComponent::UCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatStateComponent::Init(const FCombatStateConfig& Config)
{
	ClearAllStates();
	Model.Init(Config);
}

void UCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();
	if (UHealthComponent* Health = GetOwner()->FindComponentByClass<UHealthComponent>())
	{
		Health->OnDeath.AddDynamic(this, &UCombatStateComponent::HandleOwnerDeath);
	}
#if !UE_BUILD_SHIPPING
	static FDelegateHandle DebugDrawHandle = UDebugDrawService::Register(TEXT("Game"), FDebugDrawDelegate::CreateStatic(&DrawCombatStates));
#endif
}

void UCombatStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ExpiryTimer);
	}
	Super::EndPlay(EndPlayReason);
}

double UCombatStateComponent::Now() const
{
	// Game time: global dilation (Tactical Focus) slows states with the world (technical plan §5).
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.0;
}

bool UCombatStateComponent::IsOwnerDead() const
{
	const UHealthComponent* Health = GetOwner() ? GetOwner()->FindComponentByClass<UHealthComponent>() : nullptr;
	return Health && Health->IsDead();
}

bool UCombatStateComponent::ApplyPoiseDamage(float Amount, AActor* Instigator)
{
	if (IsOwnerDead() || !Model.ApplyPoiseDamage(Amount, Now()))
	{
		return false;
	}
	AddState(GameTags::State_Combat_Staggered, Model.GetConfig().StaggerDuration, Instigator);
	return HasState(GameTags::State_Combat_Staggered);
}

void UCombatStateComponent::ApplyState(FGameplayTag State, float Duration, AActor* Instigator)
{
	AddState(State, Duration, Instigator);
}

void UCombatStateComponent::AddState(FGameplayTag State, float Duration, AActor* Instigator)
{
	if (!State.IsValid() || IsOwnerDead())
	{
		return;
	}
	if (Duration <= 0.f)
	{
		const float* Default = UGameTuningSettings::Get()->StateDefaultDurations.Find(State);
		Duration = Default ? *Default : 0.f;
	}
	if (Duration <= 0.f)
	{
		UE_LOG(LogGameCombat, Warning, TEXT("%s: %s has no duration (hit, unit or Game Tuning default); not applied."),
			*GetNameSafe(GetOwner()), *State.ToString());
		return;
	}

	if (Model.ApplyState(State, Duration, Instigator, Now()) == FCombatStateModel::EApplyResult::Refreshed)
	{
		RescheduleExpiry();
		return;
	}
	UE_VLOG(GetOwner(), LogGameCombat, Log, TEXT("State +%s %.2fs by %s"), *State.ToString(), Duration, *GetNameSafe(Instigator));
	RescheduleExpiry();
	OnStateAdded.Broadcast(State, Instigator);
	PlayPresentation(State, true, Instigator);
}

void UCombatStateComponent::RemoveState(FGameplayTag State)
{
	if (Model.RemoveState(State))
	{
		HandleRemoved({ State });
	}
}

void UCombatStateComponent::ClearAllStates()
{
	HandleRemoved(Model.ClearAll());
}

void UCombatStateComponent::HandleRemoved(const TArray<FGameplayTag>& Removed)
{
	RescheduleExpiry();
	for (const FGameplayTag& State : Removed)
	{
		UE_VLOG(GetOwner(), LogGameCombat, Log, TEXT("State -%s"), *State.ToString());
		OnStateRemoved.Broadcast(State);
		PlayPresentation(State, false, nullptr);
	}
}

void UCombatStateComponent::RescheduleExpiry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const TOptional<double> Next = Model.NextExpiry();
	if (!Next)
	{
		World->GetTimerManager().ClearTimer(ExpiryTimer);
		return;
	}
	// A rate <= 0 would clear the timer, so an already-due expiry fires on the next timer update.
	const float Delay = FMath::Max(static_cast<float>(*Next - Now()), UE_KINDA_SMALL_NUMBER);
	World->GetTimerManager().SetTimer(ExpiryTimer, this, &UCombatStateComponent::HandleExpiryTimer, Delay, false);
}

void UCombatStateComponent::HandleExpiryTimer()
{
	HandleRemoved(Model.RemoveExpired(Now()));
}

void UCombatStateComponent::HandleOwnerDeath(const FCombatHit& KillingHit)
{
	ClearAllStates();
}

float UCombatStateComponent::GetStateRemaining(FGameplayTag State) const
{
	const FActiveCombatState* Active = Model.FindState(State);
	return Active ? static_cast<float>(FMath::Max(0.0, Active->ExpiryTime - Now())) : 0.f;
}

float UCombatStateComponent::GetCurrentPoise() const
{
	return Model.GetPoise(Now());
}

AActor* UCombatStateComponent::GetStateInstigator(FGameplayTag State) const
{
	const FActiveCombatState* Active = Model.FindState(State);
	return Active ? Active->Instigator.Get() : nullptr;
}

void UCombatStateComponent::SetPoise(float Value)
{
	Model.SetPoise(Value, Now());
}

FGameplayTagContainer UCombatStateComponent::GetActiveStates() const
{
	FGameplayTagContainer Tags;
	for (const FActiveCombatState& State : Model.GetStates())
	{
		Tags.AddTag(State.StateTag);
	}
	return Tags;
}

void UCombatStateComponent::PlayPresentation(FGameplayTag State, bool bApplied, AActor* Instigator) const
{
	UFeedbackSubsystem* Feedback = UFeedbackSubsystem::Get(this);
	const FCombatStatePresentationRow* Row = Feedback ? FindPresentationRow(State) : nullptr;
	const FGameplayTag FeedbackTag = Row ? (bApplied ? Row->AppliedFeedback : Row->RemovedFeedback) : FGameplayTag();
	if (!FeedbackTag.IsValid())
	{
		return;
	}
	FFeedbackEventContext Context;
	Context.Instigator = Instigator;
	Context.Target = GetOwner();
	Feedback->Play(FeedbackTag, Context);
}

bool UCombatStateComponent::HasPendingExpiry() const
{
	const UWorld* World = GetWorld();
	return World && World->GetTimerManager().IsTimerActive(ExpiryTimer);
}
