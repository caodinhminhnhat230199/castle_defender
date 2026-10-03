# Boss (BOS): Specification

| | |
|---|---|
| Feature ID | BOS |
| GDD sections covered | §22 (all), §12.4 (boss Focus restriction), §8.2 (curated boss wave), §34.4 (boss reset / bug recovery), §28.3 (boss phase change audio), §1.2, §6.2, §7.2 (boss phase hint), §25.1 (boss variation hook only), §32 P3 / VS |
| Phases | P3 (framework + one two-phase boss), VS (provisional polish) |
| Owner of | Boss framework (definition, phases, phase transitions, summons, recovery), one prototype boss, boss HUD bar |
| Not owner of | Lane/route rules (DEF), enemy base behavior (ENM), boss wave scheduling (DIR T-DIR-08), run win/lose (RUN), Tactical Focus meter (TFM), feedback rows (UXF) |
| Status | Draft v1. VS section is provisional; re-plan after G3 |

Label note: rules from unlabeled GDD sections are marked `[UNLABELED]`. `Assumption` rules link to `A-xx` or a `NEW-BOS-n` question (Section 12).

## 1. Overview

The boss is the run's final test: it checks the decisions the player made before (Pacing principle, §2). It is not just an enemy with more HP (§22.1). Each phase changes a battlefield rule and pushes a different layer.

The prototype boss is a two-phase variant of the Siege Behemoth example (A-09, §22.4):
- **Phase 1, Breach:** enters the main lane and breaks blockers/towers on its way. Tests Defense placement, with Hero damage and Armor Broken as the answer.
- **Phase 2, Split Pressure:** keeps pushing while summoning minions on two other lanes. Tests Army: the player must split squads.

The boss is built on the enemy base (ENM), so it already follows lane routes, attacks Path Obstacles and fights in Local Aggro.

## 2. Player Experience

- **"Everything I built is being tested."** Barricade layout, tower placement and squad assignments made earlier now decide the outcome (§33 21:00).
- **Readable escalation.** Every big attack is telegraphed; the phase change is loud and clear (§22.3, §28.3).
- **Hard but fair.** No unreasonable one-shots (§22.3). Hero death does not end the fight; Army + Tower keep holding (§18.1).
- **The Hero matters.** The Hero handles the boss directly (§1.2), using Heavy/parry openings on it.

## 3. Core Loop

```text
Forecast boss hint → Prepare defense → Boss enters main lane (Phase 1: breaks structures)
→ Hero + towers damage it, barricades delay it → HP threshold → Phase change (telegraph)
→ Phase 2: summons on two lanes → split squads, Hero chooses where to be
→ Boss defeated → Resolve (RUN)   |   Core destroyed → run lost (RUN)
```

## 4. Gameplay Rules

### P3 scope

- **R-BOS-01** (§22.1) [LOCKED]: The boss changes a battlefield rule or forces use of several layers. A boss that is only "more HP" fails this spec.
- **R-BOS-02** (§22.2) [UNLABELED, required wording "phải"]: **2/3 Layer Test**: the boss tests at least 2 of 3 layers (Hero, Army, Defense). Target: all three across its phases.
- **R-BOS-03** (§22.3) [LOCKED], A-09: Boss baseline has 2–3 phases; the prototype boss has exactly 2 (§32 P3).
- **R-BOS-04** (§22.3) [LOCKED]: Every phase has a clear telegraph, its own mechanic, clear counterplay, no unreasonable one-shot, and readable escalation.
- **R-BOS-05** (§22.4, A-09): **Phase 1**: the boss advances along the main lane and prioritizes breaking blockers and towers (on its route, and towers within its Local Aggro radius). It obeys §14.3 stop-and-attack like any enemy.
- **R-BOS-06** (§22.4, A-09): **Phase 2**: the boss summons minions from two directions (two lane spawn points other than its own) to force the player to split squads. Summons are prototype archetypes (Swarm/Armored); artillery summons are not built (§20.3 artillery is a [DEFERRED] biome special).
- **R-BOS-07** (§22.4, Assumption NEW-BOS-2): In Phase 2 the boss keeps following its lane (slower or faster per data), so main-lane pressure continues while side lanes are attacked.
- **R-BOS-08** (§1.2, §9.7) [LOCKED]: The boss has melee attacks the Hero must read (dodge/block/parry) and Poise; poise break → Staggered opens Hero/Army follow-ups. Boss poise and Staggered duration are [TUNABLE].
- **R-BOS-09** (§10.2, §21.1) [UNLABELED]: The boss is armored; Armor Broken (Warlord Heavy) makes Ballista and Army damage on it more effective. This is the Phase 1 Hero ↔ Tower synergy.
- **R-BOS-10** (§22.3, §34.1): Phase thresholds are fractions of max HP in data [TUNABLE, default Phase 2 at 50%].
- **R-BOS-11** (§22.3, §28.3): A phase change is a short transition: the boss stops attacking, plays a transition telegraph, fires the boss phase change audio event, then enters the new phase. Transition length is [TUNABLE].
- **R-BOS-12** (§8.2) [LOCKED]: The boss wave is scripted/curated, never procedural. The boss is spawned only by the curated boss wave hook (T-DIR-08) at the Siege Site's boss entry (§5.3).
- **R-BOS-13** (§8.2, §31.2) [LOCKED]: Summons go through the spawner and respect the concurrent enemy cap and lane caps. If the cap is full, a summon is delayed, never forced.
- **R-BOS-14** (§12.4) [LOCKED]: A boss phase may restrict Tactical Focus (via the TFM hook T-TFM-05), but must never disable the whole system. A restriction ends when its phase ends, the boss dies, the run ends or the boss is recovered.
- **R-BOS-15** (§34.4) [UNLABELED, required]: Boss reset / bug recovery: the boss never soft-locks the run. Recovery keeps the boss's current HP and phase (the player is not punished for a bug).
- **R-BOS-16** (§6.2, §18.1): Boss defeat is reported once; RUN resolves the run (T-RUN-06). Core destruction during the boss still loses the run (RUN).
- **R-BOS-17** (§18.1, §18.2) [LOCKED]: Hero death during the boss fight does not reset or pause the boss; the fight continues in Commander Spirit Mode.
- **R-BOS-18** (§28.1) [LOCKED]: Boss HP and current phase are readable at a glance (boss HUD bar).
- **R-BOS-19** (§7.2) [TUNABLE]: The boss definition carries a short phase hint text the Threat Forecast may show (DIR T-DIR-06 reads it).
- **R-BOS-20** (§34.1, §38) [LOCKED]: Boss content is data: phases, thresholds, summons, Focus restriction, hints, attacks. A second boss with the same phase mechanics needs only new data.
- **R-BOS-21** (§25.1) [LOCKED]: "Boss combo/phase variation" as a difficulty axis is a data hook only (phase data, attack list). No difficulty code in BOS.

### VS scope (provisional)

- **R-BOS-22** (§32 VS): One boss polished: animation, VFX, audio and UI near target quality.
- **R-BOS-23** (§22.3, §22.4): A third phase (e.g., duel zone / arena pressure) is considered only if the G3 2/3 Layer Test shows a weak layer (NEW-BOS-4).
- **R-BOS-24** (D-08): Boss AI implementation (C++ FSM vs StateTree) is re-evaluated at VS.

## 5. Player Actions

| Action | Effect |
|---|---|
| Build Barricades on the main lane before the boss | Boss stops to break them (time for Hero/towers) |
| Place Ballista in range of the main lane | Damages boss; better after Armor Broken |
| Heavy attack / parry the boss | Armor Broken, poise damage → Staggered opening |
| Split squads to the side lanes in Phase 2 | Holds summons away from the Core |
| Choose where the Hero fights in Phase 2 | Boss lane vs a side lane |
| Use Tactical Focus to reassign squads | Fast split (subject to any phase restriction) |

## 6. Success / Failure Conditions

- **Win:** boss HP reaches 0 → boss defeated reported → RUN resolves win.
- **Lose:** Core destroyed at any time (RUN).
- **Partial:** losing structures in Phase 1 is expected; the question is whether enough remain.
- **Hero death:** not a failure (Commander Spirit continues).

## 7. Scope

### In Scope (P3)
`ABossCharacter` on the enemy base, `UBossDefinition` with phases, phase controller + transition, Phase 1 structure-breaking behavior via data, Phase 2 summons through the spawner, telegraphs and phase feedback, curated boss wave integration, recovery watchdog, Focus restriction wiring, boss HUD bar, telemetry for the 2/3 Layer Test, Functional Tests, G3 boss playtest.

### Out of Scope
Phase 3, artillery or other special summons, AoE ground attacks, arena/duel zone, cinematic intro, unique boss music system, StateTree (VS evaluation only), multiple bosses, boss-specific difficulty modes.

## 8. Anti-Goals

- Not an HP sponge (§22.1, §25.1).
- No unreadable one-shots or untelegraphed attacks (§22.3).
- No boss mechanic that ignores lane rules (§14) without being an explicit phase rule in data.
- Not a full disable of Tactical Focus (§12.4).
- No procedural boss wave (§8.2).

## 9. Dependencies

| Needs | From |
|---|---|
| Enemy base, brain, attacks, Staggered, lane route, Local Aggro, Path Obstacle, stuck recovery | ENM (T-ENM-01..04, -07, -08, -09, -10, -15) |
| Curated boss wave hook, lane spawn points, spawner, cap constraint, forecast | DIR (T-DIR-01, -04, -06, -08) |
| Boss run step, win resolve | RUN (T-RUN-03, -06) |
| Focus restriction hook | TFM (T-TFM-05) |
| Shared states (Staggered, Armor Broken) | SYN (T-SYN-01, -02) |
| HUD shell, feedback subsystem, audio events, telemetry log, lane danger indicators | UXF (T-UXF-01, -02, -06, -07, -08) |
| Towers / Barricade / Core to test against | DEF (T-DEF-03, -08, -10) |
| Hero death flow | CSM (T-CSM-01) |

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| One hit crosses the Phase 2 threshold by a lot | Phase 2 still starts (transition runs); damage is not clamped (per-hit damage is small vs phase window) |
| Boss killed by cheat before Phase 2 | Boss dies normally; defeat reported |
| Boss Staggered during transition | Transition is not interrupted (state applies, no behavior change) |
| Summon spawn point blocked or cap full | Summon retried next interval; phase continues |
| Boss stuck | ENM stuck ladder; teleport only when off-screen; HP/phase kept |
| Boss outside Siege Site boundary / below kill height | Teleport to last valid route point; HP/phase kept |
| Transition never completes (montage missing) | Transition ends at max time |
| Boss actor destroyed without defeat (bug) | Respawned at the boss entry with saved HP fraction and phase |
| Hero dies during boss | Fight continues; boss drops Hero target |
| Core destroyed during boss | Run lost (RUN); boss and summons despawn; Focus restriction cleared |
| Boss dies with summons alive | Summons despawn with feedback (NEW-BOS-1) |
| Tactical Focus active when a restricted phase starts | Restriction applied by TFM rules (e.g., meter stops refilling); never forced off abruptly unless TFM says so |
| Boss dies during Focus restriction | Restriction cleared immediately |

## 11. Acceptance Criteria

### P3
- **AC-BOS-01**: `DA_Boss_SiegeBehemoth` passes data validation: 2 phases, thresholds descending in (0, 1], first phase at 1.0, declared layers across phases ≥ 2 distinct, summon entries valid.
- **AC-BOS-02**: The boss appears only through the curated boss wave (T-DIR-08) at the boss entry and follows the main lane.
- **AC-BOS-03**: Phase 1: the boss stops at Barricades on its route and breaks them; it leaves its route (within leash) to break a tower in its radius; unopposed, it reaches and damages the Core.
- **AC-BOS-04**: At the threshold, the boss stops attacking, plays the transition, `Feedback.Boss.PhaseChange` fires (audio + HUD cue), and Phase 2 starts within the max transition time.
- **AC-BOS-05**: Phase 2: summons appear at two different lane spawn points, announced by `Feedback.Boss.Summon`; they follow their lanes; alive enemy count never exceeds the cap.
- **AC-BOS-06**: Every boss attack has a telegraph; no single boss attack kills a full-HP Warlord.
- **AC-BOS-07**: A configured Focus restriction is active only during its phase and cleared on phase end, boss death, run end and recovery. Focus is never fully disabled.
- **AC-BOS-08**: Boss HUD bar shows name, HP, phase markers at thresholds and current phase; updates from events; hides on defeat.
- **AC-BOS-09**: Recovery: stuck boss recovers; out-of-bounds boss returns to the last valid route point; transition timeout works; a destroyed-without-defeat boss is respawned with its HP fraction and phase.
- **AC-BOS-10**: Boss defeat is reported exactly once; RUN resolves a win; live summons are handled per NEW-BOS-1.
- **AC-BOS-11**: Hero death mid-fight: fight continues in Commander Spirit Mode; boss is not reset.
- **AC-BOS-12** (**2/3 Layer Test**): In each G3 playtest session, the score sheet shows the boss tested at least 2 of 3 layers, using telemetry + observer notes:
  - **Defense** tested: in Phase 1 the boss stopped at ≥1 player structure, and the structure layout changed its time-to-Core.
  - **Army** tested: in Phase 2 squads engaged summons on ≥2 different lanes, or an unattended lane caused Core damage.
  - **Hero** tested: Hero dealt a meaningful share of boss damage (telemetry by `SourceLayer`), and dodged/blocked/parried boss telegraphs.
  - Target: all three in most sessions. Fewer than 2 in any session → CHANGE decision recorded.

### VS (provisional)
- **AC-BOS-13**: Polished boss passes AC-BOS-01..12 again with final assets.
- **AC-BOS-14**: Decision recorded on StateTree vs C++ FSM and on Phase 3 (yes/no with evidence).

## 12. Open Questions / Assumptions

| ID | Question / assumption | Class | Default |
|---|---|---|---|
| A-09 | Prototype boss = two-phase Siege Behemoth variant (structure-breaking + split-pressure summons) | Master plan | Followed |
| NEW-BOS-1 | When the boss dies, do live summons die too? | REQUIRED | Yes: despawn with feedback, so resolve is clean |
| NEW-BOS-2 | Does the boss keep advancing in Phase 2? §22.4 only says it summons | REQUIRED | Yes, keeps following its lane (data flag + speed multiplier) |
| NEW-BOS-3 | Does the prototype boss restrict Tactical Focus in any phase? | IMPROVEMENT | Hook wired, default no restriction; enable in Phase 2 only if G3 shows Focus trivializes split pressure |
| NEW-BOS-4 | Third phase (duel zone / arena pressure, §22.4) at VS? | FUTURE | Only if G3 2/3 test shows a weak layer |
| NEW-BOS-5 | AoE ground attacks (slam circles) for readability? Not in GDD | IMPROVEMENT | Not built; melee swings with long telegraphs |
| NEW-BOS-6 | Boss phase hint text shown in Forecast (§7.2): wording and when | REQUIRED (DIR) | One line per phase in data; DIR decides display |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Boss lifecycle, phase evaluation and transitions, per-phase behavior overrides, summons, Focus restriction requests, recovery, boss bar data |
| Inputs | `UBossDefinition`; spawn from the boss wave hook (lane, entry point); `FCombatHit` received; HP changes; spawner responses; run phase end; Siege Site boundary |
| Outputs | Enemy behavior (via ENM); summon spawn requests; `Feedback.Boss.*` plays; Focus restriction apply/clear; `OnBossPhaseChanged`, `OnBossDefeated`; telemetry rows |
| State (runtime only) | Current phase index, transition flag + start time, summon timers, alive summon list, last valid route point, recovery counters, `FBossSnapshot` (phase, HP fraction) |
| Events | Fires `OnBossSpawned`, `OnBossPhaseChanged(Old, New)`, `OnBossDefeated`, `OnBossRecovered(Reason)`. Listens to ENM/FND health and state events, spawner removal events |
| Failure cases | Table below |
| Performance | One boss; summon count bounded by data + cap; no per-frame work beyond ENM |

### Data model: Boss definition (design level)

| Field | Example / default | Label |
|---|---|---|
| Display name | "Siege Behemoth" | Data |
| Base enemy archetype (stats, attacks, armor, poise) | `DA_Enemy_Boss_SiegeBehemoth` | [TUNABLE] |
| Phases (2–3) | 2 | [LOCKED] range, A-09 |
| Phase: enter at HP fraction | 1.0, 0.5 | [TUNABLE] |
| Phase: name, forecast hint | "Breach", "Split Pressure" | Data |
| Phase: layers tested (Hero / Army / Tower) | P1: Defense + Hero; P2: Army + Hero | Design declaration, validated ≥2 total |
| Phase: Local Aggro priority override | P1: Path Obstacle, Combat Tower, Blocker, Hero, Soldier, Objective | [TUNABLE] |
| Phase: move speed multiplier, keeps following lane | P2: 1.0, yes | [TUNABLE] |
| Phase: attack list override (optional) | P2 adds a faster swing | [TUNABLE] |
| Phase: summons (archetype, count, lane spawn point, interval, max alive) | P2: 6 Swarm per side lane every 20 s, max 18 | [TUNABLE] |
| Phase: Focus restriction (TFM struct) | none | [TUNABLE] |
| Phase: transition animation | `AM_Boss_Roar` | Data |
| Max transition time | 4 s | [TUNABLE] |
| Structure damage multiplier (via base archetype) | ~3× | [TUNABLE] |

### Failure / recovery cases (§34.4)

| Case | Handling |
|---|---|
| Boss stuck | ENM ladder: re-move → re-route → off-screen teleport to next route point |
| Out of bounds / fell | Teleport to last valid route point (recorded at each checkpoint) |
| Transition stalls | Force-complete at max transition time |
| Summon refused | Retry next interval |
| Boss actor lost without defeat | Respawn at boss entry from `FBossSnapshot` |
| Run ends (any reason) | Despawn boss + summons, clear Focus restriction |
| Hero death | No reset |
| Debug | Cheats: spawn boss, set phase, force recovery, kill boss |

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule | Tag |
|---|---|---|---|---|---|
| Boss spawn / entry | Entry animation at boss entry | Boss arrival sting | Boss bar appears | Player knows the boss lane immediately | `Feedback.Boss.Spawn` |
| Attack telegraph | Long wind-up, weapon glow, larger than normal enemies | Wind-up roar/whoosh | — | Every attack telegraphed; heavier = longer | `Feedback.Boss.Telegraph` |
| Breaking a structure | Slam on structure | Structure impact (DEF/UXF) | Structure HP marker | Player sees which structure is going down | `Feedback.Structure.*` |
| Staggered / Armor Broken | Shared state VFX/icon | Shared state SFX | State icon on bar | Same language as all enemies (§28.2) | `Feedback.State.*` |
| Phase change | Roar animation, camera shake, VFX burst | Boss phase change SFX (§28.3) | Banner "Phase 2", bar phase marker lights | Cannot be missed even off-screen | `Feedback.Boss.PhaseChange` |
| Summons | Spawn VFX at each spawn point | Summon horn per lane | Lane danger indicators (UXF-07) + off-screen markers | Player knows which two lanes are hit | `Feedback.Boss.Summon` |
| Focus restricted | — | — | Restriction shown on Focus meter (TFM) | Player knows why Focus behaves differently | `Feedback.Boss.FocusRestricted` |
| Defeated | Death animation | Victory sting | Bar hides, resolve starts (RUN) | Clear end of fight | `Feedback.Boss.Defeated` |
| Hidden recovery | None (only when off-screen) | — | — | Logged only | — |
