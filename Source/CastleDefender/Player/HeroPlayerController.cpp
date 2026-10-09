#include "Player/HeroPlayerController.h"

#include "Core/GameCheatManager.h"
#include "Core/GameLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "Hero/HeroCharacter.h"
#include "Hero/LockOnComponent.h"
#include "UI/GameHUDWidget.h"

namespace
{
	FString ModeName(EPlayerMode Mode)
	{
		return StaticEnum<EPlayerMode>()->GetNameStringByValue(static_cast<int64>(Mode));
	}
}

AHeroPlayerController::AHeroPlayerController()
{
	CheatClass = UGameCheatManager::StaticClass();
}

void AHeroPlayerController::OnPossess(APawn* InPawn)
{
	DetachLockOnUI();
	Super::OnPossess(InPawn);
	EnsureGameHUD();
	if (const AHeroCharacter* Hero = Cast<AHeroCharacter>(InPawn))
	{
		ObservedLockOn = Hero->GetLockOnComponent();
		ObservedLockOn->OnLockOnTargetChanged.AddDynamic(this, &AHeroPlayerController::HandleLockOnTargetChanged);
		HandleLockOnTargetChanged(ObservedLockOn->GetLockOnTarget());
	}
}

void AHeroPlayerController::HandleLockOnTargetChanged(AActor* Target)
{
	if (!Target)
	{
		if (GameHUD) { GameHUD->SetLockOnMarker(nullptr); }
		if (LockOnMarker) { LockOnMarker->RemoveFromParent(); LockOnMarker = nullptr; }
	}
	else if (IsLocalController() && GetLocalPlayer() && LockOnMarkerClass && !LockOnMarker)
	{
		LockOnMarker = CreateWidget<UUserWidget>(this, LockOnMarkerClass);
		if (LockOnMarker)
		{
			if (GameHUD) { GameHUD->SetLockOnMarker(LockOnMarker); }
			else { LockOnMarker->AddToViewport(); }
		}
	}
}

void AHeroPlayerController::DetachLockOnUI()
{
	if (ObservedLockOn.IsValid()) { ObservedLockOn->OnLockOnTargetChanged.RemoveDynamic(this, &AHeroPlayerController::HandleLockOnTargetChanged); }
	ObservedLockOn.Reset();
	HandleLockOnTargetChanged(nullptr);
}

void AHeroPlayerController::OnUnPossess() { DetachLockOnUI(); Super::OnUnPossess(); }
void AHeroPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DetachLockOnUI();
	if (GameHUD) { GameHUD->RemoveFromParent(); GameHUD = nullptr; }
	Super::EndPlay(EndPlayReason);
}

void AHeroPlayerController::BeginPlay() { Super::BeginPlay(); EnsureGameHUD(); }
void AHeroPlayerController::EnsureGameHUD()
{
	if (GameHUD || !GameHUDClass || !IsLocalController() || !GetLocalPlayer()) { return; }
	GameHUD = CreateWidget<UGameHUDWidget>(this, GameHUDClass);
	if (GameHUD) { GameHUD->AddToViewport(); }
}

void AHeroPlayerController::PushMode(EPlayerMode Mode, FName Reason)
{
	const EPlayerMode OldMode = GetMode();
	ModeStack.Push(Mode, Reason);
	OnModeStackChanged(OldMode, Reason);
}

void AHeroPlayerController::PopMode(FName Reason)
{
	const EPlayerMode OldMode = GetMode();
	if (!ModeStack.Pop(Reason))
	{
		UE_LOG(LogGamePlayer, Warning, TEXT("PopMode: no mode pushed with reason '%s'"), *Reason.ToString());
		return;
	}
	OnModeStackChanged(OldMode, Reason);
}

void AHeroPlayerController::OnModeStackChanged(EPlayerMode OldMode, FName Reason)
{
	const EPlayerMode NewMode = GetMode();
	if (NewMode == OldMode)
	{
		UE_LOG(LogGamePlayer, Log, TEXT("Player mode stays %s (reason '%s', depth %d)"), *ModeName(NewMode), *Reason.ToString(), ModeStack.Num());
		return;
	}
	ApplyMode(NewMode);
	UE_LOG(LogGamePlayer, Log, TEXT("Player mode %s -> %s (reason '%s', depth %d)"), *ModeName(OldMode), *ModeName(NewMode), *Reason.ToString(), ModeStack.Num());
	OnPlayerModeChanged.Broadcast(OldMode, NewMode);
}

void AHeroPlayerController::ApplyMode(EPlayerMode Mode)
{
	UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Input)
	{
		return;
	}

	// This controller is the only writer of mapping contexts (D-19), so a full reset is safe.
	Input->ClearAllMappings();
#if !UE_BUILD_SHIPPING
	if (DebugMappingContext)
	{
		Input->AddMappingContext(DebugMappingContext, 1000);
	}
#endif

	const FPlayerModeInput* Config = ModeInput.Find(Mode);
	TArray<FString> ContextNames;
	if (Config)
	{
		for (const FPlayerModeMappingContext& Entry : Config->MappingContexts)
		{
			if (Entry.MappingContext)
			{
				Input->AddMappingContext(Entry.MappingContext, Entry.Priority);
				ContextNames.Add(Entry.MappingContext->GetName());
			}
		}
	}

	const bool bShowCursor = Config && Config->bShowCursor;
	SetShowMouseCursor(bShowCursor);
	if (bShowCursor)
	{
		SetInputMode(FInputModeGameAndUI().SetHideCursorDuringCapture(false));
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}

	UE_LOG(LogGamePlayer, Log, TEXT("Active mapping contexts for %s: [%s]"), *ModeName(Mode), *FString::Join(ContextNames, TEXT(", ")));
}

void AHeroPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	EnsureGameHUD();
	ApplyMode(GetMode());
}

void AHeroPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

#if !UE_BUILD_SHIPPING
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent); Input && DebugToggleBuildAction)
	{
		Input->BindAction(DebugToggleBuildAction, ETriggerEvent::Started, this, &AHeroPlayerController::DebugToggleBuild);
	}
#endif
}

#if !UE_BUILD_SHIPPING
void AHeroPlayerController::DebugToggleBuild()
{
	static const FName DebugReason(TEXT("Debug"));
	if (GetMode() == EPlayerMode::Build)
	{
		PopMode(DebugReason);
	}
	else
	{
		PushMode(EPlayerMode::Build, DebugReason);
	}
}
#endif
