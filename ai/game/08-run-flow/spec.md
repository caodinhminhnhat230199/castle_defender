# Run Flow (RUN): Specification

| | |
|---|---|
| Feature | RUN, folder `08-run-flow` |
| GDD sections covered | §2 (pacing principle), §4.1, §5.3, §5.5, §6.1–6.3, §7.1, §8.2 (warning time), §16.3, §18.1, §23.4 (perk step hook only), §28.1, §28.3, §30, §32 P2/P3, §33, §34.2, §34.4, §36 Full Run DoD |
| Phases | P2 (minimal: 3 authored waves, Core loss, build allowance) → P3 (full run) |
| Status | Draft v1 |
| Related | DIR (waves, forecast), DEF (Core, placement), PRK (perk offers), BOS (boss), CSM (hero death), TFM (Focus), UXF (HUD, feedback, telemetry) |

## 1. Overview

RUN owns one Siege Run: the ~25-minute match on one Siege Site, from Initial Prep to Resolve. It reads an ordered step list from a `URunDefinition` data asset, tells the Encounter Director when to forecast and start each wave, hosts the perk step, owns the single run resource, decides win and loss (Core destroyed = run lost; Hero death never loses), enforces the Siege Site boundary and records pacing data. RUN is the "Run State" owner from GDD §34.2.

Player value: a run with a clear rhythm (prepare, fight, choose), escalating to a boss, with an end screen that explains the result and makes an immediate retry easy (§33 Resolve, G3 question).

## 2. Player Experience

- Always know the current phase, the time to the next wave and what is coming (forecast, §7.1, §28.1).
- Escalating tension (§2 pacing principle): few lanes and enemy types early, split-lane pressure mid-run, defense erosion late, a boss that tests every earlier decision.
- Fair failure: only the Core falling loses the run. Dying as the Hero costs time and tempo, not the run (§18.1).
- An ending with a reason: the resolve screen says why you won or lost and invites a different build (§33 Resolve, §32 P3 gate).

## 3. Core Loop

```text
Forecast → Prepare → Defend/Fight → Collect → Choose → Reconfigure → (next wave) … → Boss → Resolve
```

| Loop step (§6.3) | Where it happens in the run | What RUN does | Content owner |
|---|---|---|---|
| Forecast | Start of every pre-wave window (run start for W1, each wave clear for the next) | Requests the next forecast, publishes it in `ARunGameState` | DIR (T-DIR-06, T-DIR-07) |
| Prepare | Prep (~2 min), Intermission | Timed window; resets build allowance (P2) | DEF placement, SQD orders |
| Defend/Fight | Wave, Boss | Starts the wave; waits for clear or boss defeat; watches the Core | DIR, ENM, BOS |
| Collect | Moment of wave clear | Grants the wave-clear reward (P3); kill rewards accrue during the wave | RUN |
| Choose | PerkChoice step after W1, W3, W4 (A-07) | Requests a 1-of-3 offer; waits for the pick | PRK |
| Reconfigure | Intermission, PressureEvent window | Timed window; spending allowed | DEF, SQD |

## 4. Gameplay Rules

### P2 scope

- **R-RUN-01** (§4.1, §5.3) [LOCKED]: A run is one Siege Run on one Siege Site map. It starts when the map loads with `ARunGameMode` and ends at Resolve. One run per map load.
- **R-RUN-02** (§6.2, §34.1) [LOCKED]: The run follows an ordered step list from a `URunDefinition`. Step types: Prep, Wave, Intermission, PerkChoice, PressureEvent, Boss, Resolve. Steps run strictly in order. No branching.
- **R-RUN-03** (§32 P2, A-08) [LOCKED for P2]: The P2 definition is Prep → Wave 1 → Intermission → Wave 2 → Intermission → Wave 3 → Resolve, using authored wave data.
- **R-RUN-04** (§6.1, §6.2) [TUNABLE]: Timed steps (Prep, Intermission, PressureEvent, Boss lead-in) last a per-step duration from the run definition. Prep default 120 s (GDD "~2 phút"). Other defaults in §13.
- **R-RUN-05** (§6.3; Assumption NEW-RUN-1): A Wave step ends when the Director reports the wave cleared. "Cleared" is defined by DIR (T-DIR-02). Waves have no fail timer.
- **R-RUN-06** (§18.1) [LOCKED]: The run is lost only when the Core is destroyed, or when a special objective fails (the prototype has none). Hero death never ends or pauses the run. Step timers keep running while the Hero is dead.
- **R-RUN-07** (§32 P2): P2 win = Wave 3 cleared with the Core alive. P2 has no boss.
- **R-RUN-08** (§18.1, §34.4): The run result latches once. After the latch: no further steps, spawning stops, remaining enemies are removed, rewards and spending are rejected, later events (a second Core hit, a boss death) are ignored.
- **R-RUN-09** (§16.3, A-05) [TUNABLE]: P2 building is free. The start of every Prep and Intermission resets a build allowance to `BuildsPerWindow` (placeholder 3, no GDD value; NEW-RUN-5). Placing one structure uses 1 allowance. At 0, placement is refused with feedback. Whether placement is allowed during waves is a DEF rule.
- **R-RUN-10** (§34.2, D-07) [LOCKED]: Run rules live in `ARunGameMode`. Run data that observers read lives in `ARunGameState`. Per-player run data (stats, revive charges, perks) lives in `ARunPlayerState` and survives Hero death.
- **R-RUN-11** (§30 [OPEN], D-14, A-11): No mid-run save in any prototype. All RUN-owned state is held in plain structs with stable IDs (Primary Asset IDs, enums, ints), never raw UObject pointers, so a later run save stays possible.

### P3 scope

- **R-RUN-12** (§6.2) [LOCKED]: Full sequence: Prep (~2 min) → W1 → Intermission → W2 → Intermission → W3 → Pressure event → W4 → Intermission → W5 → Boss → Resolve. Baseline is 5 normal waves + 1 boss. PerkChoice steps follow W1, W3 and W4 (A-07, data).
- **R-RUN-13** (§7.1, §8.2) [LOCKED]: The forecast for the next combat step (wave or boss) is requested from the Director when the pre-wave window opens: at run start for W1, on each wave clear for the next one. It stays visible through PerkChoice and Intermission. A wave never starts before its forecast has been visible for the Director's minimum warning time. If a window is shorter, RUN holds the wave start until that time is reached and logs a warning.
- **R-RUN-14** (§6.2, Q-17 default) [OPEN → Q-17]: The mid-run pressure/event is one PressureEvent step between W3 and W4. It announces a Director modifier (default: split-lane pressure) that applies to W4 and is shown in W4's forecast. Its timed window is also W4's prep time. Event content beyond the modifier tag belongs to DIR.
- **R-RUN-15** (§23.4, A-07) [LOCKED/TUNABLE]: A PerkChoice step asks PRK for a 1-of-3 offer and waits until the player picks. No time limit in the prototype. World time keeps running (no full pause). No enemies are active because the wave is cleared.
- **R-RUN-16** (§6.2, §8.2, §22) [LOCKED]: The Boss step follows W5. It has a lead-in window (default 20 s, never shorter than the minimum warning time) showing the boss forecast, then runs the curated boss wave (DIR T-DIR-08). The step ends when BOS reports the boss defeated. Boss defeated = run won.
- **R-RUN-17** (§16.3 [LOCKED], A-05) [values TUNABLE]: P3 uses exactly one run resource. Starting amount (§33 01:00 "resource khởi đầu"; placeholder 300), kill reward per enemy unit tag and wave-clear reward per wave (Collect) come from the run definition. It is spent on building (cost from `UStructureDefinition`) and on repair when DEF provides repair. No Food, Gold or Monster Material in prototype.
- **R-RUN-18** (A-05): A spend either succeeds fully or fails with no side effect. It fails with feedback when the amount is not enough or the run is resolved. The resource never goes below 0.
- **R-RUN-19** (§33 Resolve, §24, §5.5): Resolve shows the outcome (won/lost), the reason (Core destroyed at wave N, boss defeated) and a run summary (duration, waves cleared, Hero deaths, resource earned/spent, perks taken). Restart is available at once. Meta rewards (win: reward, rare material, unlock; lose: partial meta reward) are deferred to MET (VS). P3 emits a run result record for MET to consume later and grants nothing persistent.
- **R-RUN-20** (§5.5) [DEFERRED to VS]: Return to the Hub after a run. P3 offers Restart and Quit instead.
- **R-RUN-21** (§34.4, §28.1, §28.3): Core critical: when Core HP falls to or below `CoreCriticalFraction` (placeholder 0.25, no GDD value), the run enters the Core critical state with audio and HUD feedback. It leaves the state when HP rises above the threshold (repair). No gameplay effect.
- **R-RUN-22** (§5.3, §34.4, Q-16 default) [OPEN → Q-16]: The Siege Site has a gameplay boundary. Leaving the inner play area shows a warning. An outer blocking edge stops the Hero (push-back). Leaving never fails the run. If the Hero ends up outside the outer bounds (fall, physics bug), it is moved to the nearest safe point. RUN exposes the site bounds so the Commander Spirit camera stays inside them.
- **R-RUN-23** (§6.1, §33, §36): Pacing instrumentation records, per step, game-time and real-time duration against the step's pacing target, and total run time against `TargetRunSeconds` (default 1500 s = ~25 min, [LOCKED/TUNABLE]). Records go to the playtest telemetry log (T-UXF-08).
- **R-RUN-24** (§2 pacing principle) [LOCKED]: The P3 run definition orders its wave data to escalate: early waves use few lanes and archetypes, mid waves split pressure, late waves pressure structures, the boss tests all three layers. RUN provides the ordered slots; wave content is DIR's.

## 5. Player Actions

| Action | When | Notes |
|---|---|---|
| Read phase, countdown, wave n/5, resource or build allowance | Always | Run status HUD |
| Read the forecast | Pre-wave windows | Panel is DIR T-DIR-07 |
| Build / repair (spend) | Per DEF placement rules | RUN validates the cost |
| Pick 1 of 3 perks | PerkChoice | UI is PRK T-PRK-05 |
| Fight, command, use Focus | Waves, Boss | CMB, SQD, TFM |
| Restart or Quit | Resolve | Restart reloads the Siege Site map |

Developer cheats (Functional Tests and tuning): `RunSkipStep`, `RunCompleteWave`, `DamageCore <amount>`, `GiveRunResource <n>`, `RunJumpToStep <index>`.

## 6. Success / Failure Conditions

| Result | Condition | Notes |
|---|---|---|
| Win (P2) | Wave 3 cleared with Core alive | Resolve stub |
| Win (P3) | Boss defeated | Full resolve |
| Loss | Core destroyed, any step | §18.1 |
| Not a loss | Hero death, leaving the site, squad wipe, structure loss | §18.1, Q-16 |
| Partial success | None in prototype | Partial meta reward is MET (VS) |
| Retry | Restart reloads the map; nothing persists | A-11 |

## 7. Scope

### In Scope
`ARunGameMode`, `ARunGameState`, `ARunPlayerState`, `URunDefinition`, step machine, forecast handshake and warning-time gate, pressure-event step, perk step hook, run resource, P2 build allowance, Core loss, Core critical state, resolve screen with Restart/Quit, run result record, Siege Site boundary, pacing instrumentation, serializable run state structs, cheats, debug overlay, Functional Tests, G3 gate playtest.

### Out of Scope
Meta rewards and save (MET), Hub return (WLD), mid-run save (A-11, Q-10), Food/Gold/Monster Material/villagers (ECO), conversion (CNV), wave composition and forecast content (DIR), perk content (PRK), boss behavior (BOS), the repair interaction itself (DEF), other modes such as Blitz or Endless (§6.2, §26.2 DEFERRED), difficulty and Ascension (launch), "call next wave early" (NEW-RUN-3), pause menu.

## 8. Anti-Goals

- No economy sprawl: one resource, no production chains, no income buildings (§3, §16.3).
- No general timeline or scripting system: the run is a flat list of typed steps, no branching graph.
- No run save in prototype; no save scaffolding beyond plain structs.
- The run is never lost from Hero death.
- No hidden information: a wave never starts without its forecast (§7.1).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Wave spawner, wave cleared / enemy killed events | DIR T-DIR-01, T-DIR-02 | Wave steps, kill rewards |
| Forecast generation, minimum warning time | DIR T-DIR-06, T-DIR-04 | R-RUN-13 |
| Modifiers | DIR T-DIR-05 | R-RUN-14 |
| Curated boss wave | DIR T-DIR-08 | R-RUN-16 |
| `ACoreStructure` with `UHealthComponent` | DEF T-DEF-03 | Loss, Core critical |
| Placement calling the spend API | DEF T-DEF-07 | R-RUN-09, R-RUN-17 |
| Boss defeated event | BOS T-BOS-05 | Win |
| Perk offer + choice UI | PRK T-PRK-04, T-PRK-05 | R-RUN-15 |
| HUD shell, feedback rows, telemetry log | UXF T-UXF-01, T-UXF-02, T-UXF-08 | HUD, §14, R-RUN-23 |
| Settings, cheats, CVars, test harness | FND T-FND-07, T-FND-09, T-FND-10 | Data, tests |

Consumers: CSM (death count, revive charges on `ARunPlayerState`, site bounds), TFM (phase for Focus availability and combat-time telemetry), PRK (`ARunPlayerState` host), MET later (run result record).

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Core destroyed in the same frame the boss dies | The first event processed latches the result; the second is logged and ignored (R-RUN-08) |
| Core destroyed while the perk offer is open | Close the offer, resolve as loss |
| Hero dead when the run resolves | Resolve screen shows; CSM cancels respawn |
| Hero dead when PerkChoice opens | Perk UI works in Commander Spirit Mode (CSM/PRK) |
| Wave never clears (stuck enemy) | ENM stuck recovery handles it; RUN logs a pacing warning after `WaveSoftLimitSeconds` (data); dev cheat `RunCompleteWave`. No auto-clear (not in GDD) |
| Pre-wave window shorter than minimum warning time | Wave start held until warning time is reached; warning logged (R-RUN-13) |
| Pressure modifier tag unknown to the Director | Director logs and ignores; run continues |
| Invalid run definition (no Resolve, no waves, Wave step without wave data) | `IsDataValid` error in editor; at runtime the run refuses to start and shows an on-screen error |
| Map has 0 or 2+ Cores | Error on start; run refuses to start |
| Spend after Resolve | Rejected (R-RUN-08) |
| Kill reward from an enemy dying after Resolve | Ignored |
| Tactical Focus slows time during a countdown | Step timers use game time, so the window stretches a little in real time. Accepted; the pacing log records real time |
| Hero knocked or falls through the outer edge | Boundary recovery moves the Hero to the nearest safe point (R-RUN-22); not a death |
| KillZ reached | Must not count as a run loss; recovery rule as above (verify KillZ order with CMB) |

## 11. Acceptance Criteria

### P2
- **AC-RUN-01**: Loading `L_SiegeSite_Proto` with `DA_Run_P2` runs Prep → W1 → Int → W2 → Int → W3 → Resolve; each phase change is logged and shown on the HUD.
- **AC-RUN-02**: Damaging the Core to 0 during any step ends the run as lost within one frame; no further step starts; the resolve stub shows "Core destroyed" and the wave number.
- **AC-RUN-03**: Killing the Hero (cheat `KillHero`) in each step never changes the phase or result; step timers keep counting.
- **AC-RUN-04**: Clearing W3 with the Core alive shows the P2 win stub; Restart reloads the map into a fresh Prep.
- **AC-RUN-05**: With `BuildsPerWindow = 3`, the 4th placement in one window is refused with feedback; the next window allows 3 again.
- **AC-RUN-06**: A run definition missing a Resolve step or with a Wave step without wave data fails `IsDataValid`.

### P3
- **AC-RUN-07**: `DA_Run_P3` runs the full §6.2 sequence end to end (5 waves, pressure event, boss, resolve) with no manual intervention other than playing.
- **AC-RUN-08**: Before every wave and the boss, the forecast is published when the window opens and the wave never starts before the minimum warning time has passed (logged timestamps prove it).
- **AC-RUN-09**: W4's forecast shows the pressure-event modifier; the Director receives that modifier for W4 only.
- **AC-RUN-10**: PerkChoice appears after W1, W3, W4; the next step starts only after a pick.
- **AC-RUN-11**: Run resource starts at the definition value, increases on kills by unit tag and on wave clear, decreases on build; a build costing more than the balance is refused with feedback and the balance is unchanged.
- **AC-RUN-12**: Boss defeated → resolve as won with summary; Core destroyed → resolve as lost with reason and wave number; both offer Restart and Quit.
- **AC-RUN-13**: A run result record (outcome, reason, duration, waves cleared, Hero deaths, resource earned/spent, perk IDs) is broadcast and written to the telemetry log at Resolve.
- **AC-RUN-14**: Core HP crossing 25% triggers the Core critical feedback once; healing above 25% clears it.
- **AC-RUN-15**: Walking the Hero to the site edge shows the boundary warning, the outer edge blocks movement, the run continues; teleporting the Hero outside the outer bounds returns it to a safe point within 1 s.
- **AC-RUN-16**: After a full run, the telemetry log contains every step's game/real duration and target, and the total run time.
- **AC-RUN-17**: An Automation Spec round-trips `FRunStateData` and `FRunPlayerData` through serialization with no loss; neither struct contains UObject pointers.
- **AC-RUN-18** (§36 Full Run DoD): In the G3 playtest, a full run takes ~25 min after tuning (pass band set at G3 review, working band 22–28 min).

## 12. Open Questions / Assumptions

| ID | Item | Default used |
|---|---|---|
| A-05 | P2 free building with build limit; P3 one run resource | R-RUN-09, R-RUN-17 |
| A-07 | Perk offers after W1, W3, W4 | R-RUN-12, R-RUN-15 |
| A-08 | P2 waves are authored | R-RUN-03 |
| A-11 / Q-10 | No mid-run save | R-RUN-11 |
| Q-16 | Leaving the Siege Site | Soft boundary: warning + push-back, no run fail (R-RUN-22) |
| Q-17 | Mid-run event content | One Director modifier before W4, split-lane pressure (R-RUN-14) |
| NEW-RUN-1 | Wave end condition. Default: wave ends when DIR reports it cleared; no wave timer or wave fail. | REQUIRED to confirm with DIR |
| NEW-RUN-2 | Boss defeated while minions are alive. Default: run won immediately; remaining enemies removed at Resolve. | REQUIRED to confirm with BOS |
| NEW-RUN-3 | Player "call next wave early" to shorten windows. Not in GDD. | FUTURE, not built |
| NEW-RUN-4 | Collect = automatic credit, no physical pickups. | IMPROVEMENT if playtest asks |
| NEW-RUN-5 | P2 build allowance resets each window (no carry-over), 3 per window. | Tune at G2 |
| Placeholders | Starting resource 300, intermission 45 s, pressure window 45 s, boss lead-in 20 s, Core critical 0.25, `WaveSoftLimitSeconds` 360 | No GDD values; all data |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Sequence one run from data; gate waves behind forecasts; own run result, run resource, build allowance, Core critical state, site boundary, pacing records |
| Inputs | `URunDefinition`; DIR wave cleared / enemy killed events, minimum warning time; Core `UHealthComponent` damage/death; BOS boss defeated; PRK perk chosen; DEF spend requests; boundary overlap events; cheats |
| Outputs | Calls to DIR: prepare/forecast wave, start wave, apply modifier, stop and clear; perk offer request to PRK; HUD data in `ARunGameState`; `FRunResult` record; telemetry events; `Feedback.Run.*` events |
| State | `FRunStateData` on `ARunGameState` (definition ID, phase, step index, step timing, wave number, resource, allowance, Core critical, outcome, end reason); `FRunPlayerData` on `ARunPlayerState` (stats, revive charges) |
| Events | `OnRunPhaseChanged(Phase, StepIndex)`, `OnRunResourceChanged(New, Delta)`, `OnBuildAllowanceChanged`, `OnCoreCriticalChanged(bool)`, `OnRunResolved(FRunResult)`, `OnBoundaryWarningChanged(bool)` |
| Data model | `URunDefinition`: `Steps[]` (type, duration, pacing target, wave definition, modifier tag, wave-clear reward, display name), `EconomyMode` (BuildLimit/RunResource), `BuildsPerWindow`, `StartingResource`, `KillRewardByUnitTag`, `CoreCriticalFraction`, `TargetRunSeconds`, `WaveSoftLimitSeconds` |
| Failure cases (§34.4) | Core critical HP (R-RUN-21), Core destroyed (R-RUN-06), player leaving Siege Site (R-RUN-22), Hero death (R-RUN-06, flow in CSM), boss reset (BOS; RUN keeps waiting in Boss step), invalid data, stuck wave |
| Performance | Event-driven; one timer per timed step; no Tick except the HUD countdown text; boundary uses overlap events |

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Prep / Intermission open (`Feedback.Run.WindowOpen`) | — | Soft stinger | Phase label + countdown | Countdown readable at a glance (§28.1) |
| Wave countdown last 10 s (`Feedback.Run.WaveCountdown`) | Countdown pulse | Tick per second, last 3 louder | Countdown highlight | Player hears a wave coming without looking |
| Wave start (`Feedback.Run.WaveStart`) | Lane direction flash (DIR forecast lanes) | War horn | "Wave n/5" banner, 2 s | Distinct from boss horn |
| Wave cleared / Collect (`Feedback.Run.WaveCleared`) | — | Short victory sting | "Wave n cleared +X" toast | Toast never covers the Core HP bar |
| Resource gained (`Feedback.Run.ResourceGained`) | Floating +X near the HUD counter | Soft tick, throttled | Counter bump | Throttle to 4/s so swarms do not spam |
| Spend refused (`Feedback.Run.CannotAfford`) | Counter flashes red | Error blip | Reason text ("Not enough", "No builds left") | Always says why |
| Pressure event (`Feedback.Run.PressureEvent`) | — | Alarm sting | Banner with modifier name; forecast highlights the modifier | Visible ≥ minimum warning time |
| Boss incoming (`Feedback.Run.BossIncoming`) | Boss entry point marker | Unique boss horn | "Boss" banner + lead-in countdown | Different from wave horn |
| Core critical (`Feedback.Run.CoreCritical`) | Core HP bar pulses red | Core critical alarm (§28.3 "Core under attack" family) | Warning text | Fires on threshold crossing only, not every hit |
| Core destroyed (`Feedback.Run.CoreDestroyed`) | Core destruction VFX, camera on Core | Collapse sting | Transition to resolve | — |
| Run won / lost (`Feedback.Run.Victory`, `Feedback.Run.Defeat`) | — | Music sting | Resolve screen with reason + summary + Restart/Quit | Loss reason in one line |
| Boundary warning (`Feedback.Run.BoundaryWarning`) | Screen-edge vignette | Low warning tone | "Return to the Siege Site" | Clears as soon as the Hero is back inside |
| Boundary recovery (`Feedback.Run.BoundaryRecovered`) | Short fade | — | — | Never looks like a death |

`Feedback.Core.UnderAttack` (per-hit, throttled) belongs to DEF/UXF (T-UXF-07); RUN only owns the threshold crossing.
