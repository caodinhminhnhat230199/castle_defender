#include "Enemy/EnemyTargeting.h"

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
