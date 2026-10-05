#include "Core/GameDebug.h"

namespace GameDebug
{
	TAutoConsoleVariable<int32> CVarCombat(TEXT("game.debug.Combat"), 0, TEXT("Draw combat debug: health, hits, states."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarAI(TEXT("game.debug.AI"), 0, TEXT("Draw enemy and soldier AI state."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarArmy(TEXT("game.debug.Army"), 0, TEXT("Draw squad anchors, formation slots, orders."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarLanes(TEXT("game.debug.Lanes"), 0, TEXT("Draw lane routes, blockers, break costs."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarDirector(TEXT("game.debug.Director"), 0, TEXT("Show Encounter Director budget and spawn queue."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarFeedback(TEXT("game.debug.Feedback"), 0, TEXT("Show the last 10 played Feedback tags on screen."), ECVF_Cheat);
	TAutoConsoleVariable<int32> CVarCombatStates(TEXT("game.debug.CombatStates"), 0, TEXT("Draw poise and active combat states with remaining seconds above each unit."), ECVF_Cheat);
}
