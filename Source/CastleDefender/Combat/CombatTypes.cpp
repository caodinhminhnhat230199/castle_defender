#include "Combat/CombatTypes.h"

#include "GenericTeamAgentInterface.h"

bool AreHostile(const AActor* A, const AActor* B)
{
	const FGenericTeamId TeamA = FGenericTeamId::GetTeamIdentifier(A);
	const FGenericTeamId TeamB = FGenericTeamId::GetTeamIdentifier(B);
	return TeamA != FGenericTeamId::NoTeam && TeamB != FGenericTeamId::NoTeam && TeamA != TeamB;
}
