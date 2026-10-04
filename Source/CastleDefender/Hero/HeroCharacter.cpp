#include "Hero/HeroCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Hero/HeroCombatComponent.h"
#include "Hero/StaminaComponent.h"
#include "Core/GameLog.h"
#include "Core/GameDebug.h"
#include "DrawDebugHelpers.h"

AHeroCharacter::AHeroCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	TeamId = FGenericTeamId(Team_Player);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
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
}

void AHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyTuning();

	if (Health)
	{
		Health->OnDeath.AddDynamic(this, &AHeroCharacter::HandleDeath);
	}
}

void AHeroCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if ENABLE_DRAW_DEBUG
	if (GameDebug::CVarCombat.GetValueOnGameThread() > 0)
	{
		const FVector Top = GetActorLocation() + FVector(0.f, 0.f, 110.f);
		const float CurHP = Health ? Health->GetCurrentHealth() : 0.f;
		const float MaxHP = Health ? Health->GetMaxHealth() : 0.f;
		const float CurStam = StaminaComponent ? StaminaComponent->GetCurrentStamina() : 0.f;
		const float MaxStam = StaminaComponent ? StaminaComponent->GetMaxStamina() : 0.f;
		const bool bTickOn = StaminaComponent ? StaminaComponent->IsComponentTickEnabled() : false;
		DrawDebugString(GetWorld(), Top,
			FString::Printf(TEXT("HP: %.0f/%.0f | Stamina: %.0f/%.0f (Tick:%s)"),
				CurHP, MaxHP, CurStam, MaxStam, bTickOn ? TEXT("ON") : TEXT("OFF")),
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
	const FVector2D MovementVector = Value.Get<FVector2D>();

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

void AHeroCharacter::Look(const FInputActionValue& Value)
{
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
	StopSprint();
	OnHeroDeath.Broadcast(KillingHit);
}

