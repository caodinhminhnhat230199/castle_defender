#include "Enemy/EnemyCharacter.h"
#include "AIController.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/GameLog.h"
#include "Feedback/FeedbackTags.h"
#include "Feedback/FeedbackSubsystem.h"

AEnemyCharacter::AEnemyCharacter()
{
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	CombatState = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatState"));
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::InitFromSpawn(UEnemyArchetypeDefinition* Definition, const FEnemySpawnParams& /*Params*/)
{
	if (HasActorBegunPlay() || bRemovalReported)
	{
		UE_LOG(LogGameAI, Error, TEXT("%s: InitFromSpawn must be called before BeginPlay."), *GetName());
		return;
	}
	Archetype = Definition;
	if (Archetype) { RuntimeParams = Archetype->MakeRuntimeParams(); }
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	FString Error;
	if (!Archetype || !Archetype->ValidateDefinition(Error))
	{
		UE_LOG(LogGameAI, Error, TEXT("%s: invalid enemy archetype: %s"), *GetName(), Archetype ? *Error : TEXT("missing definition"));
		Despawn();
		return;
	}
	RuntimeParams = Archetype->MakeRuntimeParams();
	Health->InitializeHealth(RuntimeParams.MaxHealth, RuntimeParams.BaseArmor);
	CombatState->Init(RuntimeParams.CombatState);
	GetCharacterMovement()->MaxWalkSpeed = RuntimeParams.WalkSpeed;
	GetCharacterMovement()->RotationRate.Yaw = RuntimeParams.WindUpTurnRate;
	Health->OnDeath.AddDynamic(this, &AEnemyCharacter::HandleDeath);
}

void AEnemyCharacter::StopActing()
{
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		AI->StopMovement();
		AI->ClearFocus(EAIFocusPriority::Gameplay);
	}
	StopAnimMontage();
	if (UMeleeTraceComponent* Trace = FindComponentByClass<UMeleeTraceComponent>()) { Trace->EndHitWindow(); }
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	// Meshes can have a separate collision profile; a corpse must not block the hero through either body.
	GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void AEnemyCharacter::ReportRemoval(EEnemyRemovedReason Reason)
{
	if (bRemovalReported) { return; }
	bRemovalReported = true; // Set before observers: callbacks may destroy/despawn this actor reentrantly.
	OnEnemyRemoved.Broadcast(this, Reason);
}

void AEnemyCharacter::HandleDeath(const FCombatHit& KillingHit)
{
	if (bRemovalReported) { return; }
	StopActing();
	ReportRemoval(EEnemyRemovedReason::Killed);
	if (IsActorBeingDestroyed()) { return; }
	OnDeathPresentation();
	if (IsActorBeingDestroyed()) { return; }
	if (UFeedbackSubsystem* Feedback = UFeedbackSubsystem::Get(this))
	{
		FFeedbackEventContext Context;
		Context.Instigator = KillingHit.Instigator;
		Context.Target = this;
		Context.Location = GetActorLocation();
		Feedback->Play(FeedbackTags::Enemy_Death, Context);
	}
	// UE treats lifespan 0 as "cancel expiry", so zero-delay tuning must destroy explicitly.
	if (RuntimeParams.DespawnDelay <= 0.f) { Destroy(); }
	else { SetLifeSpan(RuntimeParams.DespawnDelay); }
}

void AEnemyCharacter::Despawn()
{
	StopActing();
	ReportRemoval(EEnemyRemovedReason::Despawned);
	if (!IsActorBeingDestroyed()) { Destroy(); }
}

void AEnemyCharacter::FellOutOfWorld(const UDamageType& DamageType)
{
	StopActing();
	ReportRemoval(EEnemyRemovedReason::OutOfWorld);
	Super::FellOutOfWorld(DamageType);
}

void AEnemyCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
	ReportRemoval(EEnemyRemovedReason::Despawned);
	Super::EndPlay(Reason);
}
