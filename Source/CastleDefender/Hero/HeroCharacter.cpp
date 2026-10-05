#include "Hero/HeroCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"
#include "Combat/MeleeTraceComponent.h"
#include "Core/GameLog.h"
#include "Core/GameTags.h"
#include "Core/GameDebug.h"
#include "DrawDebugHelpers.h"

AHeroCharacter::AHeroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	TeamId = FGenericTeamId(Team_Player);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Matches FHeroMovementData::bFaceCameraDirection's default; ApplyTuning applies the definition's value.
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 450.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 10.f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	CombatState = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatState"));
	CombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("CombatComponent"));
	StaminaComponent = CreateDefaultSubobject<UStaminaComponent>(TEXT("StaminaComponent"));
	MeleeTraceComponent = CreateDefaultSubobject<UMeleeTraceComponent>(TEXT("MeleeTraceComponent"));

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh());
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->ComponentTags.Add(TEXT("Weapon"));
}

void AHeroCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Socket lookup on a mesh without an asset logs a warning, so only attach once the asset has the socket.
	if (GetMesh()->GetSkeletalMeshAsset() && GetMesh()->DoesSocketExist(WeaponSocketName))
	{
		WeaponMesh->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WeaponSocketName);
	}
}

void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyTuning();

	if (WeaponMesh->GetStaticMesh())
	{
		MeleeTraceComponent->SetTraceMesh(WeaponMesh);
	}

	if (Health)
	{
		Health->OnDeath.AddDynamic(this, &AHeroCharacter::HandleDeath);
	}
}

void AHeroCharacter::Tick(float DeltaSeconds)
{
	// Tick: retain ACharacter's root-motion processing; build a development readout only while its CVar is enabled.
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG && !UE_BUILD_SHIPPING
	if (GameDebug::CVarCombat.GetValueOnGameThread() > 0)
	{
		const FVector Top = GetActorLocation() + FVector(0.f, 0.f, 110.f);
		const float CurHP = Health ? Health->GetCurrentHealth() : 0.f;
		const float MaxHP = Health ? Health->GetMaxHealth() : 0.f;
		const float CurStam = StaminaComponent ? StaminaComponent->GetCurrentStamina() : 0.f;
		const float MaxStam = StaminaComponent ? StaminaComponent->GetMaxStamina() : 0.f;
		const bool bTickOn = StaminaComponent ? StaminaComponent->IsComponentTickEnabled() : false;
		DrawDebugString(GetWorld(), Top,
			FString::Printf(TEXT("HP: %.0f/%.0f | Stamina: %.0f/%.0f (Tick:%s)\n%s"),
				CurHP, MaxHP, CurStam, MaxStam, bTickOn ? TEXT("ON") : TEXT("OFF"), *CombatComponent->GetCombatDebugString()),
			nullptr, FColor::Yellow, 0.f);
	}
#endif
}

void AHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Move);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHeroCharacter::StopMove);
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AHeroCharacter::StopMove);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHeroCharacter::Look);
		}
		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &AHeroCharacter::OnSprintStarted);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHeroCharacter::OnSprintCompleted);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &AHeroCharacter::OnSprintCompleted);
		}
		if (LightAttackAction)
		{
			EnhancedInput->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &AHeroCharacter::OnLightAttack);
		}
		if (HeavyAttackAction)
		{
			EnhancedInput->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &AHeroCharacter::OnHeavyAttack);
		}
		if (DodgeAction)
		{
			EnhancedInput->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AHeroCharacter::OnDodge);
		}
		if (BlockAction)
		{
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Started, this, &AHeroCharacter::OnBlockStarted);
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Completed, this, &AHeroCharacter::OnBlockCompleted);
			EnhancedInput->BindAction(BlockAction, ETriggerEvent::Canceled, this, &AHeroCharacter::OnBlockCompleted);
		}
		if (ParryAction)
		{
			EnhancedInput->BindAction(ParryAction, ETriggerEvent::Triggered, this, &AHeroCharacter::OnParry);
		}
	}
}

void AHeroCharacter::ApplyTuning()
{
	if (HeroClassDefinition)
	{
		GetCharacterMovement()->RotationRate = FRotator(0.f, HeroClassDefinition->Movement.RotationRateYaw, 0.f);
		GetCharacterMovement()->bOrientRotationToMovement = !HeroClassDefinition->Movement.bFaceCameraDirection;
		GetCharacterMovement()->bUseControllerDesiredRotation = HeroClassDefinition->Movement.bFaceCameraDirection;

		if (CameraBoom)
		{
			CameraBoom->TargetArmLength = HeroClassDefinition->Camera.TargetArmLength;
			CameraBoom->SocketOffset = HeroClassDefinition->Camera.SocketOffset;
			CameraBoom->bEnableCameraLag = HeroClassDefinition->Camera.bEnableCameraLag;
			CameraBoom->CameraLagSpeed = HeroClassDefinition->Camera.CameraLagSpeed;
		}

		if (Health)
		{
			Health->InitializeHealth(HeroClassDefinition->MaxHealth, 0.f);
		}

		if (StaminaComponent)
		{
			StaminaComponent->InitializeFromConfig(HeroClassDefinition->Stamina);
		}
	}

	UpdateMaxWalkSpeed();
}

void AHeroCharacter::StartSprint()
{
	if (Health && Health->IsDead()) { return; }
	bIsSprinting = true;
	if (StaminaComponent && HeroClassDefinition && HeroClassDefinition->Stamina.SprintDrainPerSecond > 0.f)
	{
		StaminaComponent->SetSprintDraining(true);
	}
	UpdateMaxWalkSpeed();
}

void AHeroCharacter::StopSprint()
{
	bIsSprinting = false;
	if (StaminaComponent)
	{
		StaminaComponent->SetSprintDraining(false);
	}
	UpdateMaxWalkSpeed();
}

void AHeroCharacter::UpdateMaxWalkSpeed()
{
	const float JogSpeed = HeroClassDefinition ? HeroClassDefinition->Movement.JogSpeed : 450.f;
	const float SprintSpeed = HeroClassDefinition ? HeroClassDefinition->Movement.SprintSpeed : 700.f;
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : JogSpeed;
}

void AHeroCharacter::Move(const FInputActionValue& Value)
{
	if (Health && Health->IsDead()) { MovementInputAxes = FVector2D::ZeroVector; return; }
	const FVector2D MovementVector = Value.Get<FVector2D>();
	MovementInputAxes = MovementVector;

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHeroCharacter::StopMove(const FInputActionValue& Value)
{
	MovementInputAxes = FVector2D::ZeroVector;
}

FVector AHeroCharacter::GetMovementInputWorldDirection() const
{
	const FRotator CameraYaw(0.f, Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw, 0.f);
	return (FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::X) * MovementInputAxes.Y
		+ FRotationMatrix(CameraYaw).GetUnitAxis(EAxis::Y) * MovementInputAxes.X).GetSafeNormal2D();
}

bool AHeroCharacter::IsFacingCameraDirection() const
{
	return GetCharacterMovement()->bUseControllerDesiredRotation;
}

void AHeroCharacter::Look(const FInputActionValue& Value)
{
	if (Health && Health->IsDead()) { return; }
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHeroCharacter::OnSprintStarted(const FInputActionValue& Value)
{
	StartSprint();
}

void AHeroCharacter::OnSprintCompleted(const FInputActionValue& Value)
{
	StopSprint();
}

void AHeroCharacter::OnLightAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::Light);
	}
}

void AHeroCharacter::OnHeavyAttack(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::Heavy);
	}
}

void AHeroCharacter::OnDodge(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::Dodge);
	}
}

void AHeroCharacter::OnBlockStarted(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::BlockStart);
	}
}

void AHeroCharacter::OnBlockCompleted(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::BlockEnd);
	}
}

void AHeroCharacter::OnParry(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		CombatComponent->RequestAction(EHeroAction::Parry);
	}
}

void AHeroCharacter::HandleDeath(const FCombatHit& KillingHit)
{
	if (bDeathHandled) { return; }
	bDeathHandled = true;
	StopSprint();
	MovementInputAxes = FVector2D::ZeroVector;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	CombatComponent->HandleOwnerDeath(KillingHit);
	OnHeroDeath.Broadcast(KillingHit);
	OnFeedbackRequested.Broadcast(GameTags::Feedback_Hero_Death, KillingHit);
	OnDeathPresentation(KillingHit);
}
