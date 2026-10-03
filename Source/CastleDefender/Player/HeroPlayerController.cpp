#include "Player/HeroPlayerController.h"

#include "Core/GameLog.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"

namespace
{
	FString ModeName(EPlayerMode Mode)
	{
		return StaticEnum<EPlayerMode>()->GetNameStringByValue(static_cast<int64>(Mode));
	}
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
