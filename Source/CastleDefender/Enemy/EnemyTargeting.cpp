#include "Enemy/EnemyTargeting.h"
#include "Enemy/EnemyArchetypeDefinition.h"

int32 EnemyTargeting::PickTarget(TConstArrayView<FEnemyTargetCandidate> Candidates, TConstArrayView<EEnemyTargetKind> Priority)
{
	int32 Best = INDEX_NONE;
	int32 BestRank = MAX_int32;
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		const FEnemyTargetCandidate& Candidate = Candidates[Index];
		const int32 Rank = Priority.Find(Candidate.Kind);
		if (Rank == INDEX_NONE) { continue; }
		if (Best != INDEX_NONE)
		{
			const FEnemyTargetCandidate& Current = Candidates[Best];
			if (Rank != BestRank) { if (Rank > BestRank) { continue; } }
			else if (Candidate.bRecentAttacker != Current.bRecentAttacker) { if (!Candidate.bRecentAttacker) { continue; } }
			else if (Candidate.DistanceSq >= Current.DistanceSq) { continue; }
		}
		Best = Index;
		BestRank = Rank;
	}
	return Best;
}

int32 EnemyTargeting::PickAttack(TConstArrayView<FEnemyAttackDefinition> Attacks, TConstArrayView<double> ReadyTimes, float Gap, double Now, float Roll)
{
	const auto Usable = [&](int32 Index)
	{
		return Attacks[Index].Weight > 0.f && Gap <= Attacks[Index].Range && (!ReadyTimes.IsValidIndex(Index) || Now >= ReadyTimes[Index]);
	};
	float TotalWeight = 0.f;
	for (int32 Index = 0; Index < Attacks.Num(); ++Index) { if (Usable(Index)) { TotalWeight += Attacks[Index].Weight; } }
	if (TotalWeight <= 0.f) { return INDEX_NONE; }
	float Remaining = FMath::Clamp(Roll, 0.f, 1.f) * TotalWeight;
	int32 Last = INDEX_NONE;
	for (int32 Index = 0; Index < Attacks.Num(); ++Index)
	{
		if (!Usable(Index)) { continue; }
		Last = Index;
		Remaining -= Attacks[Index].Weight;
		if (Remaining < 0.f) { return Index; }
	}
	return Last; // Roll == 1 lands on the last usable attack.
}
