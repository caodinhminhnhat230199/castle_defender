#pragma once

#include "CoreMinimal.h"
#include "EnemyTargeting.generated.h"

/** Target kinds an archetype's priority list orders (R-ENM-27). */
UENUM(BlueprintType)
enum class EEnemyTargetKind : uint8 { Hero, Soldier, PathObstacle, CombatTower, Blocker, Objective };

/** One hostile that already passed the alive/hostile/radius filters. */
struct FEnemyTargetCandidate
{
	AActor* Actor = nullptr;
	EEnemyTargetKind Kind = EEnemyTargetKind::Hero;
	float DistanceSq = 0.f;
	bool bRecentAttacker = false;
};

namespace EnemyTargeting
{
	/**
	 * Index of the best candidate by (position of its kind in Priority, recent attacker first, distance²),
	 * or INDEX_NONE. Kinds missing from Priority are never picked: the list is what the archetype may target.
	 */
	CASTLEDEFENDER_API int32 PickTarget(TConstArrayView<FEnemyTargetCandidate> Candidates, TConstArrayView<EEnemyTargetKind> Priority);
}
