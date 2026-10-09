#include "Army/SoldierCharacter.h"
#include "Army/Squad.h"
#include "Army/SquadDefinition.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Core/GameLog.h"

ASoldierCharacter::ASoldierCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	Health = CreateDefaultSubobject<UHealthComponent>(TEXT("Health"));
	CombatState = CreateDefaultSubobject<UCombatStateComponent>(TEXT("CombatState"));
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void ASoldierCharacter::InitFromSquad(ASquad* InSquad, int32 InSlotIndex, USquadDefinition* InDefinition)
{
	if (HasActorBegunPlay()) { UE_LOG(LogGameArmy, Error, TEXT("Soldier initialization must precede BeginPlay.")); return; }
	Squad = InSquad;
	SlotIndex = InSlotIndex;
	Definition = InDefinition;
	TeamId = Team_Player;
	if (Definition)
	{
		// Deferred spawn initializes stats before component/Blueprint BeginPlay observes the soldier.
		Health->InitializeHealth(Definition->MaxHealth, Definition->BaseArmor);
		CombatState->Init(Definition->CombatState);
		GetCharacterMovement()->MaxWalkSpeed = Definition->MoveSpeed;
	}
}

void ASoldierCharacter::BeginPlay()
{
	Super::BeginPlay();
	FString Error;
	if (!Squad.IsValid() || !Definition || !Definition->ValidateDefinition(Error))
	{
		UE_LOG(LogGameArmy, Error, TEXT("%s: invalid soldier initialization: %s"), *GetName(), *Error);
		Destroy();
		return;
	}
}

void ASoldierCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer.Reset();
	if (Definition && Definition->SquadTypeTag.IsValid()) { TagContainer.AddTag(Definition->SquadTypeTag); }
}

ASquad* ASoldierCharacter::GetSquad() const { return Squad.Get(); }
