#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Army/Squad.h"
#include "Army/SquadDefinition.h"
#include "Army/SoldierCharacter.h"
#include "Combat/HealthComponent.h"
#include "Combat/CombatStateComponent.h"
#include "Core/GameTags.h"
#include "Core/GameTuningSettings.h"
#include "Player/HeroPlayerController.h"
#include "Player/CommandComponent.h"
#include "Tests/EnemyTestFixture.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/AssetManager.h"
#include "Misc/DataValidation.h"
#include <limits>

namespace SquadSpawnSpecPrivate
{
	USquadDefinition* MakeDefinition(UObject* Outer, int32 Count = 4)
	{
		USquadDefinition* Definition = NewObject<USquadDefinition>(Outer);
		Definition->DisplayName = FText::FromString(TEXT("Squad fixture"));
		Definition->SquadTypeTag = GameTags::Unit_Squad_Infantry;
		Definition->SoldierClass = ASoldierCharacter::StaticClass();
		Definition->SoldierCount = Count;
		Definition->FormationColumns = 2;
		Definition->MaxHealth = 125.f;
		Definition->BaseArmor = 0.25f;
		Definition->CombatState.MaxPoise = 60.f;
		return Definition;
	}

	struct FSpawnFixture : FEnemyTestWorld
	{
		AHeroPlayerController* Controller = World->SpawnActor<AHeroPlayerController>();
		FSpawnFixture() { World->InitializeActorsForPlay(FURL()); Controller->DispatchBeginPlay(); }
		ASquad* Spawn(USquadDefinition* Definition, const FTransform& Transform = FTransform::Identity)
		{
			ASquad* Squad = World->SpawnActorDeferred<ASquad>(ASquad::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			Squad->Definition = Definition;
			Squad->FinishSpawning(Transform);
			if (!Squad->HasActorBegunPlay()) { Squad->DispatchBeginPlay(); }
			return Squad;
		}
	};
}

BEGIN_DEFINE_SPEC(FSquadSpawnSpec, "CastleDefender.Army.Spawn", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FSquadSpawnSpec)

void FSquadSpawnSpec::Define()
{
	using namespace SquadSpawnSpecPrivate;
	It("spawns configured count/stats/team/back-pointers with rotated home-relative grid positions", [this]()
	{
		FSpawnFixture F;
		USquadDefinition* Definition = MakeDefinition(F.Controller);
		const FTransform Home(FRotator(0,90,0), FVector(1000,2000,100));
		ASquad* Squad = F.Spawn(Definition, Home);
		TestTrue("Registered", F.Controller->GetCommandComponent()->GetSquads().Contains(Squad));
		const TArray<ASoldierCharacter*> Soldiers = Squad->GetSoldiers();
		TestEqual("Count from data", Soldiers.Num(), 4);
		TestTrue("Home snapshot", Squad->GetHomeTransform().Equals(Home));
		for (int32 Index = 0; Index < Soldiers.Num(); ++Index)
		{
			ASoldierCharacter* Soldier = Soldiers[Index];
			TestTrue("Back-pointer", Soldier->GetSquad() == Squad);
			TestEqual("Slot", Soldier->GetSlotIndex(), Index);
			TestEqual("Team", Soldier->GetGenericTeamId().GetId(), Team_Player);
			TestEqual("Health", Soldier->GetHealthComponent()->GetMaxHealth(), 125.f);
			TestEqual("Armor", Soldier->GetHealthComponent()->GetBaseArmor(), 0.25f);
			TestEqual("Poise", Soldier->GetCombatStateComponent()->GetMaxPoise(), 60.f);
			TestEqual("Speed", Soldier->GetCharacterMovement()->MaxWalkSpeed, Definition->MoveSpeed);
			const FVector Expected(-static_cast<float>(Index/2)*Definition->FormationSpacing, (Index%2-0.5f)*Definition->FormationSpacing, 0);
			TestTrue("Grid rotated into home space", Soldier->GetActorLocation().Equals(Home.TransformPosition(Expected),0.1f));
		}
		TestFalse("Anchor tick off", Squad->PrimaryActorTick.bCanEverTick);
		TestEqual("Definition immutable", Definition->SoldierCount, 4);
	});

	It("uses edited definition count at next spawn and destroys only owned soldiers on teardown", [this]()
	{
		FSpawnFixture F;
		USquadDefinition* FirstDefinition = MakeDefinition(F.Controller, 3);
		ASquad* First = F.Spawn(FirstDefinition);
		USquadDefinition* Edited = DuplicateObject<USquadDefinition>(FirstDefinition, F.Controller, MakeUniqueObjectName(F.Controller, USquadDefinition::StaticClass(), TEXT("DA_EditedSquadFixture")));
		Edited->SoldierCount = 6;
		ASquad* Second = F.Spawn(Edited, FTransform(FVector(2000,0,0)));
		TestEqual("Initial count", First->GetSoldiers().Num(), 3);
		TestEqual("Edited data count", Second->GetSoldiers().Num(), 6);
		const TArray<ASoldierCharacter*> Owned = First->GetSoldiers();
		First->Destroy();
		TestFalse("Unregistered", F.Controller->GetCommandComponent()->GetSquads().Contains(First));
		for (ASoldierCharacter* Soldier : Owned) { TestTrue("Owned soldier removed", !IsValid(Soldier) || Soldier->IsActorBeingDestroyed()); }
		TestEqual("Other squad untouched", Second->GetSoldiers().Num(), 6);
		TestEqual("Original asset untouched", FirstDefinition->SoldierCount, 3);
	});

	It("rejects fourth squad before it creates any soldiers", [this]()
	{
		TGuardValue<int32> Cap(GetMutableDefault<UGameTuningSettings>()->MaxActiveSquads, 3);
		FSpawnFixture F;
		USquadDefinition* Definition = MakeDefinition(F.Controller);
		for (int32 Index = 0; Index < 3; ++Index) { TestEqual("Accepted count", F.Spawn(Definition)->GetSoldiers().Num(), 4); }
		AddExpectedError(TEXT("cannot register squad"), EAutomationExpectedErrorFlags::Contains, 1);
		ASquad* Rejected = F.Spawn(Definition);
		TestEqual("No rejected soldiers", Rejected->GetSoldiers().Num(), 0);
		TestEqual("Exactly three registered", F.Controller->GetCommandComponent()->GetSquads().Num(), 3);
	});

	It("rejects zero count, missing class, invalid columns/leashes and nonfinite stats via data validation", [this]()
	{
		USquadDefinition* Definition = MakeDefinition(GetTransientPackage());
		FString Error;
		TestTrue("Valid fixture", Definition->ValidateDefinition(Error));
		Definition->SoldierCount = 0;
		FDataValidationContext Context;
		TestTrue("Zero invalid in editor", Definition->IsDataValid(Context) == EDataValidationResult::Invalid);
		Definition->SoldierCount = 4;
		Definition->SoldierClass = nullptr;
		FDataValidationContext MissingClass;
		TestTrue("Missing class invalid in editor", Definition->IsDataValid(MissingClass) == EDataValidationResult::Invalid);
		Definition->SoldierClass = ASoldierCharacter::StaticClass();
		Definition->FormationColumns = 0;
		TestFalse("Zero columns rejected", Definition->ValidateDefinition(Error));
		Definition->FormationColumns = 2;
		Definition->GuardLeashRadius = Definition->EngageRadius - 1.f;
		TestFalse("Leash shorter than engagement rejected", Definition->ValidateDefinition(Error));
		Definition->GuardLeashRadius = 1500.f;
		Definition->MaxHealth = std::numeric_limits<float>::quiet_NaN();
		TestFalse("Nonfinite health rejected", Definition->ValidateDefinition(Error));
	});

	It("discovers the authored SquadDefinition asset and validates its soldier blueprint", [this]()
	{
		UAssetManager& Manager = UAssetManager::Get();
		const FPrimaryAssetId Id(TEXT("SquadDefinition"), TEXT("DA_Squad_Test"));
		TArray<FPrimaryAssetId> Ids;
		Manager.GetPrimaryAssetIdList(Id.PrimaryAssetType, Ids);
		TestTrue("Definition type registered and asset discovered", Ids.Contains(Id));
		USquadDefinition* Asset = Cast<USquadDefinition>(Manager.GetPrimaryAssetPath(Id).TryLoad());
		if (!TestNotNull("Authored definition loads", Asset)) { return; }
		FDataValidationContext Context;
		TestTrue("Authored asset valid", Asset->IsDataValid(Context) == EDataValidationResult::Valid);
		TestTrue("Soldier blueprint derives from body", Asset->SoldierClass && Asset->SoldierClass->IsChildOf(ASoldierCharacter::StaticClass()));
		TestTrue("Actual inherited primary ID", Asset->GetPrimaryAssetId() == Id);
	});
}
#endif
