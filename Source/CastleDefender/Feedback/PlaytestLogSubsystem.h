#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Feedback/FeedbackTypes.h"
#include "Feedback/PlaytestSummary.h"
#include "Hero/HeroCombatTypes.h"
#include "Combat/CombatTypes.h"
#include "PlaytestLogSubsystem.generated.h"

class AHeroCharacter;
class APlayerController;
class UFeedbackSubsystem;

/** Development-only local recording. World lifetime; observers never mutate gameplay. */
UCLASS()
class CASTLEDEFENDER_API UPlaytestLogSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnWorldBeginPlay(UWorld& World) override;
	virtual void Deinitialize() override;
	void LogEvent(FName Event, const TMap<FName, float>& Numbers, const TMap<FName, FString>& Strings);
	/** Reserved envelope keys ev/t/rt are retained. Numeric fields win duplicate payload names. */
	UFUNCTION(BlueprintCallable, Category = "Playtest", meta = (WorldContext = "WorldContextObject"))
	static void LogEventForWorld(const UObject* WorldContextObject, FName Event, const TMap<FName, float>& Numbers, const TMap<FName, FString>& Strings);
	UFUNCTION(BlueprintPure, Category = "Playtest")
	FString GetLogPath() const;
protected:
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;
private:
	UFUNCTION()
	void HandleFeedback(FGameplayTag Tag, const FFeedbackEventContext& Context);
	UFUNCTION()
	void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION()
	void HandleAction(EHeroActionState OldState, EHeroActionState NewState);
	UFUNCTION()
	void HandleDamaged(const FCombatHit& Hit, float NewHealth);
#if !UE_BUILD_SHIPPING
#if WITH_DEV_AUTOMATION_TESTS
	friend class FPlaytestSummarySpec;
#endif
	void BindController(APlayerController* Controller);
	void BindHero(AHeroCharacter* Hero);
	bool IsRecording() const;
	bool OpenFile();
	void WriteLine(FName Event, TFunctionRef<void(FPlaytestJsonWriter&)> WritePayload);
	void FailWrite();
	FPlaytestSummary Summary;
	TUniquePtr<FArchive> Writer;
	FString LogPath;
	TWeakObjectPtr<UFeedbackSubsystem> Feedback;
	TWeakObjectPtr<APlayerController> ObservedController;
	TWeakObjectPtr<AHeroCharacter> ObservedHero;
	FDelegateHandle SpawnHandle;
	double StartGameTime = 0.0;
	float LastHealth = 0.f;
	bool bSessionActive = false;
	bool bWriteFailed = false;
#endif
};
