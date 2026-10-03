# Foundation: Tasks

## 1. Summary

Ten tasks that turn an empty folder into a buildable, testable, debuggable UE5 C++ project with the shared contracts from [technical-plan.md](technical-plan.md). All tasks are Phase F. `T-FND-01` to `T-FND-07` block every gameplay task.

## 2. Task Overview

| ID | Task | Type | Phase | Priority | Dependencies | Status |
|---|---|---|---|---|---|---|
| T-FND-01 | Create UE5 C++ project, pin engine version, name module | BUILD | F | P0-blocker | none | Todo |
| T-FND-02 | Git + LFS + `.gitignore` | BUILD | F | P0-blocker | T-FND-01 | Todo |
| T-FND-03 | Domain folders, log categories, Build.cs dependencies | BUILD | F | P0-blocker | T-FND-01 | Todo |
| T-FND-04 | Gameplay Tag taxonomy (native tags) | GAMEPLAY | F | P0-blocker | T-FND-03 | Todo |
| T-FND-05 | Shared combat contract skeletons + team interface | GAMEPLAY | F | P0-blocker | T-FND-04 | Todo |
| T-FND-06 | Enhanced Input base + `AHeroPlayerController` skeleton + context switching | GAMEPLAY | F | P0-blocker | T-FND-03 | Todo |
| T-FND-07 | `UGameTuningSettings` + Primary Asset Types + `IsDataValid` pattern | TOOLS | F | P0-blocker | T-FND-03 | Todo |
| T-FND-08 | Reference PC spec + packaged Development build smoke + profiling checklist | BUILD | F | High | T-FND-01 | Todo |
| T-FND-09 | Debug tooling: CVars, cheat manager, Visual Logger convention | TOOLS | F | High | T-FND-03 | Todo |
| T-FND-10 | Automation test harness: Spec + Functional Test map + CLI runner | QA | F | High | T-FND-05 | Todo |

## 3. Detailed Tasks

## Phase F

### T-FND-01 — Create UE5 C++ project, pin engine version, name module
**Type** BUILD · **Phase** F

**Objective** A blank C++ third-person-capable project that builds.

**Related Requirements** R-FND-01, R-FND-02, AC-FND-01, Q-15

**Dependencies** none

**Implementation Notes**
- [ ] Answer Q-15: project name, module name `<Game>`, UE 5.x version. Record them in `00-foundation/technical-plan.md` §1 and `main_implement_plan.md` §2.
- [ ] Create from the Blank C++ template (not the Third Person template, to avoid template code; copy only needed input/anim assets later).
- [ ] Set `EngineAssociation` in `.uproject` to the pinned version.
- [ ] Default maps: an empty `L_Boot` in `Content/<Game>/Maps/`.
- [ ] Fill `AGENTS.md`: §1 engine/module line and §7 "Build editor target" command (the exact command you ran). Decide where Win64 builds are made if development stays on macOS.

**Expected Files / Assets** `<Game>.uproject`, `Source/<Game>/<Game>.Build.cs`, `Source/<Game>.Target.cs`, `Source/<Game>Editor.Target.cs`, `Config/DefaultEngine.ini`, `Config/DefaultGame.ini`

**Test Case** Clean build of `<Game>Editor` Development from CLI (`Build.sh`/`Build.bat`) → exit code 0.

**Acceptance Criteria**
- [ ] Editor and game targets build with zero errors.
- [ ] Project opens in the editor at the pinned engine version.

**Verification** CLI build log; open editor.

---

### T-FND-02 — Git + LFS + `.gitignore`
**Type** BUILD · **Phase** F

**Objective** Version control that keeps binaries in LFS and generated folders out.

**Related Requirements** R-FND-09, AC-FND-02, D-18

**Dependencies** T-FND-01

**Implementation Notes**
- [ ] `git init`, `git lfs install`.
- [ ] `.gitattributes`: LFS for `*.uasset *.umap *.fbx *.png *.tga *.exr *.wav *.ogg *.psd *.blend`.
- [ ] `.gitignore`: `Binaries/ Intermediate/ Saved/ DerivedDataCache/ .vs/ .idea/ *.sln *.xcworkspace .codegraph/`.
- [ ] Commit GDD, `ai/`, `.claude/`, `AGENTS.md`, `CLAUDE.md` with the project. Commit and branch format: `AGENTS.md` §8.
- [ ] Create branch `main`; work on feature branches per task group.

**Expected Files / Assets** `.gitignore`, `.gitattributes`

**Test Case** Clone into a new folder → `git lfs pull` → build → open editor → no missing asset warnings.

**Acceptance Criteria**
- [ ] `git status` clean after a build (no generated files tracked).
- [ ] `git lfs ls-files` lists `.uasset`/`.umap` files.

**Verification** Fresh-clone build.

---

### T-FND-03 — Domain folders, log categories, Build.cs dependencies
**Type** BUILD · **Phase** F

**Objective** Source/Content layout from technical-plan §4–§5 so ownership is obvious from day one.

**Related Requirements** R-FND-02, D-01, D-02

**Dependencies** T-FND-01

**Implementation Notes**
- [ ] Create `Source/<Game>/{Core, Combat, Hero, Player, Army, Enemy, Structures, Navigation, Zones, Encounter, Run, Perks, Boss, Feedback, UI, Tests}` (empty folders get a placeholder `.h` only when the first class lands; do not commit empty dirs).
- [ ] Declare log categories in `Core/GameLog.h`: `LogGameCombat, LogGameAI, LogGameArmy, LogGameNav, LogGameEncounter, LogGameRun, LogGameUI`.
- [ ] Add module deps from technical-plan §3 to `<Game>.Build.cs`.
- [ ] Create Content folders from technical-plan §5.

**Expected Files / Assets** `Source/<Game>/Core/GameLog.h/.cpp`, Content folder tree

**Test Case** Build; `UE_LOG(LogGameCombat, Log, ...)` from a test actor appears in Output Log.

**Acceptance Criteria**
- [ ] Build succeeds with the new dependencies.
- [ ] No `Managers/`, `Utils/`, `Misc/` folders exist.

**Verification** Build + PIE log line.

---

### T-FND-04 — Gameplay Tag taxonomy (native tags)
**Type** GAMEPLAY · **Phase** F

**Objective** One central tag definition with the roots from master plan §8.

**Related Requirements** AC-FND-03, D-04, D-06

**Dependencies** T-FND-03

**Implementation Notes**
- [ ] Use `UE_DECLARE_GAMEPLAY_TAG_EXTERN` / `UE_DEFINE_GAMEPLAY_TAG` in `Core/GameTags.h/.cpp` (verify macro names for the pinned UE version).
- [ ] Define the leaf tags listed in master plan §8 (`State.*`, `Unit.*`, `Structure.Role.*`, `Command.*`, `Zone.*`, `Damage.Physical`, `Perk.Category.*`). `Feedback.*`, `Stat.*`, `Modifier.*` leaves are added by owning features.
- [ ] Add a comment block at the top: "New roots require an entry in 00-foundation/technical-plan.md §18".

**Expected Files / Assets** `Source/<Game>/Core/GameTags.h/.cpp`

**Test Case** Open a Gameplay Tag picker on any property → all roots visible; code references `GameTags::State_Combat_Staggered` compile.

**Acceptance Criteria**
- [ ] All master plan §8 roots/leaves present.
- [ ] No tags defined in `.ini` files that duplicate native tags.

**Verification** Editor tag picker; build.

---

### T-FND-05 — Shared combat contract skeletons + team interface
**Type** GAMEPLAY · **Phase** F

**Objective** Compile-ready types for D-05 so CMB, ENM, SQD, DEF, SYN build on one pipeline.

**Related Requirements** AC-FND-04, D-05, technical-plan §7

**Dependencies** T-FND-04

**Implementation Notes**
- [ ] `Combat/CombatTypes.h`: `ECombatLayer`, `FCombatHit` (fields from technical-plan §7, `USTRUCT(BlueprintType)`).
- [ ] `Combat/HealthComponent`: Max/Current health, `BaseArmor` (fraction of damage blocked, clamped 0–0.9, set by the owner from its definition), `ApplyHit`, `OnDamaged`, `OnDeath`. Death fires once. The armor math and the Armor Broken multiplier are added by T-SYN-02.
- [ ] `Combat/CombatStateComponent`: API only (`ApplyPoiseDamage`, `ApplyState`, `RemoveState`, `HasState`, delegates). Minimal implementation: store tags without timing. Timing/poise logic is `T-SYN-01`.
- [ ] Team: implement `IGenericTeamAgentInterface` on a small base used by characters (or per class later); team IDs constants `Team_Player = 0`, `Team_Enemy = 1` in `CombatTypes.h`.
- [ ] Helper `static bool AreHostile(const AActor*, const AActor*)`.

**Expected Files / Assets** `Source/<Game>/Combat/CombatTypes.h`, `HealthComponent.h/.cpp`, `CombatStateComponent.h/.cpp`

**Test Case** Automation Spec: actor with `UHealthComponent` (Max 100) receives `FCombatHit{Damage=100}` → `OnDeath` fires exactly once; second hit does nothing.

**Acceptance Criteria**
- [ ] Spec test passes.
- [ ] Components are `BlueprintSpawnableComponent`; delegates `BlueprintAssignable`.

**Verification** Automation Spec `<Game>.Combat.Health`.

---

### T-FND-06 — Enhanced Input base + `AHeroPlayerController` skeleton + context switching
**Type** GAMEPLAY · **Phase** F

**Objective** Device-agnostic input with one mapping context per mode.

**Related Requirements** R-FND-06, AC-FND-05, D-12, R-CMB-36

**Dependencies** T-FND-03

**Implementation Notes**
- [ ] Input Actions (P0 set): `IA_Move, IA_Look, IA_Sprint, IA_LightAttack, IA_HeavyAttack, IA_Dodge, IA_Block, IA_LockOn, IA_Interact`. Later sets are created by owning features (`IA_CommandWheel`, `IA_TacticalFocus`, build actions).
- [ ] Mapping contexts: `IMC_Combat` (KBM), empty `IMC_CommandWheel`, `IMC_TacticalFocus`, `IMC_CommanderSpirit`, `IMC_Build` with priorities documented.
- [ ] `AHeroPlayerController` is the **single owner of player mode** (D-19): `PushMode(EPlayerMode, FName Reason)` / `PopMode(Reason)` on a stack (`Combat, Wheel, Build, Focus, Spirit, Modal`). The top mode decides the active mapping contexts (via `UEnhancedInputLocalPlayerSubsystem`) and the UI input mode; every change broadcasts `OnPlayerModeChanged(Old, New)` and is logged. Features push/pop; they never set mapping contexts or UI input mode directly, and HUD layers / tactical overlays only listen to this event.
- [ ] Parry and lock-on target switching are CMB-owned actions (`IA_Parry`, `IA_LockOnSwitch`); whether Parry is a separate key or a timed block press is NEW-CMB-01, decided by CMB before P0. Reserve a key for it either way.
- [ ] Write the default KBM key map in `00-foundation/input-keymap.md` and **reserve** keys for Command Wheel, squad selection, Tactical Focus and build mode now, so combat bindings never collide with them (GDD §29.2, R-CMB-36 / AC-CMB-14). Owners fill the reserved actions later.

**Expected Files / Assets** `Source/<Game>/Player/HeroPlayerController.h/.cpp`, `Content/<Game>/Core/Input/IA_*`, `IMC_*`, `ai/game/00-foundation/input-keymap.md`

**Test Case** PIE: debug key pushes `Build` → log shows switch, `IA_LightAttack` no longer fires; push `Modal` then pop it → back to `Build`, not `Combat`; pop `Build` → `Combat`. Automation Spec on the stack: pop of a reason not on top removes only that entry and the top mode is unchanged.

**Acceptance Criteria**
- [ ] Mode stack works (push/pop, out-of-order pop), every change is logged and broadcast.
- [ ] No input bound directly to keys in C++ (all via actions).

**Verification** PIE manual check.

---

### T-FND-07 — `UGameTuningSettings` + Primary Asset Types + `IsDataValid` pattern
**Type** TOOLS · **Phase** F

**Objective** One home for global tunables and stable IDs for definition assets.

**Related Requirements** R-FND-03, R-FND-04, AC-FND-06, D-06, D-14

**Dependencies** T-FND-03

**Implementation Notes**
- [ ] `Core/GameTuningSettings` (`UDeveloperSettings`, `Config=Game, DefaultConfig`): empty categories `Combat`, `Army`, `Focus`, `Respawn`, `Debug`. Features add properties.
- [ ] Base class `UGameDefinition : UPrimaryDataAsset` overriding `GetPrimaryAssetId()`. **Every** definition class derives from it. The Primary Asset Type name is the class name without the `U` prefix (e.g. `HeroClassDefinition`, `StructureDefinition`); the canonical list lives in technical-plan §9 and is frozen before VS save data exists.
- [ ] Register Primary Asset Types for the definitions in technical-plan §9 in `DefaultGame.ini` (Asset Manager settings) as each type lands; register `UGameDefinition` scan paths now.
- [ ] `IsDataValid` example on `UGameDefinition` (e.g., DisplayName required).

**Expected Files / Assets** `Source/<Game>/Core/GameTuningSettings.h/.cpp`, `Core/GameDefinition.h/.cpp`, `Config/DefaultGame.ini`

**Test Case** Create a test Data Asset with an empty required field → Data Validation reports an error.

**Acceptance Criteria**
- [ ] Settings page visible under Project Settings → Game.
- [ ] Asset Manager lists the registered type.

**Verification** Editor: Data Validation on the folder; Asset Manager audit window.

---

### T-FND-08 — Reference PC spec + packaged Development build smoke + profiling checklist
**Type** BUILD · **Phase** F

**Objective** Measure on real hardware from the start (GDD §31).

**Related Requirements** R-FND-07, AC-FND-09, D-15

**Dependencies** T-FND-01

**Implementation Notes**
- [ ] Record reference PC (CPU, GPU, RAM, resolution, target frame rate as a working number, marked [TUNABLE]) in `00-foundation/technical-plan.md` §15.
- [ ] Package Win64 Development build of `L_Boot` (needs a Windows machine; UE does not build Win64 on macOS — verify for the pinned version). Fill `AGENTS.md` §7 "Package a Development build".
- [ ] Write `ai/game/00-foundation/profiling-checklist.md`: how to launch with `-trace=cpu,gpu,frame,memory`, open Unreal Insights, which `stat` commands to capture, where to store traces (outside git).

**Expected Files / Assets** packaged build (not committed), `profiling-checklist.md`

**Test Case** Launch packaged build → `stat unit` visible → `game.debug.Combat 1` works in the Development build → capture 30 s trace → open in Insights.

**Acceptance Criteria**
- [ ] Packaged build runs on the reference PC.
- [ ] Checklist reproduces a trace capture.

**Verification** Manual run following the checklist.

---

### T-FND-09 — Debug tooling: CVars, cheat manager, Visual Logger convention
**Type** TOOLS · **Phase** F

**Objective** Fast inspection of any system without custom UI.

**Related Requirements** AC-FND-07, D-16

**Dependencies** T-FND-03

**Implementation Notes**
- [ ] `Core/GameDebug.h`: `TAutoConsoleVariable<int32>` for `game.debug.Combat`, `game.debug.AI`, `game.debug.Army`, `game.debug.Lanes`, `game.debug.Director`. Features read them; this task only declares them.
- [ ] `UGameCheatManager` with `SpawnTestDummy`, `God`, `SetTimeDilation <float>`. Features add their cheats here (`SpawnEnemy`, `SpawnSquad`, `StartWave`, `DamageCore`, `KillHero`, `SkipPhase`).
- [ ] Visual Logger convention: category = log category; every AI decision logs `UE_VLOG` with state name.

**Expected Files / Assets** `Source/<Game>/Core/GameDebug.h/.cpp`, `Core/GameCheatManager.h/.cpp`

**Test Case** PIE: `game.debug.Combat 1` → test debug sphere draws; `SpawnTestDummy` spawns an actor with `UHealthComponent`.

**Acceptance Criteria**
- [ ] CVars listed by `help game.debug`.
- [ ] Cheats work in PIE and are compiled out of Shipping.

**Verification** PIE console.

---

### T-FND-10 — Automation test harness: Spec + Functional Test map + CLI runner
**Type** QA · **Phase** F

**Objective** Every later task can add a test that runs from one command.

**Related Requirements** R-FND-08, AC-FND-08, D-16

**Dependencies** T-FND-05

**Implementation Notes**
- [ ] `Source/<Game>/Tests/` with the Health Spec from `T-FND-05` named `<Game>.Combat.Health`.
- [ ] Enable Functional Testing Editor plugin; create `Content/<Game>/Maps/Test/FT_Smoke` with one `AFunctionalTest` that spawns a dummy, damages it and asserts death.
- [ ] Script `Tools/run_tests.sh` / `.bat` invoking `UnrealEditor-Cmd <Game>.uproject -ExecCmds="Automation RunTests <Game>.;Quit" -unattended -nullrhi -log` (verify flags for the pinned UE version; Functional Tests may need RHI).
- [ ] Document test naming: `<Game>.<Feature>.<Case>`; Functional Test maps `FT_<Feature>_<Case>`.
- [ ] Fill `AGENTS.md` §7 "Run automation tests" with the script command.

**Expected Files / Assets** `Source/<Game>/Tests/*.spec.cpp`, `Content/<Game>/Maps/Test/FT_Smoke.umap`, `Tools/run_tests.*`

**Test Case** Run the script → both tests reported as passed; exit code 0.

**Acceptance Criteria**
- [ ] CLI run passes both tests.
- [ ] A deliberately failing assert makes the script exit non-zero.

**Verification** CLI run.

## 4. Dependency Graph

```mermaid
flowchart LR
  F01[T-FND-01] --> F02[T-FND-02]
  F01 --> F03[T-FND-03]
  F01 --> F08[T-FND-08]
  F03 --> F04[T-FND-04]
  F03 --> F06[T-FND-06]
  F03 --> F07[T-FND-07]
  F03 --> F09[T-FND-09]
  F04 --> F05[T-FND-05]
  F05 --> F10[T-FND-10]
```

## 5. Integration / Regression Checklist
- [ ] Fresh clone builds.
- [ ] Test script passes.
- [ ] Packaged Development build launches.
- [ ] No gameplay logic added in Foundation tasks.
- [ ] State ownership in code matches the D-07 table (R-FND-05); review at every gate.

## 6. Final Definition of Done
All AC-FND-01…09 pass; master plan §2 updated with project name, UE version and reference PC.
