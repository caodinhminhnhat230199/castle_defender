# G0 Enemy Combat Functional Tests & G0 Evaluation: T-ENM-12

UE 5.8.3, Development Win64, branch `feat/01-hero-combat`. Map: `L_Test_EnemyCombat`.

## 1. Automated Scenarios

Execution command:
```powershell
Tools\run_tests.bat -Filter "Project.Functional Tests.CastleDefender.Maps.Test.L_Test_EnemyCombat"
Tools\run_tests.bat -Filter "CastleDefender.Enemy.CombatSuite"
```

| Functional Test | Scenario | Verified Acceptance Criteria | Result |
|---|---|---|---|
| `FT_Enemy_AggroChase` | Idle outside aggro radius (1200 cm), Engages inside aggro radius (250 cm) | R-ENM-10, R-ENM-11; AC-ENM-01, AC-ENM-07 | Pass |
| `FT_Enemy_TelegraphGap` | Telegraph wind-up precedes active hit window; MinEnemyTelegraphTime >= 0.4s | R-ENM-05; AC-ENM-02 | Pass |
| `FT_Enemy_StaggerCancel` | Poise break during wind-up cancels attack montage, closes trace window, staggers brain | R-ENM-07, R-ENM-08; AC-ENM-04, AC-ENM-05 | Pass |
| `FT_Enemy_DeathReportOnce` | Lethal hit sets Dead state, clears pawn collision, reports removal exactly once | R-ENM-04, R-ENM-09; AC-ENM-06 | Pass |
| `FT_Enemy_ParryStaggers` | Hero parry intercepts enemy attack, prevents damage, staggers enemy attacker | R-ENM-06; AC-ENM-03 | Pass |

## 2. Full Gate Verification Evidence

- `CastleDefender.Enemy.CombatSuite`: 5/5 Automation Specs passed (0 warnings, 0 failures, exit code 0).
- `Project.Functional Tests.CastleDefender.Maps.Test.L_Test_EnemyCombat`: 5/5 Functional Tests passed (0 warnings, 0 failures, exit code 0).
- Full regression suite `Tools\run_tests.bat`: 235/235 tests passed headless with exit code 0.

## 3. G0 Enemy Checklist Item Evaluation

Checklist item (master plan §3 G0): "one melee enemy is enough for 3–5 minutes".

| Evaluation Point | Observation & Contract Status | Decision |
|---|---|---|
| 1. Melee Combat Loop Sufficiency | Melee enemy possesses two telegraphed attacks (Light 0.5s wind-up, Heavy 0.8s wind-up), dynamic targeting, aggro chase, hit reactions, poise break into Staggered state, and responds to player parry by staggering. Supports 3-5 minute combat loop with Warlord hero across 1/3/5 enemy group areas in `L_CombatSandbox`. | KEEP |
| 2. Attack Readability & Defense Mechanics | Both attacks give readable wind-ups exceeding the 0.4s tuning minimum before hit-windows open, allowing player dodge, block, and parry. Enemy ceases rotation during the active strike window so sideways evasions are effective. | KEEP |
| 3. State & Lifecycle Robustness | Enemy correctly transitions between Idle -> Engage -> Attacking -> Staggered -> Dead. All removal paths report exactly once (`bRemovalReported` guard). Corpses ignore pawn collision and despawn cleanly after delay. | KEEP |
| 4. Tuning Balance Exposure | All attributes (Health, Armor, Poise, MoveSpeed, Attacks, Telegraph, Cooldowns) reside exclusively in `DA_Enemy_Melee` / Data Assets; zero hardcoded constants in C++. | KEEP |
