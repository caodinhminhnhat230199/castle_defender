#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "Combat/CombatTypes.h"
#include "Enemy/EnemyArchetypeDefinition.h"
#include "EnemyCharacter.generated.h"

class UHealthComponent;
class UCombatStateComponent;
class UEnemyBrainComponent;
class UMeleeTraceComponent;

UENUM(BlueprintType)
enum class EEnemyRemovedReason : uint8 { Killed, Despawned, OutOfWorld };

/** P0 enemies have no route. P1/P2 add their owned goal/lane fields when those providers land. */
USTRUCT(BlueprintType)
struct CASTLEDEFENDER_API FEnemySpawnParams
{
	GENERATED_BODY()
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEnemyRemovedSignature, AEnemyCharacter*, Enemy, EEnemyRemovedReason, Reason);

/** Enemy body and lifecycle; decisions live in UEnemyBrainComponent, attacks arrive with T-ENM-03. */
UCLASS()
class CASTLEDEFENDER_API AEnemyCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	AEnemyCharacter();
	virtual FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId(Team_Enemy); }
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void FellOutOfWorld(const UDamageType& DamageType) override;

	/** Set before FinishSpawning, or before BeginPlay for a level-placed enemy. */
	void InitFromSpawn(UEnemyArchetypeDefinition* Definition, const FEnemySpawnParams& Params);
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void Despawn();
	UFUNCTION(BlueprintPure, Category = "Enemy")
	UHealthComponent* GetHealthComponent() const { return Health; }
	UFUNCTION(BlueprintPure, Category = "Enemy")
	UCombatStateComponent* GetCombatStateComponent() const { return CombatState; }
	UFUNCTION(BlueprintPure, Category = "Enemy")
	UEnemyBrainComponent* GetBrainComponent() const { return Brain; }
	UMeleeTraceComponent* GetMeleeTraceComponent() const { return MeleeTrace; }
	const FEnemyRuntimeParams& GetRuntimeParams() const { return RuntimeParams; }
	bool HasReportedRemoval() const { return bRemovalReported; }
	UFUNCTION(BlueprintPure, Category = "Enemy")
	UEnemyArchetypeDefinition* GetArchetype() const { return Archetype; }
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FEnemyRemovedSignature OnEnemyRemoved;
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Presentation")
	void OnDeathPresentation();
	/** R-ENM-08: every damaging hit while alive. Presentation only: it never changes the enemy's action. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Presentation")
	void OnHitReactPresentation(const FCombatHit& Hit);
	/** R-ENM-07: true when Staggered starts, false when it ends. The brain owns the state change. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy|Presentation")
	void OnStaggerPresentation(bool bStaggered);

protected:
	/** Set per instance for placed enemies, on the Blueprint spawn node (ExposeOnSpawn), or by InitFromSpawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy", meta = (ExposeOnSpawn = true))
	TObjectPtr<UEnemyArchetypeDefinition> Archetype;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UHealthComponent> Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UCombatStateComponent> CombatState;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<UEnemyBrainComponent> Brain;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UMeleeTraceComponent> MeleeTrace;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Enemy")
	FEnemyRuntimeParams RuntimeParams;

private:
	UFUNCTION()
	void HandleDeath(const FCombatHit& KillingHit);
	UFUNCTION()
	void HandleDamaged(const FCombatHit& Hit, float NewHealth);
	void StopActing();
	void ReportRemoval(EEnemyRemovedReason Reason);
	bool bRemovalReported = false;
};
