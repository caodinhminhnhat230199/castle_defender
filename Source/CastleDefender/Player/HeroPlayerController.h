#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Player/PlayerMode.h"
#include "HeroPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UUserWidget;
class ULockOnComponent;
class UGameHUDWidget;
class UCommandComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerModeChangedSignature, EPlayerMode, OldMode, EPlayerMode, NewMode);

USTRUCT(BlueprintType)
struct FPlayerModeMappingContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> MappingContext;

	/** Higher wins when two active contexts map the same key. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	int32 Priority = 0;
};

/** What a mode turns on: its mapping contexts and the UI input mode. */
USTRUCT(BlueprintType)
struct FPlayerModeInput
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TArray<FPlayerModeMappingContext> MappingContexts;

	/** Game and UI input with a visible cursor; otherwise game-only input. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	bool bShowCursor = false;
};

/**
 * Single owner of player mode (D-19). Features push/pop modes; they never set mapping
 * contexts or UI input mode themselves. HUD and overlays listen to OnPlayerModeChanged.
 */
UCLASS()
class CASTLEDEFENDER_API AHeroPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AHeroPlayerController();
	UFUNCTION(BlueprintPure, Category = "Squads")
	UCommandComponent* GetCommandComponent() const { return Command; }
	UFUNCTION(BlueprintPure, Category = "HUD")
	UGameHUDWidget* GetGameHUD() const { return GameHUD; }

	UFUNCTION(BlueprintCallable, Category = "Player Mode")
	void PushMode(EPlayerMode Mode, FName Reason);

	/** Removes the latest entry pushed with Reason, even if it is not on top. */
	UFUNCTION(BlueprintCallable, Category = "Player Mode")
	void PopMode(FName Reason);

	UFUNCTION(BlueprintPure, Category = "Player Mode")
	EPlayerMode GetMode() const { return ModeStack.Top(); }

	UPROPERTY(BlueprintAssignable, Category = "Player Mode")
	FPlayerModeChangedSignature OnPlayerModeChanged;

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UGameHUDWidget> GameHUDClass;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/** CMB presentation hosted by the HUD's LockOn slot. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> LockOnMarkerClass;
	virtual void ReceivedPlayer() override;
	virtual void SetupInputComponent() override;

	/** Contexts per mode; only the top mode's set is active. Modes without an entry have no contexts. */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode")
	TMap<EPlayerMode, FPlayerModeInput> ModeInput;

	/** Always-on debug context (not used in Shipping). */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Debug")
	TObjectPtr<UInputMappingContext> DebugMappingContext;

	/** Debug action that toggles Build mode (AC-FND-05). */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Debug")
	TObjectPtr<UInputAction> DebugToggleBuildAction;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squads", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCommandComponent> Command;
	void EnsureGameHUD();
	UPROPERTY(Transient)
	TObjectPtr<UGameHUDWidget> GameHUD;
	UFUNCTION()
	void HandleLockOnTargetChanged(AActor* Target);
	void DetachLockOnUI();
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LockOnMarker;
	TWeakObjectPtr<ULockOnComponent> ObservedLockOn;
	void OnModeStackChanged(EPlayerMode OldMode, FName Reason);
	void ApplyMode(EPlayerMode Mode);

#if !UE_BUILD_SHIPPING
	void DebugToggleBuild();
#endif

	FPlayerModeStack ModeStack;
};
