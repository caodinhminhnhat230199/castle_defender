#pragma once

#include "HAL/IConsoleManager.h"

// game.debug.* toggles (D-16). Features read these to draw debug info; Foundation only declares them.
// Visual Logger convention: use the domain log category, and log every AI decision with its state name:
//   UE_VLOG(this, LogGameAI, Log, TEXT("State %s -> %s"), *Old, *New);
namespace GameDebug
{
#if !UE_BUILD_SHIPPING
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarCombat;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarCombatTrace;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarEnemy;
#endif
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarAI;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarArmy;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarLanes;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarDirector;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarFeedback;
	extern CASTLEDEFENDER_API TAutoConsoleVariable<int32> CVarCombatStates;
}
