# Foundation: UE5 Project Architecture

Source: GDD v2 header (platform, engine, implementation direction), §29, §31, §34.1, §34.2, §38. Decisions D-01…D-18 are summarized in [main_implement_plan.md §7](../main_implement_plan.md#7-architecture-baseline). This file holds the detail every feature technical plan builds on.

> Foundation code and editor assets exist, and Phase F passed on 2026-10-04. See tasks.md and progress.md for verification evidence. Later-phase classes and assets remain proposals until their tasks are completed; P0 is open.

## 1. Project Context

- Engine: **UE 5.8** (5.8.3 at creation), pinned in `T-FND-01` (Q-15). Project and runtime module: **`CastleDefender`** (written `<Game>` in docs).
- Toolchain on the dev PC: Visual Studio 2026 Community, MSVC 14.51 (UBT warns it is newer than the preferred 14.50; builds succeed).
- Platform: Windows PC. Keyboard + mouse first; gamepad mappable later (GDD §29.1).
- Game type: third-person action + squad command + tower defense roguelite.
- Expected scale (prototype): 1 hero, ≤3 squads × ~8–12 soldiers, enemy count capped by the P2 benchmark (Q-04), ≤ ~15 structures per Siege Site.
- Multiplayer: none (GDD §3). Code still uses GameMode/GameState/PlayerState for clear ownership, not for replication.
- Team: solo developer + AI coding agents.

## 2. Architecture Principles

1. Gameplay feel first; architecture serves the gate questions.
2. One module, domain folders, composition over inheritance.
3. C++ owns rules and state; Blueprint owns content, tuning, presentation.
4. Definition data (Data Assets) ≠ runtime state (components/actors) ≠ presentation assets. Never mutate a Data Asset at runtime.
5. Events over Tick. Any per-frame work must justify itself.
6. Measure before optimizing. No pooling/Mass/schedulers without a benchmark.
7. Every [TUNABLE] value lives in data.

## 3. Module / Plugin Layout

```text
<Game>.uproject
Source/
  <Game>/            runtime module (all gameplay)
  <Game>.Target.cs
  <Game>Editor.Target.cs
```

- `<Game>Editor` module: create only when editor-only code appears (e.g., lane authoring tools, data validators). Not in Phase F.
- Engine plugins to enable: Enhanced Input (default in UE5), Gameplay Tags (built-in), Functional Testing Editor, (later) StateTree only if D-08 changes.
- Module dependencies (`<Game>.Build.cs`): `Core, CoreUObject, Engine, InputCore, EnhancedInput, GameplayTags, AIModule, NavigationSystem, UMG, Slate, SlateCore, DeveloperSettings`. Add others when a task needs them, not before.

## 4. Source Structure

```text
Source/<Game>/
  Core/          UGameTuningSettings, tag native declarations, log categories, CVars, cheat manager
  Combat/        UHealthComponent, UCombatStateComponent, FCombatHit, damage helpers, anim notify states
  Hero/          AHeroCharacter, UHeroCombatComponent, UStaminaComponent, ULockOnComponent, UHeroClassDefinition
  Player/        AHeroPlayerController, ARunPlayerState, input config, UCommandComponent (owned by SQD)
  Army/          ASquad, ASoldierCharacter, USquadDefinition, formation, target selection
  Enemy/         AEnemyCharacter, UEnemyBrainComponent, UEnemyArchetypeDefinition
  Structures/    AStructureBase, ACoreStructure, tower weapon, placement, UStructureDefinition
  Navigation/    ULaneNavigationSubsystem, ALaneRoute, corridor grid, break-cost search
  Zones/         ATacticalZone, UTacticalZoneDefinition
  Encounter/     UEncounterDirectorComponent, UWaveDefinition, spawn points, forecast types
  Run/           ARunGameMode, ARunGameState, URunDefinition, run phase logic
  Perks/         UPerkDefinition, UPerkEffect, UPerkManagerComponent
  Boss/          ABossCharacter, UBossDefinition, phase logic
  Feedback/      UFeedbackSubsystem, feedback row struct
  UI/            C++ widget bases only when Blueprint widgets need typed data
  Tests/         Automation Spec files (*.spec.cpp), test helpers
  # Vertical Slice only (provisional, created by their first task):
  Meta/  World/  Economy/  Conversion/  Tutorial/
```

Each folder uses `Public/`/`Private/` only if a second module ever needs its headers; with one module, keep headers next to sources inside the domain folder.

## 5. Content Structure

```text
Content/<Game>/
  Core/          input actions, mapping contexts, tag tables, global data
  Hero/          BP_Hero_Warlord, ABP_, AM_ montages, DA_HeroClass_Warlord
  Army/          BP_Squad_*, BP_Soldier_*, DA_Squad_*
  Enemy/         BP_Enemy_*, DA_Enemy_*
  Structures/    BP_Structure_*, DA_Structure_*
  Zones/         BP_TacticalZone_*, DA_Zone_*
  Encounter/     DA_Wave_*, DA_Modifier_*
  Run/           DA_Run_*, BP_RunGameMode
  Perks/         DA_Perk_*
  Boss/          BP_Boss_*, DA_Boss_*
  Feedback/      DT_Feedback, NS_*, SFX_*, camera shakes
  UI/            WBP_*
  Maps/          L_CombatSandbox, L_CombinedArms, L_SiegeSite_Proto, test maps under Maps/Test/
  Placeholder/   third-party/sample placeholder assets (delete before VS art lock)
```

## 6. Gameplay Framework Ownership

| UE type | Proposed class | Owns | Notes |
|---|---|---|---|
| GameInstance | default | nothing in prototype | Meta subsystem arrives at VS (MET) |
| GameMode | `ARunGameMode` (P2+) / `ASandboxGameMode` (P0–P1) | Run rules, spawning hero, owns `UEncounterDirectorComponent` | Sandbox mode is a thin Blueprint subclass, no run logic |
| GameState | `ARunGameState` | Run phase, wave index, forecast, run resource, Core ref | UI reads via delegates |
| PlayerController | `AHeroPlayerController` | Input contexts, `UCommandComponent`, build placement, Focus, Commander Spirit view switch | Not a gameplay dumping ground: each capability is a component |
| PlayerState | `ARunPlayerState` | Perks, revive charges, run stats | Survives hero death |
| Character | `AHeroCharacter`, `ASoldierCharacter`, `AEnemyCharacter`, `ABossCharacter` | World body + components | Soldiers/enemies keep components minimal |
| Actor | `ASquad`, `AStructureBase`, `ACoreStructure`, `ATacticalZone`, `ALaneRoute`, spawn points | World presence | `ASquad` = anchor, no mesh |
| WorldSubsystem | `ULaneNavigationSubsystem`, `UFeedbackSubsystem` | Lane layer for the loaded Siege Site; feedback playback | World lifetime matches the battlefield |
| DeveloperSettings | `UGameTuningSettings` | Global tunables (Focus, respawn, hit stop defaults, debug flags) | Editable in Project Settings |

## 7. Shared Combat Contract (D-05)

Created as skeletons in `T-FND-05`; gameplay logic filled by CMB, SYN, ENM.

```text
ECombatLayer : Hero | Army | Tower | Enemy | Environment

FCombatHit
  AActor* Instigator           (weak)
  ECombatLayer SourceLayer
  float Damage
  float PoiseDamage
  FGameplayTag DamageType      (Damage.Physical for prototype)
  FGameplayTagContainer AppliedStates   (e.g. State.Combat.ArmorBroken)
  float StateDuration          (seconds; 0 = definition default)
  FVector HitLocation, HitDirection
  EPhysicalSurface Surface     (trace result; unknown = row default sound, T-UXF-03)
  bool bIsHeavy, bIsParryCounter

UHealthComponent
  MaxHealth / CurrentHealth (init from the owner's definition asset)
  ApplyHit(const FCombatHit&) -> float applied damage
  Armor value hook (reduced while State.Combat.ArmorBroken is active)
  OnDamaged(FCombatHit, NewHealth), OnDeath(FCombatHit)

UCombatStateComponent
  Poise (max, current, regen delay, regen rate) — from definition
  ApplyPoiseDamage(float, Instigator) -> breaks to State.Combat.Staggered
  ApplyState(Tag, Duration, Instigator), RemoveState(Tag), HasState(Tag)
  Active states stored as tag + expiry (game time); one timer, no Tick
  OnStateAdded(Tag, Instigator), OnStateRemoved(Tag)
```

Contract additions agreed during feature planning (owners in brackets):

| Type | Where | Purpose |
|---|---|---|
| `UCombatLibrary::DeliverHit(Target, FCombatHit)` | `Combat/` (T-CMB-04) | **Single entry point for every hit** (hero, enemy, soldier, tower, boss). Applies i-frames, block, parry, armor, poise and states in a fixed order. Calling `UHealthComponent::ApplyHit` directly is a bug. |
| `ICombatHitInterceptor` | `Combat/` (T-CMB-04) | Lets a target modify or reject a hit (hero block/parry, future enemy block). |
| `UMeleeTraceComponent` | `Combat/` (T-CMB-04) | Owner-agnostic hit-window trace, one hit per target per swing. Used by hero, enemies, melee soldiers, boss. |
| `ACombatProjectile` | `Combat/` (T-SQD-10) | Base projectile delivering hits via `DeliverHit`. Archer arrows (P1) and tower projectiles (T-DEF-09) derive from it. |
| `FCombatStateConfig` + `BaseArmor` | embedded in `UEnemyArchetypeDefinition`, `USquadDefinition`, `UBossDefinition`, hero class data (T-SYN-01) | Poise max/regen delay/regen rate, Staggered duration, base armor per definition. |
| `FStateDamageMultipliers` | `Combat/` (T-SYN-07) | Optional per-attacker multipliers vs Staggered / Armor Broken / Marked targets. |
| `IInteractable` | `Core/Interactable.h` (T-CMB-12) | Interact verb; first consumer `ABuildZone` (T-DEF-07). |
| `UStatModifierSubsystem` (`AddModifier`, `RemoveModifier`, `GetStatFor`) | `Perks/` (T-PRK-02) | Actor-scoped stat modifiers keyed by `Stat.*` tags; zone bonuses (P2) and perks (P3). |

Team affiliation uses UE's `IGenericTeamAgentInterface` (`FGenericTeamId`: 0 Player/Ally, 1 Enemy). No custom faction system.

## 8. C++ / Blueprint Boundary

| C++ | Blueprint |
|---|---|
| Combat action state machine, hit resolution, stamina, poise, states | Montage selection per definition, AnimBP wiring, hit VFX/SFX choice |
| Squad FSM, formation slots, target priority, leash, stuck recovery | Squad/soldier BP children, mesh, animation |
| Enemy brain, lane following, structure attack | Enemy BP children per archetype |
| Lane layer, break-cost search, placement validation | Lane/zone authoring in levels |
| Director solver, run state machine, perk effects | Data assets, widget layout, feedback rows |

Blueprint-exposed API rules: `BlueprintReadOnly` by default; `BlueprintCallable` only for intentional entry points; `BlueprintImplementableEvent` for presentation hooks (`OnHitPresentation`, `OnStateVisualChanged`).

## 9. Data Architecture

All definitions derive from `UGameDefinition` (T-FND-07). Primary Asset Type name = class name without `U`; this list is canonical and must not change once VS saves exist.

| Definition (Primary Data Asset) | Primary Asset Type | Feature | Key data |
|---|---|---|---|
| `UHeroClassDefinition` | `HeroClassDefinition` | CMB | movement, stamina costs/regen, attack chain, dodge/block/parry windows, poise damage |
| `UEnemyArchetypeDefinition` | `EnemyArchetypeDefinition` | ENM | archetype tag, stats, armor, poise, threat cost, aggro radius, leash, structure preference |
| `USquadDefinition` | `SquadDefinition` | SQD | soldier class, count, formation slots, target priority rules, leash radius, stats |
| `UStructureDefinition` | `StructureDefinition` | DEF | role tag, HP, footprint cells, cost, weapon data, nav behavior |
| `UTacticalZoneDefinition` | `TacticalZoneDefinition` | ZON | zone tag, suggested commands per squad type, bonuses |
| `UWaveDefinition` | `WaveDefinition` | DIR | budget or authored list, allowed archetypes, lane caps, modifiers |
| `URunDefinition` | `RunDefinition` | RUN | step sequence (prep, waves, intermissions, perk offers, event, boss), timings |
| `UPerkDefinition` | `PerkDefinition` | PRK | category tags, weight, instanced effects |
| `UBossDefinition` | `BossDefinition` | BOS | phases, thresholds, attacks, layer tests |
| VS: `UConversionRecipeDefinition`, `UMetaUnlockDefinition`, `UBiomeDefinition`, `USiegeSiteDefinition` | same rule (`ConversionRecipeDefinition`, …) | CNV, MET, WLD | see feature plans |

- Register each type with the Asset Manager (Primary Asset Types in Project Settings) so they get stable `FPrimaryAssetId`s for future save data (D-14).
- Foundation demonstrates registration with the existing concrete `UTestGameDefinition`, type `TestGameDefinition`, and `DA_FoundationSmoke` under `Maps/Test/Definitions`. This type is editor-only, `NeverCook`, and outside gameplay/save contracts. The integration spec discovers its ID/path and validates its content; missing required-field validation uses a transient asset. Gameplay types register when their own classes land.
- Hard references inside definitions are fine in prototype; switch large meshes/sounds to soft references at VS when load profiles show a need.
- Data validation: implement `IsDataValid` on definitions with required fields (cheap, catches broken data in editor).

## 10. Communication

- Owner → owned: direct calls.
- State change → observers: dynamic multicast delegates on the owning component/actor (`OnDamaged`, `OnStateAdded`, `OnSquadStateChanged`, `OnRunPhaseChanged`, `OnRouteInvalidated`).
- Gameplay → presentation: `UFeedbackSubsystem::Play(FGameplayTag FeedbackTag, const FFeedbackEventContext&)`. Rows in `DT_Feedback` map tag → sound, Niagara system, camera shake, hit stop duration, UI toast. One place to audit the §34.5 feedback contract.
- No global event bus, no "manager of managers".

## 11. AI / Navigation

- Enemies, soldiers: `AAIController` (or none, see ENM/SQD plans) + C++ brain component with an enum state machine. Decision ticks run on a timer at a data-driven interval (default 0.1–0.25 s), not every frame.
- Squads: `ASquad` pathfinds its anchor with the navmesh; soldiers move to formation slots with crowd avoidance (`UCrowdFollowingComponent` via `ADetourCrowdAIController`).
- Strategic lane layer (D-09) in `Navigation/`, owned by DEF.
- Navmesh: Recast, Runtime Generation = Dynamic Modifiers Only; structures carry `NavModifier` areas so only their tiles rebuild.

## 12. UI

- `AHUD` subclass optional; root `WBP_GameHUD` created by `AHeroPlayerController`.
- Widgets bind to delegates when created; no `Tick` bindings for gameplay values unless a value changes every frame (e.g., stamina bar interpolation).
- World-space markers (squad icons, enemy class icons, structure HP) through one marker widget component per actor type, hidden by default, shown by HUD/Tactical Focus rules.

## 13. World / Streaming

- Prototype maps are standalone levels (D-17).
- Siege Site boundary: blocking/push-back volume + warning UI (Q-16 default).
- Controlled Open World technique decided by WLD spike at VS.

## 14. Save / Config

- Phase F–P3: no SaveGame. `UGameTuningSettings` (config) for developer tunables; `UGameUserSettings` for video settings.
- Run-state structs: plain USTRUCTs with IDs (Primary Asset IDs, tags, ints), no UObject pointers (D-14).
- VS: `UMetaSaveGame` with `SaveVersion` int, migration function per version step, backup copy on write (MET).

## 15. Performance

- Tools: Unreal Insights (CPU/GPU/memory traces), `stat unit`, `stat game`, `stat ai`, `stat navigation`.
- Reference PC (recorded 2026-10-04, `T-FND-08`, Q-15). All budgets are measured here in a packaged Development build, never only in the editor.

  | Item | Value |
  |---|---|
  | CPU | Intel Core i5-14600KF (14 cores / 20 threads) |
  | GPU | NVIDIA GeForce RTX 5060 |
  | RAM | 16 GB |
  | OS | Windows 11 Pro |
  | Resolution | 1920x1080, 144 Hz display |
  | Target frame rate | 60 fps (16.7 ms frame) [TUNABLE] working number |

  The owner confirmed this current dev PC as the reference PC on 2026-10-04. Re-check budgets on a representative lower-spec PC before VS.
- How to capture: [profiling-checklist.md](profiling-checklist.md).
- Known hot spots to watch: CharacterMovement per enemy, skeletal animation count, navmesh tile rebuilds, target queries (use spatial queries/overlaps with cached candidate sets, not all-to-all scans), projectile count.

## 16. Testing and Debug

- Automation Spec (`Source/<Game>/Tests/*.spec.cpp`) for pure logic.
- Functional Tests (`AFunctionalTest` Blueprint actors in `Content/<Game>/Maps/Test/`, maps named `FT_<Feature>_<Case>`) for scenario checks. They run as `Project.Functional Tests.CastleDefender.Maps.Test.<Map>.<ActorLabel>`. Blueprint only: `FunctionalTesting` is a Developer module, so a C++ `AFunctionalTest` subclass would break Shipping builds of the single runtime module (D-01).
- CLI: `Tools/run_tests.ps1` runs both groups headless (`-NullRHI`) and reads `Saved/Automation/CLI/index.json`, because the editor exit code does not reflect test results.
- Visual Logger for AI decisions and paths. Convention: the VLog category is the domain log category (`LogGameAI`, `LogGameArmy`, ...), and every AI decision logs its state name: `UE_VLOG(this, LogGameAI, Log, TEXT("State %s -> %s"), ...)`.
- CVars under `game.debug.*` (`Combat`, `AI`, `Army`, `Lanes`, `Director`, `Feedback` (added by T-UXF-01), `CombatStates` (T-SYN-01), `CombatTrace` (T-CMB-21), `Enemy` (T-ENM-02); declared in `Core/GameDebug.h`, flagged cheat) toggling debug draw.
- `UGameCheatManager` commands: spawn enemy/squad/wave, set stamina infinite, kill hero, damage Core, skip phase.

## 17. Source Control

- Git + Git LFS. Track `*.uasset`, `*.umap`, source art formats (`*.fbx`, `*.png`, `*.wav`, …) with LFS.
- `.gitignore`: `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `*.sln`, `.codegraph/`.
- Work on one branch, the active `feat/<NN-feature>` branch, for example `feat/01-hero-combat`; keep commits small and scoped to one task, with multiple commits per task when needed. Dependency tasks from other features are committed on that same branch. Do not create per-task or per-dependency branches, or commit feature work directly on `main`. Create the PR into `main` only after all feature tasks are Done and verified (AGENTS.md section 8).

## 18. Risks / Decisions Log

| ID | Item | Status |
|---|---|---|
| D-04 | No GAS in prototype | Review at G3 |
| D-08 | C++ FSM for all prototype AI | Review boss AI at VS |
| D-09 | Lane layer + navmesh hybrid | Pending spike `T-DEF-01` |
| D-11 | No CommonUI in prototype | Review at VS |
| R-F1 | Character-based enemies may not scale | Benchmark in DEF `PERF` task |

| D-07 | Conditional change: if spike T-WLD-01 chooses seamless Siege Site entry inside the open world, `ARunGameMode` can no longer own run state; move run state to a run-scoped actor/component | Open until VS |
| Q-04 | `MaxConcurrentEnemies` is written by benchmark T-DEF-12; copy the result into master plan Q-04 | Pending P2 |

New Gameplay Tag roots or changes to D-xx decisions are recorded here with date and reason.

| Date | Change | Reason |
|---|---|---|
| 2026-10-02 | New tag root `Lane.*` | Lane identity for routes, spawners, forecast (DEF, DIR) |
| 2026-10-02 | New tag roots `Resource.*`, `Tutorial.Gate.*` (VS) | Economy resources (ECO, CNV), tutorial gating (ONB) |
| 2026-10-02 | `UGameTuningSettings` gains `StateDefaultDurations`, `ArmorBrokenArmorMultiplier`, combat state presentation table ref | SYN tunables |
| 2026-10-02 | Cheat `SpawnEnemy <Archetype> <Count> [Lane]` | ENM/DIR testing (extends T-FND-09) |

## 19. Requirement Coverage

| Requirement | Technical area | Task |
|---|---|---|
| R-FND-01 UE5, PC, single-player | §1, §3 | T-FND-01 |
| R-FND-02 C++ core, Blueprint content | §8 | T-FND-03 |
| R-FND-03 Content as data | §9 | T-FND-07 |
| R-FND-04 [TUNABLE] values in data | §9, `UGameTuningSettings` | T-FND-07 |
| R-FND-05 One owner per §34.2 state | §6, master plan D-07 | Gate reviews (FND regression checklist) |
| R-FND-06 Action-based input, gamepad later | §3, master plan D-12 | T-FND-06 |
| R-FND-07 Profile packaged builds on reference PC | §15 | T-FND-08 |
| R-FND-08 Every task verifiable | §16 | T-FND-10 |
| R-FND-09 No generated folders in git | §17 | T-FND-02 |
