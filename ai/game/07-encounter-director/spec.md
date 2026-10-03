# Encounter Director (DIR): Specification

| | |
|---|---|
| Feature code | DIR |
| GDD sections covered | §7 (Threat Forecast), §8 (Encounter Director), §25 (Difficulty, hooks only), §27.2 (allowed procedural variation), §31.2 (concurrent cap), §36 Full Run DoD ("Director never generates impossible waves"). Cross-refs: §2 pacing principle, §5.3, §6.2–6.3, §20, §22, §27.1, §28.1, §32 P2/P3, §33, §34.2, §34.4 |
| Phases | P2 (authored 3-wave spawner, A-08) → P3 (Director, constraints, modifiers, Threat Forecast, boss wave, difficulty hooks) |
| Master plan refs | D-06, D-07, D-10, D-14, D-15, D-16; A-08; Q-04, Q-17 |
| Status | Draft v1 |

## 1. Overview

DIR decides **what** attacks, **where** and **when**, and tells the player before it happens.

- P2: authored waves (`UWaveDefinition`) spawned at authored lane spawn points, within the concurrent enemy cap, with wave lifecycle events for RUN.
- P3: the Encounter Director builds each wave from a threat budget under the §8.2 constraints, applies modifiers, publishes a Threat Forecast into `ARunGameState`, runs a curated boss wave, and exposes difficulty hooks that are not HP sponges.

RUN owns the run sequence and calls the Director (prepare wave → forecast, start wave, stop). ENM owns enemy behavior after spawn. DEF owns lanes and the concurrent cap value.

## 2. Player Experience

- "I saw it coming." Before every wave the player reads the threat in 1–2 s: Swarm High, Armored Medium, coming Left; Armored Legion active (§7.1, §28.1).
- Losing feels like a wrong call or bad execution, never a hidden surprise (§7.1).
- Pressure escalates: early waves use one lane and few types; mid-run waves split pressure across lanes; late waves erode defenses; the boss tests everything (§2 pacing principle, §33).
- Runs differ: compositions and modifiers vary inside authored bounds, so the same Siege Site asks new questions (§27.2).
- Harder difficulty feels smarter, not spongier (§25.1).

## 3. Core Loop

```text
Wave cleared → Director plans next wave (budget + constraints + modifiers, seeded)
→ Forecast published → Player prepares (build, assign squads, perks)
→ Minimum warning time met → Wave spawns in pulses on lanes, within caps
→ Enemies removed → Wave cleared → repeat → curated Boss wave
```

## 4. Gameplay Rules

### P2 scope: authored waves

- **R-DIR-01 (A-08, §32 P2)**: P2 runs 3 authored waves (`DA_Wave_P2_01..03`) without the Director solver. The same `UWaveDefinition` type later feeds the Director.
- **R-DIR-02 (§5.3) [LOCKED]**: Enemies enter only from authored lane spawn points. Each spawn point belongs to exactly one lane (`Lane.*` tag). The boss entry is a separate authored spawn point.
- **R-DIR-03 (§31.2, Q-04) [LOCKED]**: Spawning never exceeds the concurrent enemy cap (`UGameTuningSettings::MaxConcurrentEnemies`, set by DEF `T-DEF-12`; placeholder 40). Spawns beyond the cap wait in a queue. Spawns per tick are limited (`MaxSpawnsPerTick`, default 4) to avoid hitches [TUNABLE].
- **R-DIR-04 (§34.2, D-07)**: Wave State (current wave, plan, queue, alive enemies per lane) is owned by `UEncounterDirectorComponent` on `ARunGameMode` and mirrored to `ARunGameState` for UI.
- **R-DIR-05 (Assumption, answers NEW-RUN-1)**: A wave is **cleared** when every planned spawn has spawned and no enemy of that wave remains (killed, despawned or out of world). Waves have no timer and no fail state.
- **R-DIR-06 (§6.3 Collect, derived)**: The Director reports every removal of a wave enemy with its archetype, lane and cause (killed, despawned, out of world) so RUN can grant kill rewards and stats.
- **R-DIR-07 (§36 "no serious blocker bug", NEW-DIR-01)**: If a wave makes no progress for `WaveStallTimeoutSeconds` with only a few enemies left, the Director logs an error and removes the stragglers so the run cannot hang. This is a bug guard, not a design rule.

### P3 scope: Director

- **R-DIR-10 (§8.1) [LOCKED]**: Waves are never fully random. The Director builds each wave from a **Threat Budget** and constraints.
- **R-DIR-11 (§8.1) [TUNABLE]**: Wave budget (per wave definition) and archetype threat cost (`UEnemyArchetypeDefinition::ThreatCost`, ENM) are data. GDD example (budget 100; Swarm 2, Armored 8, Giant/Siege 25) is illustrative only.
- **R-DIR-12 (§8.2) [LOCKED]**: Max concurrent unit budget: no spawn pulse is larger than the cap and the runtime spawner never exceeds it (R-DIR-03).
- **R-DIR-13 (§8.2) [LOCKED]**: Enemy-type cap: each archetype has a max count per wave (wave data × modifier cap multiplier).
- **R-DIR-14 (§8.2) [LOCKED]**: Lane cap: each lane has a max threat share per wave; the wave data sets allowed lanes and a minimum number of lanes.
- **R-DIR-15 (§8.2) [LOCKED]**: Minimum warning time: a wave's forecast is visible for at least `MinWarningSeconds` (default 20 s) before its first spawn. RUN holds the start (`T-RUN-12`); the Director also refuses to spawn early.
- **R-DIR-16 (§8.2) [LOCKED]**: No impossible hard counter vs the current loadout: if the loadout has none of an archetype's counter tags, that archetype's threat share is capped (`MaxShareWithoutCounter`, default 0.25) and none of its elites spawn.
- **R-DIR-17 (§8.2) [LOCKED]**: Elite cap for readability: elites (`Unit.Enemy.Elite`, ENM NEW-ENM-8) are capped per wave and concurrently alive.
- **R-DIR-18 (§8.2) [LOCKED]**: The boss wave is scripted/curated, not procedural: authored boss + authored adds. Boss summons go through the Director spawner and respect the caps (immediate spawn refused when the cap is full; queued requests wait for a free slot). The boss itself always gets a reserved slot.
- **R-DIR-19 (§8.3)**: Modifiers change costs and rules (cost, type cap, weight, budget, lane rules). The forecast must state every active modifier clearly. Example: **Armored Legion** makes Armored cheaper and allows more of them.
- **R-DIR-20 (§36 Full Run DoD) [LOCKED]**: The Director never produces an impossible wave because of a logic bug. Every plan passes a validator; on failure the Director retries with the next seed, then falls back to the wave's authored safe groups and logs an error. Proven with fixed-seed Automation Specs.
- **R-DIR-21 (§27.2) [LOCKED]**: Allowed variation: wave composition, modifier choice, enemy spawn variation inside authored bounds (spawn point among the lane's points, spawn radius, pulse timing jitter). Not varied (§27.1): lane topology, terrain, boss arena, camera-critical choke points. All variation is seeded and reproducible.
- **R-DIR-22 (§2 pacing principle) [LOCKED]**: Escalating tension is authored in wave data: early waves few lanes and types, mid-run multi-lane compositions, late waves heavier Siege, boss tests all layers.
- **R-DIR-23 (§6.2, Q-17 default)**: The mid-run pressure event is the **Split Pressure** modifier applied to W4: at least 2 lanes, each with at least `MinLaneShare` (default 0.35) of the threat, first pulses together. RUN's PressureEvent step passes the modifier tag (R-RUN-14).

### P3 scope: Threat Forecast

- **R-DIR-30 (§7.1) [LOCKED]**: A forecast is shown before every wave, including the boss wave.
- **R-DIR-31 (§7.2) [TUNABLE]**: Forecast content: per enemy type None / Low / Medium / High with icon, lane direction (with per-lane level), elite warning, active modifiers, boss phase hint, unknown slots at higher difficulty. No exact counts. Level thresholds are data (default by threat share: Low ≤ 20%, Medium ≤ 45%, High above).
- **R-DIR-32 (§7.3)**: At higher difficulty the forecast hides detail (unknown type slots) but never becomes pure guess: lanes, modifiers and elite warning are always shown and at least one type stays visible.
- **R-DIR-33 (§7.1, derived)**: The forecast is built from the actual plan. It hides, it never lies. Once published it does not change before the wave starts; the loadout snapshot for R-DIR-16 is taken at planning time.
- **R-DIR-34 (§28.1) [LOCKED]**: The next-wave forecast is readable in 1–2 s.

### P3 scope: difficulty hooks

- **R-DIR-40 (§25.1) [LOCKED]**: Difficulty is not mainly HP sponge. Director hooks (data): budget multiplier (smarter composition), extra minimum lanes (simultaneous lane pressure), earliest elite wave (elites earlier), hidden forecast slots (less accurate forecast), pulse gap multiplier (tighter timing). Siege targeting (ENM data), resource zone attacks (VS), boss variation (BOS) and respawn pressure (CSM) live in other features.
- **R-DIR-41 (§25.2) [TUNABLE]**: Stat scaling is support only: health/damage multipliers in the difficulty definition are capped by validation (`MaxStatScale`, default 1.25) and stay 1.0 in the prototype (NEW-DIR-05).

## 5. Player Actions

DIR has no direct input. The player:

| Action | Phase |
|---|---|
| Reads the forecast panel during Prep/Intermission and the compact forecast on the HUD | P3 |
| Prepares based on it (build, squads, perks) | P3 |
| Reads lane-incoming and elite-spawn cues during the wave | P2 (lane), P3 (elite) |

## 6. Success / Failure

- Feature success: every wave of a run is planned, forecast, spawned and cleared with no constraint violation (G3 checks).
- Partial: under-budget waves are valid (weaker, never impossible).
- Failure handling: invalid plan → retry → authored fallback (R-DIR-20); stalled wave → straggler cleanup (R-DIR-07).

## 7. Scope

### In Scope
- `UWaveDefinition`, `AEnemySpawnPoint`, spawner with caps, wave lifecycle events, mirror into `ARunGameState`.
- Threat budget solver, constraints, validator, fallback, modifiers, Split Pressure event modifier.
- Threat Forecast generation and Forecast UI.
- Curated boss wave hook and scripted spawn requests.
- `UEncounterDifficultyDefinition` hooks.
- P2 and P3 wave content (`DA_Wave_P2_01..03`, `DA_Wave_P3_01..05`, `DA_Wave_P3_Boss`), modifiers.
- Cheats, debug overlay, Specs, Functional Tests, G3 Director/Forecast checks.

### Out of Scope
- Run sequence, warning-time gate holding, wave start/clear banners (RUN).
- Enemy behavior, archetype stats (ENM); boss behavior and reset (BOS).
- Flying enemies (§20.2 [DEFERRED]), biome special enemies (§20.3), Endless/Daily/seed sharing (§26.2 [DEFERRED]).
- Reward, weather, resource node and optional encounter variation (§27.2 lists them; owned by RUN/ECO/WLD later).
- Ascension content (§25.3, Q-11).

## 8. Anti-Goals

- No fully random waves (§8.1).
- No HP-sponge difficulty (§25.1).
- No forecast that lies or turns into pure guess (§7.3).
- No adaptive rubber-banding based on how well the player is doing (not in GDD).
- No exact unit counts or spreadsheet-style forecast (§7.2).
- No procedural lanes, terrain or boss arenas (§27.1).

## 9. Dependencies

| Needs | From | For |
|---|---|---|
| `UEnemyArchetypeDefinition` (threat cost, tags incl. `Unit.Enemy.Elite`, enemy class), `InitFromSpawn`, `FEnemySpawnParams`, `OnEnemyRemoved` | ENM `T-ENM-01` (+ archetypes `T-ENM-05/06/10`) | Spawning, solver input |
| Lane tags, `SelectRoute`, route validity, structure registry | DEF `T-DEF-04/05` | Lane per spawn, loadout snapshot |
| `MaxConcurrentEnemies` value | DEF `T-DEF-12` | Cap |
| `L_SiegeSite_Proto` | DEF `T-DEF-13` | Spawn point placement |
| `ARunGameMode`, `ARunGameState`, `URunDefinition` wave refs, PressureEvent tag | RUN `T-RUN-01`, `T-RUN-03`, `T-RUN-12` | Host, mirror, calls |
| Squad definitions / tags | SQD `T-SQD-01` | Loadout snapshot |
| Boss class/definition | BOS `T-BOS-01`, `T-BOS-05` | Boss wave |
| Feedback subsystem, HUD shell, lane danger indicator | UXF `T-UXF-01, 02, 07` | Feedback, forecast HUD |
| Tag roots, settings, data pattern, cheats, test harness | FND `T-FND-04, 07, 09, 10` | Everything |

Consumers: RUN (wave events, forecast timestamp, kills), UXF (lane danger from alive-per-lane), BOS (scripted spawns), TFM (lane pressure in overlay).

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Budget cannot be fully spent because of caps | Valid under-budget plan; warning logged if > 10% unspent |
| No allowed archetype affordable or all capped to 0 | Validator fails → retry → authored fallback + error |
| Loadout lacks counters for every allowed archetype | Shares capped; plan may be under budget; never empty (fallback) |
| Modifier makes a cost ≤ 0 | Blocked by `IsDataValid` |
| Concurrency cap lower than a pulse | Spawner trickles the pulse as slots free up |
| Spawn point blocked (crowd, collision) | Retry next tick at another point in radius; after 3 failures use another spawn point of the same lane |
| Lane has no valid route (DEF) | Group moves to the nearest allowed lane, error logged |
| Enemy removed without death (out of world) | Counts as removed for clearing; removal event with cause OutOfWorld (no kill reward) |
| Hero dead (Commander Spirit) | Director continues unchanged |
| Tactical Focus time dilation | All Director timers use game time; spawning slows with dilation (consistent with RUN) |
| RUN calls start before warning time | Director logs a warning and delays the first spawn until met |
| Player builds after the forecast | Plan and forecast stay as published (R-DIR-33) |
| Boss removed without being defeated | Director tells BOS (reset hook `T-BOS-05`); boss wave stays active |
| Unknown modifier tag from RUN | Logged and ignored; wave planned without it |
| Same seed replay | Identical plans for the same inputs |

## 11. Acceptance Criteria

### P2
- **AC-DIR-01**: `DA_Wave_P2_01..03` spawn their authored groups at the authored lanes in `L_SiegeSite_Proto`.
- **AC-DIR-02**: With the cap set to 10 and a 30-enemy wave, alive count never exceeds 10 (Functional Test samples on every spawn).
- **AC-DIR-03**: `OnWaveCleared` fires exactly once, within 1 s after the last enemy of the wave is removed, and never before all spawns are done.
- **AC-DIR-04**: `OnWaveEnemyRemoved` fires once per removed enemy with archetype, lane and cause; cause = Killed only for deaths.
- **AC-DIR-05**: `StopAndClear` removes all wave enemies and empties the queue; no further spawns.
- **AC-DIR-06**: Stall guard: one enemy made unkillable → after the timeout an error is logged and the wave clears.
- **AC-DIR-07**: Cheats `Encounter.StartWave`, `Encounter.ClearWave`, `Encounter.Seed`; `game.debug.Encounter 1` shows queue, alive per lane, cap, current plan.

### P3
- **AC-DIR-10**: Same seed + same inputs → identical plan (Spec).
- **AC-DIR-11**: 1000 fixed seeds × every P3 wave definition × 3 loadout snapshots → 0 validator failures, 0 empty plans, fallback never used (Spec).
- **AC-DIR-12**: Budget never exceeded in any plan.
- **AC-DIR-13**: Type, lane and elite caps respected in every plan; concurrently alive elites never exceed the cap (Functional Test).
- **AC-DIR-14**: Loadout with no Armored counter → Armored threat share ≤ `MaxShareWithoutCounter` and no uncountered elites.
- **AC-DIR-15**: Armored Legion raises the mean Armored share over 200 seeds vs no modifier; forecast names "Armored Legion" with its description.
- **AC-DIR-16**: W4 with Split Pressure: both lanes get ≥ `MinLaneShare` of threat; first pulses on both lanes start within 5 s of each other.
- **AC-DIR-17**: Telemetry over a full run: every wave's first spawn is ≥ `MinWarningSeconds` after its forecast was published.
- **AC-DIR-18**: Forecast levels follow the thresholds; with 1 hidden slot exactly one type shows "?"; lanes, modifiers and elite warning always visible.
- **AC-DIR-19**: Forecast content is identical at publication and at wave start, even if the player builds in between.
- **AC-DIR-20**: Boss wave: boss spawns at the boss entry with only authored adds; scripted summons respect the cap; forecast shows the boss hint.
- **AC-DIR-21**: G3 playtest: testers name the main threat and its lane within 2 s of opening the forecast.
- **AC-DIR-22**: A difficulty definition with a stat multiplier above `MaxStatScale` fails `IsDataValid`.
- **AC-DIR-23**: G3 checklist items "Threat Forecast works before every wave" and "Director never generates an impossible wave" pass.

## 12. Open Questions / Assumptions

| ID | Item | Default until answered |
|---|---|---|
| A-08 | P2 waves are authored | R-DIR-01 |
| Q-04 | Concurrent enemy cap | Placeholder 40 until `T-DEF-12` |
| Q-17 | Mid-run event | Split Pressure modifier on W4 (R-DIR-23) |
| NEW-RUN-1 | Wave end condition (asked by RUN) | R-DIR-05 |
| NEW-DIR-01 | Stall guard removes stragglers (bug guard) | On, `WaveStallTimeoutSeconds` 90 s, ≤ 3 enemies left, logs Error |
| NEW-DIR-02 | Hard-counter data: `CounterTags` (any-of) on `UEnemyArchetypeDefinition` (ENM-owned type) | Added by `T-DIR-04` after agreeing with ENM; Swarm: Bombard / Infantry; Armored and Siege: Ballista / `State.Combat.ArmorBroken` |
| NEW-DIR-03 | Do perks add loadout counter tags? | No in prototype |
| NEW-DIR-04 | Which archetypes are elite? (ENM NEW-ENM-8) | Giant/Siege via `Unit.Enemy.Elite` |
| NEW-DIR-05 | How is stat scaling applied (needs spawn multipliers in ENM)? | Not applied; fields validated only |
| NEW-DIR-06 | Forecast "confidence" beyond unknown slots (fuzzed levels)? | No; unknown slots only |

New leaf tags (existing roots): `Modifier.Encounter.ArmoredLegion`, `Modifier.Encounter.SplitPressure`, `Feedback.Encounter.ForecastUpdated`, `Feedback.Encounter.LaneIncoming`, `Feedback.Encounter.EliteSpawned`. Lane tags use the `Lane.*` root requested by DEF.

## 13. System Contract

| Item | Content |
|---|---|
| Responsibility | Turn wave data into spawned enemies on lanes, under caps; in P3 plan waves from budget + constraints + modifiers and publish a truthful forecast; report wave lifecycle |
| Inputs | Calls from RUN: `PrepareWave(Wave, WaveNumber, ModifierTags)`, `StartPreparedWave()`, `StopAndClear()`; `UWaveDefinition`, `UEncounterModifierDefinition`, `UEncounterDifficultyDefinition`, `UEnemyArchetypeDefinition` data; `MaxConcurrentEnemies`, `MinWarningSeconds`; loadout snapshot (DEF structure type tags, SQD squad tags, difficulty base tags); lane validity (DEF); run seed; scripted spawn requests (BOS): `TrySpawnEnemy` returns null when the cap is full, `RequestScriptedSpawn` queues until a slot frees |
| Outputs | Spawned enemies (`InitFromSpawn` with lane route); `FWavePlan`; `FThreatForecast` in `ARunGameState`; wave state mirror (wave number, state, alive total and per lane); events below |
| State | Run seed; current `FWavePlan`; `EWaveState` (Idle, Planned, Spawning, Active, Cleared, Stopped); spawn queue; alive set per lane; alive elites; forecast publication time; stall timer |
| Events | `OnWavePrepared(WaveNumber)`, `OnWaveStarted(WaveNumber)`, `OnWaveCleared(WaveNumber)`, `OnEnemySpawned(Enemy, Lane)`, `OnWaveEnemyRemoved(Enemy, Archetype, Lane, Cause)`, `OnAliveCountChanged`, `OnBossSpawned(Boss)`; `ARunGameState::OnForecastChanged` |
| Performance | Solver + validator once per wave (target < 1 ms, Spec timing log); spawner on a 0.1 s timer, ≤ `MaxSpawnsPerTick`; no Tick; pooling only if `T-DEF-12` shows spawn churn (D-15) |

### Data model (design level)

| Definition | Fields |
|---|---|
| `UWaveDefinition` (`DA_Wave_*`) | Mode (Authored / Budget / Boss); AuthoredGroups [archetype, count, lane, start time, interval]; Budget: BaseBudget, Archetypes [archetype, weight, min count, max count], LaneRules (allowed lanes, min lanes, max lane share), ModifierPool [modifier, weight] + ModifierChance, SpawnWindowSeconds, PulseMaxSize, PulseJitterSeconds; FallbackGroups (required for Budget mode); Boss: BossDefinition, BossSpawnTag, BossForecastHint |
| `UEncounterModifierDefinition` (`DA_Modifier_*`) | Tag (`Modifier.Encounter.*`), DisplayName, ForecastText, Icon, BudgetMultiplier, ArchetypeRules [archetype tag, cost ×, cap ×, weight ×], LaneRuleOverride (min lanes, min lane share, simultaneous first pulses) |
| `UEncounterDifficultyDefinition` (`DA_Difficulty_*`) | BudgetMultiplier, EliteFirstWave, MaxElitesPerWave, MaxConcurrentElites, ExtraMinLanes, ForecastHiddenTypeSlots, PulseGapMultiplier, MaxShareWithoutCounter, BaseLoadoutTags, HealthMultiplier, DamageMultiplier (≤ MaxStatScale) |
| `UEnemyArchetypeDefinition` (ENM) fields read | ThreatCost, archetype tags incl. `Unit.Enemy.Elite`, EnemyClass, CounterTags (NEW-DIR-02) |
| `UGameTuningSettings` (Encounter group) | MaxConcurrentEnemies (DEF writes), MaxSpawnsPerTick, MinWarningSeconds, WaveStallTimeoutSeconds, StallMaxAlive, ForecastLowShare, ForecastMediumShare, MaxStatScale |
| `AEnemySpawnPoint` (level actor) | LaneTag, SpawnRadius, Weight, bBossEntry |
| `FWavePlan` (runtime, plain struct, D-14) | WaveNumber, Seed, Budget, Spent, ModifierTags, Groups [archetype `FPrimaryAssetId`, count, lane tag, pulse time], bIsFallback |
| `FThreatForecast` (runtime) | WaveNumber, bBossWave, Types [type tag, level or Unknown], Lanes [lane tag, level], bEliteWarning, Modifiers [tag, name, text], BossHint, UnknownSlots |

### Failure cases (GDD §34.4 and Director-specific)

| Case | Handling |
|---|---|
| Hero death | No change; spawning continues |
| Squad wipe | No change |
| Core critical HP | No change (no rubber-banding) |
| Player leaving Siege Site | No change (RUN boundary rule) |
| Boss reset / bug recovery | Boss removed without defeat → `OnBossRemovedUnexpectedly` for BOS `T-BOS-05`; boss wave not cleared |
| Invalid plan | Retry with seed + n (up to 8), then authored fallback + error (R-DIR-20) |
| Stalled wave | Straggler cleanup + error (R-DIR-07) |
| Missing data (null archetype, no spawn point for a lane) | `IsDataValid` / BeginPlay error; group skipped and logged, never a crash |

## 14. Feedback Contract

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Forecast published (P3) | — | Soft cue `Feedback.Encounter.ForecastUpdated` | Forecast panel opens; compact forecast on HUD | Main threat + lane readable in 1–2 s |
| Modifier active (P3) | — | — | Modifier name + one-line effect in panel and HUD chip | Always shown, never hidden by difficulty |
| Unknown slot (P3, higher difficulty) | — | — | "?" type icon | Clearly marked as unknown, not as "None" |
| Lane first spawn of a wave | Lane arrow flash at the lane entry | `Feedback.Encounter.LaneIncoming` | Lane danger indicator (UXF `T-UXF-07`) | Player knows which lane activates even off-screen |
| Elite spawned (P3) | Elite marker on the enemy (UXF) | `Feedback.Encounter.EliteSpawned` | — | Elite distinguishable from normal enemies (§28.1) |
| Wave start / cleared | — | RUN `Feedback.Run.WaveStart` / `WaveCleared` | RUN banners | Not duplicated by DIR |
| Pressure event / boss incoming | — | RUN `Feedback.Run.PressureEvent` / `BossIncoming` | RUN banners; forecast highlights the modifier / boss hint | Visible ≥ minimum warning time |
| Spawn held by cap | — | — | Debug overlay only | Never shown to players |
