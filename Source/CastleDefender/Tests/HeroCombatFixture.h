#pragma once

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Hero/HeroCharacter.h"
#include "Hero/HeroClassDefinition.h"

/** Isolated registered pawn: exercises real montage playback rather than ownerless state simulation. */
struct FHeroCombatFixture
{
	UWorld* World = nullptr;
	AHeroCharacter* Hero = nullptr;

	FHeroCombatFixture()
	{
		World = UWorld::CreateWorld(EWorldType::Game, false);
		Hero = World->SpawnActor<AHeroCharacter>();
		Hero->SetHeroClassDefinition(DuplicateObject<UHeroClassDefinition>(
			LoadObject<UHeroClassDefinition>(nullptr, TEXT("/Game/CastleDefender/Hero/DA_HeroClass_Warlord")), Hero));
		Hero->GetMesh()->SetSkeletalMesh(LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/CastleDefender/Placeholder/Mannequins/Meshes/SKM_Manny_Simple")));
		Hero->GetMesh()->SetAnimInstanceClass(UAnimInstance::StaticClass());
	}

	~FHeroCombatFixture()
	{
		World->DestroyWorld(false);
	}

	void BeginPlay()
	{
		// Actor ProcessEvent rejects dynamic callbacks until actors are initialized.
		World->InitializeActorsForPlay(FURL());
		Hero->DispatchBeginPlay();
	}
};
#endif
