# HUD & Feedback (UXF): Specification

| | |
|---|---|
| GDD sections covered | §28 (28.1 readability, 28.2 visual language, 28.3 audio language), §9.8 combat feel, §34.5 feedback contract, §34.4 failure feedback, §29.2 input principles, §2.1 (hit/stagger readable), §12.2 (overlay items via shared markers + lane danger), §31.2 (VFX LOD) |
| Phases | P0 → P3 (cross-cutting). VS rows provisional |
| Owner folder | `ai/game/13-hud-feedback/` |
| Status | Draft v1. Class/asset/tag names are proposals (no UE project exists yet). Tag names in Section 14 follow the finished feature docs |

## 1. Overview

UXF owns the presentation contract for the whole game:

- one feedback entry point (`UFeedbackSubsystem`) and one data table (`DT_Feedback`) keyed by `Feedback.*` tags (D-10), with cooldowns and burst throttling;
- world-level presentation state: HUD layers (Combat, Command Wheel, Build, Tactical Focus, Commander Spirit, Modal) and the markers' tactical display mode;
- the HUD shell (`WBP_GameHUD`) every feature plugs its panel into, plus Hero HP/stamina;
- combat feel presentation: hit stop, camera shake, impact SFX/VFX per material and hit type;
- world markers (squad, enemy class, structure HP), combat state icons/VFX, Core HP and lane danger (lane danger values live on a world subsystem so non-HUD code can read them);
- the §28.3 audio rules;
- playtest tooling: telemetry log with a generic event API, notes template, and the feedback contract audit run at every gate.

UXF does not own gameplay state. Features decide *when* a state happens and call the feedback API; UXF decides *how* it is shown and keeps the contract table (Section 14) complete.

## 2. Player Experience

- Hits feel heavy and readable without photorealism (§9.8): normal, armored, parry and stagger hits look and sound different.
- In a 1–2 second glance the player knows where squads are, which lane is in danger, which tower is about to break, which enemies are elite/siege, Core HP, the current command state and the next forecast (§28.1).
- The player does not need to stare at the UI: important events have their own sound (§28.3).
- The screen stays calm: alerts do not spam, effects do not hide telegraphs.

## 3. Core Loop

```text
Gameplay state changes (owner feature)
→ Owner calls UFeedbackSubsystem::Play(Feedback.* tag, context)
→ Data row plays visual / audio / camera / UI (throttled)
→ Player reads it in ≤ 1–2 s, reacts
→ Telemetry records it → gate audit checks coverage and readability
```

## 4. Gameplay Rules

### 4.1 Contract and pipeline (all phases)

- **R-UXF-01** (§34.5) Every important gameplay state has visual feedback, audio feedback, UI feedback if needed, and a failure/readability rule. Section 14 lists them; `—` means "none by design", never "forgotten".
- **R-UXF-02** (§34.5; D-10) All contract feedback goes through `UFeedbackSubsystem` with a `Feedback.<Domain>.<Event>[.<Variant>]` tag. Assets and numbers per tag live in `DT_Feedback`, not in gameplay code. A row may have variant rows (`<Tag>.Armored`, `<Tag>.Infantry`) picked from the context.
- **R-UXF-03** (§28, §31.2) Feedback does not spam: (a) each row has a cooldown [TUNABLE] (global or per actor); (b) each row has a burst limit [TUNABLE] (starting value: 4 plays per 0.25 s per tag) so one Bombard splash staggering 15 enemies plays a few cues, not 15; (c) threshold alerts fire once per crossing (owners: DEF, SQD, RUN; Hero low HP: UXF).
- **R-UXF-04** (§31.2) [LOCKED] VFX density has LOD: Niagara effect-type budgets and scalability; sound concurrency limits. Crowd readability beats raw effect count.
- **R-UXF-05** Assumption (NEW-UXF-1, accessibility basics): no information is carried by color alone (state, class, alert and category cues pair color with shape). Camera shake has a global scale that can reach 0.
- **R-UXF-06** (§34.5) A tag with no data row is reported in development builds (on-screen + log, once per tag) and is a no-op in Shipping (log once).

### 4.2 Combat feel (P0, §9.8 [LOCKED])

- **R-UXF-07** (§9.8) Short hit stop only on large impacts: `Feedback.Combat.Hit.Heavy`, `Feedback.Combat.Parry`, `Feedback.Combat.BlockBreak`, `Feedback.State.Staggered.Applied`, and only when the local Hero is instigator or target. Duration [TUNABLE] per row (starting 0.06–0.12 s), global cap in `UGameTuningSettings` (starting 0.15 s).
- **R-UXF-08** (§9.8; D-13) Hit stop sets per-actor `CustomTimeDilation` on the actors in the hit only. It never touches global time dilation, which Tactical Focus owns.
- **R-UXF-09** (§31.1, §2.1) Hit stop and shake never drop input: inputs buffered during hit stop execute after it (verified with CMB's input buffer).
- **R-UXF-10** (§9.8) Camera shake is controlled: only Hero-involved hits or world events within a radius; one active shake per tag; global scale [TUNABLE] (default 1.0).
- **R-UXF-11** (§9.8) Impact sound differs by the hit surface's armor/material (Flesh, Armor, Shield, Wood, Stone via Physical Materials). Unknown surface → row default sound.
- **R-UXF-12** (§9.8) Hit VFX distinguish normal / armored / parry / stagger. CMB's `UCombatLibrary::DeliverHit` plays one tag per outcome (`Hit.Light`, `Hit.Heavy`, `Block`, `BlockBreak`, `Parry`) with `bTargetArmored`; UXF plays the `.Armored` variant row for armored targets; poise break plays SYN's `Feedback.State.Staggered.Applied`. Optional P1 accent `Feedback.Combat.Hit.StateBonus` when a hit consumed a state bonus, so synergy is visible (§33, §36); needs a flag from `DeliverHit` (NEW-UXF-10).
- **R-UXF-13** (§2.1, §9.6) Hero HP and stamina are always visible in combat. Stamina spending shows on the bar; `Feedback.Hero.StaminaInsufficient` flashes the bar. The HUD rebinds when the possessed Hero pawn changes (respawn spawns a fresh pawn).

### 4.3 Readability (§28.1 [LOCKED], §28.2)

- **R-UXF-14** (§28.1) [LOCKED] Each item of the 1–2 second list has a named element. Pass bar of the quiz: NEW-UXF-9.

| §28.1 item | Element | Owner | Phase |
|---|---|---|---|
| Where squads are | Squad world markers + `WBP_SquadStrip` + tactical display in Focus/Spirit | UXF `T-UXF-04` (component); SQD `T-SQD-13` (wiring, strip); TFM `T-TFM-03` | P1 / P3 |
| Which lane is dangerous | Lane danger strip from `ULaneDangerSubsystem` (None/Low/Medium/High per lane) + lane pulses | UXF `T-UXF-07` | P2 |
| Which tower is about to break | Structure HP markers (shown when hit, critical state) + positional critical alarm | UXF `T-UXF-04`; DEF `T-DEF-17` | P2 |
| Which enemy is elite/siege | Silhouette rules + class icon marker on Armored / Siege / Boss (none on Swarm) | UXF `T-UXF-04`, `T-UXF-12`; ENM art | P1 / P2 |
| Core HP | Core HP bar, always visible in run maps | UXF `T-UXF-07` | P2 |
| Current command state | `WBP_SquadStrip` order icon + marker order icon | SQD `T-SQD-13` | P1 |
| Next forecast | Forecast panel in its HUD slot | DIR `T-DIR-07` | P3 |

- **R-UXF-15** (§28.2) Ally and enemy silhouettes and colors differ (production plan: ally blue family, enemy red family, elite/siege accent).
- **R-UXF-16** (§28.2) Armored, Giant/Siege and Swarm are distinguishable from far away by silhouette/size; Armored/Siege/Boss also carry a class icon. Swarm carries none (icons on crowds are noise, §31.2).
- **R-UXF-17** (§28.2) Tower role reads through shape and VFX: Ballista = long single bolt, Bombard = arcing shell + splash decal sized to the real radius, Barricade = low wide blocker (DEF rows).
- **R-UXF-18** (§28.2) Marked / Armor Broken / Staggered use one icon, color and loop VFX each, identical on every unit type and from every source. Source of truth: SYN's `DT_CombatStatePresentation`. Up to 3 icons, ordered by `DisplayPriority`. State colors are reserved.
- **R-UXF-19** (§28.1, §29.2) HUD panels never cover the screen center or the lock-on target; toasts stay at the screen edge (max 3).
- **R-UXF-20** (§13.6) The hidden stuck-recovery teleport produces no feedback and only happens off-screen (SQD rule, audited here).

### 4.4 Audio language (§28.3)

- **R-UXF-21** (§28.3) Each of these has its own sound: tower critical HP (`Feedback.Structure.Critical`), Core under attack (`Feedback.Core.UnderAttack`), squad low strength (`Feedback.Squad.LowStrength`), boss phase change (`Feedback.Boss.PhaseChange`), successful parry (`Feedback.Combat.Parry`), armor break (`Feedback.State.ArmorBroken.Applied`), tactical order acknowledged (`Feedback.Command.Acknowledged`, per squad type variant).
- **R-UXF-22** (§28.3 "do not make the player look at the UI all the time") Each §28.3 event is identifiable by sound alone. Positional events (tower critical, squad low strength) are spatialized; global events (Core under attack, boss phase change) are 2D. Alert-class sounds duck combat SFX briefly.

### 4.5 HUD, markers, run indicators

- **R-UXF-23** (§28.1; D-11, D-19) One HUD shell with fixed slots; each feature owns its panel. HUD layers are world-level presentation state on `UFeedbackSubsystem`, derived from the controller-owned mode stack for SQD wheel, DEF build, TFM Focus, CSM Spirit and PRK/RUN modal. Features push/pop their modes; presentation listens to `OnPlayerModeChanged`, and widgets only observe. Mode-driven projection is integrated by the relevant mode-owning task from P1 onward; the P0 layer setter is not a second mode owner. Visibility matrix in technical plan §5.3. Plain UMG (D-11).
- **R-UXF-24** (§28.1, §12.2, §18.2) World markers: squads (type, order icon, strength), enemy class (Armored/Siege/Boss), structures (HP when hit; critical state). Normal display: distance-limited [TUNABLE]. **Tactical display** (on while the Tactical Focus or Commander Spirit layer is active): all markers visible, structures show HP. Hidden in Modal.
- **R-UXF-25** (§28.1, §12.2) Core HP bar always visible in run maps (P2+). Lane danger per lane = alive enemy threat weighted by closeness to the Core, plus temporary pulses (path opened, Core attacked, boss summons). Values are owned by `ULaneDangerSubsystem` and readable by TFM/CSM; widgets only display. Thresholds [TUNABLE] in `UGameTuningSettings`.
- **R-UXF-26** (§34.4) Each failure/recovery case has feedback: Hero death (FC-14, FC-50), squad wipe (FC-24), tower destroyed (FC-31), blocked path opened (FC-32), Core critical HP (FC-46), leaving Siege Site (FC-49), boss reset/bug recovery (FC-66).

### 4.6 Playtest tooling (master plan §3 gates)

- **R-UXF-27** Assumption (NEW-UXF-2; master plan §3 "playtest notes recorded in `ai/game/playtests/`", §13 risk "% time in Focus"): a local telemetry log per session (sandbox map) and per run (Siege Site map) records every feedback event, a Core HP curve, time in Tactical Focus, perks offered/picked, Hero deaths, and generic events from features (`LogEvent`: RUN step timings, SQD orders, BOS layer damage, CMB action counts). Development builds only.
- **R-UXF-28** Same source: playtest notes template at `ai/game/playtests/_template.md` (created by `T-UXF-11`; this doc only describes it).
- **R-UXF-29** (§34.5; master plan §3) The feedback contract audit runs at every gate (G0–G3); its result goes into the gate's playtest notes.

## 5. Player Actions

None of its own. The player reads the HUD and hears alerts. Camera shake scale becomes a user setting at VS (MET).

## 6. Success / Failure

- Success: every contract row in the current phase fires as listed; the §28.1 quiz passes; §28.3 events are identified by ear; no alert spam; frame time within budget in crowd bursts.
- Failure: a state with no feedback (coverage gap), an alert repeating every hit, hit stop eating input or touching global dilation, markers/VFX hiding telegraphs, HUD showing a dead pawn's values after respawn.

## 7. Scope

### In Scope (P0–P3)
Feedback subsystem + table + tags + variants + cooldown/burst throttle; HUD layers + marker display mode; hit stop/shake/surface routing and the P0 hit rows; physical surfaces; HUD shell + Hero vitals + Hero low-HP feedback; world marker component; state presenter (reads SYN table); §28.3 sound classes/concurrency/ducking; alert toasts; Core HP bar + lane danger subsystem/strip; icon set + color tokens; telemetry log + generic event API + run summary; notes template; audit tooling + gate audits; crowd stress check.

### Out of Scope (owned elsewhere or deferred)
- Panels owned by features: lock-on marker (CMB `T-CMB-10`), Command Wheel and `WBP_SquadStrip` (SQD `T-SQD-05`, `T-SQD-13`), build bar (DEF `T-DEF-15`), run status (RUN `T-RUN-10`), forecast (DIR `T-DIR-07`), Focus meter/overlay (TFM), spirit HUD (CSM `T-CSM-06`), boss bar (BOS `T-BOS-06`), perk cards/tray (PRK `T-PRK-05`). UXF provides slots, layers, rows infrastructure.
- Debug HP/poise readout (SYN `game.debug.CombatStates`).
- CommonUI, gamepad UI navigation, settings menu (VS/MET).
- Dynamic music (NEW-UXF-6), floating damage numbers (NEW-UXF-8), off-screen edge indicators (NEW-UXF-4) unless a gate readability check fails.
- Final art/audio (production plan).

## 8. Anti-Goals

- Not cinematic/photoreal combat presentation (§9.8).
- No HUD that needs constant reading to play (§28.3).
- No feedback that rewards staying in Tactical Focus (§3, §12.1).
- No global gameplay event bus: the feedback subsystem is a presentation sink (D-10).
- No widget owns gameplay state (D-11, UE5 skill 09-ui).

## 9. Dependencies

| Needs | From |
|---|---|
| `Feedback.*` root, tuning settings, controller, CVars, test harness | FND `T-FND-04`, `T-FND-06`, `T-FND-07`, `T-FND-09`, `T-FND-10` |
| `DeliverHit` feedback call with Instigator/Target/`bIsHeavy`/`bTargetArmored`; health/stamina delegates; hero death; input buffer | CMB `T-CMB-01`, `T-CMB-03`, `T-CMB-04`, `T-CMB-08`, `T-CMB-09`, `T-CMB-11`; FND `T-FND-05` |
| `DT_CombatStatePresentation`, `OnStateAdded/OnStateRemoved`, `Feedback.State.*` plays | SYN `T-SYN-01`, `T-SYN-04` |
| Squad registry, marker wiring, strip, `Feedback.Command.*`/`Feedback.Squad.*` plays | SQD `T-SQD-01`, `T-SQD-12`, `T-SQD-13` |
| Enemy class, assigned lane, threat cost, telegraph plays | ENM `T-ENM-01`, `T-ENM-03`, `T-ENM-06`, `T-ENM-07`, `T-ENM-10` |
| Structures, Core, lanes, `OnStructureCritical`, `OnCoreUnderAttack`, DEF feedback rows | DEF `T-DEF-02`, `T-DEF-03`, `T-DEF-04`, `T-DEF-17` |
| Run phases, Core ref, `OnCoreCriticalChanged`, run telemetry events | RUN `T-RUN-01`, `T-RUN-10`, `T-RUN-13`, `T-RUN-14` |
| Layer requests and rows from TFM, CSM, PRK, BOS | TFM `T-TFM-01`, `T-TFM-03`; CSM `T-CSM-06`, `T-CSM-08`; PRK `T-PRK-05`; BOS `T-BOS-04`, `T-BOS-06`, `T-BOS-08` |

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Bombard splash staggers 15 Swarm in one frame | Burst limit plays ≤ 4 `Staggered.Applied` cues; state VFX still on each unit (budgeted); §28.3 alerts stay audible |
| Overlapping hit stops (parry then heavy) | Remaining duration = max of both, capped |
| Hit stop while Tactical Focus is active | Global dilation untouched; hit stop length measured in real time (NEW-UXF-7) |
| Actor destroyed during hit stop or with state VFX | Cleanup skips it; no dangling components or errors |
| Hero respawns as a fresh pawn | Vitals rebind to the new pawn's components; low-HP latch resets |
| Many enemies hit the Core | `OnCoreUnderAttack` is already throttled by DEF; row cooldown is a second guard |
| HP oscillates around a threshold | Owner latch fires once; re-arm only above re-arm value |
| Tag without row | Dev warning once; no crash |
| Telemetry disk write fails | One warning, logging off for the session, game continues |
| Several HUD layers on (Spirit + Modal) | Priority Modal > CommanderSpirit > TacticalFocus > Build > CommandWheel > Combat |
| Marker owner off-screen | Not drawn; strip/lane danger still inform (off-screen arrows NEW-UXF-4) |

## 11. Acceptance Criteria

### P0
- **AC-UXF-01** `Play` with a known tag plays its row; unknown tag → dev warning, no crash; a 20-call burst in one frame plays at most the burst limit.
- **AC-UXF-02** HUD shows Hero HP and stamina; stamina drops on dodge/block/heavy, regenerates smoothly, flashes on `Feedback.Hero.StaminaInsufficient`.
- **AC-UXF-03** Light, Heavy, Light/Heavy Armored, Block, Block Break, Parry and Staggered-applied each have distinct VFX and SFX; a tester with eyes closed names the type in ≥ 8 of 10 trials (NEW-UXF-9).
- **AC-UXF-04** Hit stop happens only on large Hero-involved impacts, freezes only the actors in the hit, never changes global time dilation, and loses no buffered input.
- **AC-UXF-05** One shake instance per tag; global scale 0 disables shakes.
- **AC-UXF-06** Impact SFX differ between Flesh, Armor, Shield and Wood/Stone surfaces.
- **AC-UXF-07** A `.jsonl` file is written per session; every line parses as JSON; `LogEvent` from a test actor appears; summary has session length, Hero deaths, parries.
- **AC-UXF-08** After Hero death and respawn (fresh pawn) the vitals show the new pawn's values.
- **AC-UXF-09** Hero low-HP feedback fires once per crossing and stops above the re-arm value.
- **AC-UXF-10** G0 feedback audit recorded with no unresolved P0 gap.

### P1
- **AC-UXF-11** Squad markers show type, order icon and strength (with SQD wiring); Armored shows a class icon; Swarm none.
- **AC-UXF-12** Setting the Tactical Focus or Commander Spirit layer switches every marker to tactical display within one frame; clearing it restores normal display.
- **AC-UXF-13** Staggered / Armor Broken / Marked show the same icon, color and loop VFX on any unit, whatever the source, ordered by priority.
- **AC-UXF-14** Parry, armor break, order acknowledged and squad low strength are identified by sound alone in ≥ 9 of 10 trials.
- **AC-UXF-15** G1 audit + readability check (squads, command state, elite) recorded.

### P2
- **AC-UXF-16** Core HP bar always visible in `L_SiegeSite_Proto`; Core critical pulse follows `OnCoreCriticalChanged`.
- **AC-UXF-17** The lane receiving a wave shows rising danger within 1 s; `ULaneDangerSubsystem::GetLaneDanger` returns the same level the strip shows; `PulseLane` raises a lane for its duration.
- **AC-UXF-18** Tower critical and Core under attack are identified by ear and are spatialized/2D as specified; structure destroyed and path opened toasts name the lane.
- **AC-UXF-19** 50+ simultaneous hit events do not cut §28.3 alerts (production plan checkpoint).
- **AC-UXF-20** Run telemetry summary has a Core HP curve and Hero deaths; RUN step events present.
- **AC-UXF-21** G2 audit recorded.

### P3
- **AC-UXF-22** Boss phase change plays its 2D sound + banner; every P3 contract tag played at least once in a full run (`game.feedback.Coverage` empty for P3 tags).
- **AC-UXF-23** HUD layer matrix holds in Tactical Focus, Commander Spirit and Modal screens.
- **AC-UXF-24** Run summary includes time in Tactical Focus (s and % of run) and perks offered/picked.
- **AC-UXF-25** §28.1 quiz: on 10 random paused frames of a full run, all 7 questions answered within 2 s on ≥ 8 frames.
- **AC-UXF-26** G3 audit recorded.

## 12. Open Questions / Assumptions

| ID | Question / assumption | Class | Default until answered |
|---|---|---|---|
| NEW-UXF-1 | Accessibility basics: no color-only cues, shake scale to 0 | REQUIRED | Applied from P0; user setting at VS (MET) |
| NEW-UXF-2 | Telemetry + notes template are tooling from master plan gate rules, not GDD | REQUIRED | JSON Lines in `Saved/Playtest/`, local only, Development builds only |
| NEW-UXF-3 | Hero low-HP threshold (GDD gives none) | TUNABLE | 30%, re-arm 40% |
| NEW-UXF-4 | Off-screen indicators for squads/Core/critical towers/boss summons (BOS spec mentions off-screen markers for summons) | IMPROVEMENT | Not built; lane strip pulses cover summon lanes. Add if G1–G3 readability fails |
| NEW-UXF-5 | Damage direction indicator on the Hero | IMPROVEMENT | No; CMB front/back hit reaction shows direction |
| NEW-UXF-6 | Dynamic music intensity | FUTURE | Stings only |
| NEW-UXF-7 | Hit stop duration in real time or game time under Focus | REQUIRED | Real time |
| NEW-UXF-8 | Floating damage numbers | OUT OF SCOPE | Debug only |
| NEW-UXF-9 | Pass bars for blind sound tests and §28.1 quiz | TUNABLE | 8/10 hit types, 9/10 §28.3 events, 8/10 quiz frames |
| NEW-UXF-10 | `Feedback.Combat.Hit.StateBonus` needs `DeliverHit` to flag hits whose state multiplier > 1 (SYN `T-SYN-07`) | IMPROVEMENT | Requested; row skipped if the flag never comes |
| NEW-UXF-11 | Row cooldown and burst window (R-UXF-03) measured in real time or game time under Focus | REQUIRED | Real time, like hit stop (NEW-UXF-7): spam is heard in real time |
| NEW-UXF-12 | Should action telemetry count every physical/requested input, including refused inputs and unchanged combo states? | DECIDED at G0 (KEEP current telemetry), see owner-review.md | Count documented OnActionStateChanged entries and played-feedback counts. Broader attempt/raw-resolution counts require a separate approved provider/consumer change; do not infer presses from polling. |
| NEW-UXF-13 | Uploaded VFX guide proposes afterimages, Focus glow, FOV/impact flashes, a second impact subsystem and mandatory pooling/destruction budgets | OUT OF SCOPE for this integration | Preserve current feedback contract, subsystem/tables and D-15; use only compatible effect-layer/readability/profiling guidance. No new presentation mechanics or numeric budgets. See [skill integration](../skill-integration.md), user decision 2026-10-09 |
| Q-13 | Art direction | Master plan | Placeholder; R-UXF-15..18 hold for any art |
| Q-16 | Leaving Siege Site boundary | Master plan | RUN rows FC-49 |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Turn gameplay events into visual/audio/camera/UI feedback from data; hold world-level presentation state (HUD layers, marker display mode, lane danger); host the HUD shell and shared indicators; record playtest telemetry; keep the contract auditable |
| Inputs | `Play(Tag, Context)` from every feature; `SetHUDLayerActive(Layer, bool)` for presentation derived from `AHeroPlayerController::OnPlayerModeChanged` as mode-owning tasks integrate; `PulseLane(Lane, Seconds)` from DEF/BOS; `LogEvent(Name, Numbers, Strings)` from any feature; delegates: health, stamina, possessed pawn, state add/remove, Core health, Core critical; `DT_Feedback`, `DT_CombatStatePresentation` |
| Outputs | Sounds, Niagara, camera shakes, per-actor hit stop, toasts, `OnFeedbackPlayed`, `OnHUDLayersChanged`, `OnLaneDangerChanged`, lane danger values, telemetry lines |
| State | Tag → row (+ variants) map; cooldown and burst counters; active hit stops; played-tag set; HUD layer flags; lane danger levels + pulses; telemetry file + summary aggregates. All world lifetime, presentation only |
| Events | `OnFeedbackPlayed(Tag, Context)`, `OnHUDLayersChanged(Flags)`, `OnLaneDangerChanged(Lane, Level)` |
| Data model | `FFeedbackRow`: tag, sound (+ per-surface map, 2D flag, alert class), Niagara (attach flag), camera shake class + scale + radii, hit stop seconds, hero-only flag, toast text/icon, cooldown (+ per-actor), burst limit/window. `FFeedbackEventContext`: location, direction, instigator, target, surface, `bIsHeavy`, `bTargetArmored`, variant, lane, magnitude, detail. Marker component: kind, icon, normal visibility rule, max distance, show health. Telemetry schema: technical plan §5.4 |
| Failure cases | Missing table/row/asset (R-UXF-06); spam bursts (R-UXF-03); destroyed actors in hit stop/state VFX; time dilation interplay (R-UXF-08); telemetry write failure; pawn change (R-UXF-13); §34.4 cases (R-UXF-26) |
| Performance | Event-driven. Per-frame work allowed only for the stamina bar interpolation. Lane danger 2 Hz; marker visibility 4 Hz. Concurrency + Niagara budgets + burst limit for crowds. Expected markers ≤ ~30 |

## 14. Feedback Contract: master table (GDD §34.5)

Audit baseline for every gate. Feature specs own the detail of their rows; when a feature spec changes a row, update this table in the same change. `—` = none by design. Phase = phase in which the row must pass the audit. Variant rows (`.Armored`, per squad type) are listed with their base tag.

| ID | Feature | State / event | Visual | Audio | UI | Readability / failure rule | Tag | Phase |
|---|---|---|---|---|---|---|---|---|
| FC-01 | FND | No player-facing state | — | — | — | Hosts the `Feedback.*` root (`T-FND-04`) | — | F |
| FC-02 | CMB | Light hit lands | Spark (armored variant: dull sparks) | Impact by surface | — | No hit stop/shake; armored variant distinct by ear | `Feedback.Combat.Hit.Light` (+ `.Armored`) | P0 / P1 armored |
| FC-03 | CMB | Heavy hit lands | Bigger burst (armored variant) | Heavy impact by surface | — | Hit stop + light shake (Hero involved) | `Feedback.Combat.Hit.Heavy` (+ `.Armored`) | P0 |
| FC-04 | CMB | Hero blocks | Shield flash | Shield impact | Stamina drop | Visible even at full stamina | `Feedback.Combat.Block` | P0 |
| FC-05 | CMB | Block break | Break burst, guard-break pose | Break sound | Stamina bar empty flash | Hit stop + shake; distinct from block by ear | `Feedback.Combat.BlockBreak` | P0 |
| FC-06 | CMB | Successful parry (§28.3) | Parry flash at contact | Unique parry sound | — | Longest hit stop; sound never reused | `Feedback.Combat.Parry` | P0 |
| FC-07 | SYN | Poise break → Staggered applied | Stagger burst + state VFX (FC-27) | Crack | — | Hit stop + shake if Hero instigated; burst-limited | `Feedback.State.Staggered.Applied` (`.Removed` optional) | P0 |
| FC-08 | SYN/DEF/SQD | Hit consumed a state bonus | Accent burst | Accent layer | — | Only when the bonus applied (NEW-UXF-10) | `Feedback.Combat.Hit.StateBonus` | P1 |
| FC-09 | CMB | Hero takes damage | Front/back hit reaction, edge flash | Hurt sound | HP drop | Flash ≤ 0.3 s, never covers center | `Feedback.Hero.Damaged` | P0 |
| FC-10 | UXF | Hero low HP | Vignette pulse | Heartbeat sting | HP bar pulse | Once per crossing; stops above re-arm | `Feedback.Hero.LowHealth` | P0 |
| FC-11 | CMB | Action refused (stamina) | — | Soft fail sound | Stamina bar flash | Not confusable with block break | `Feedback.Hero.StaminaInsufficient` | P0 |
| FC-12 | CMB | Dodge | Montage only | Montage whoosh | — | i-frames visible only in debug | — | P0 |
| FC-13 | CMB | Lock-on target | Lock-on marker on target (`WBP_LockOnMarker`) | — | Marker | Hidden in Focus/Spirit/Modal layers | — | P0 |
| FC-14 | CMB | Hero death (§34.4) | Death animation, bars fade | Death sound | HP empty | P3 leads into FC-50; never ends the run (§18.1) | `Feedback.Hero.Death` | P0 |
| FC-15 | CMB | Interact available | — | — | Prompt | Only in range and valid | — | P2 |
| FC-16 | ENM | Attack wind-up | Wind-up pose + weapon flash; heavy looks different | Wind-up whoosh | — | Starts ≥ minimum telegraph time before the hit; never hidden by VFX | `Feedback.Enemy.Telegraph`, `Feedback.Enemy.Telegraph.Heavy` | P0 |
| FC-17 | ENM | Enemy death | Death anim (cheap for Swarm) | Death by class | — | Swarm VFX budget-culled; burst-limited | `Feedback.Enemy.Death` | P0 |
| FC-18 | ENM | Armored / Siege / Boss present | Silhouette + class icon marker | — | Marker | Identified in 1–2 s; none on Swarm | — (marker) | P1 / P2 |
| FC-19 | SQD | Order acknowledged (§28.3) | Ground ping / target outline ~1.5 s; marker pulse | Bark/horn per squad type | Order icon changes | Audible without looking; ≤ 0.3 s after release | `Feedback.Command.Acknowledged` (+ `.Infantry`, `.Archer`) | P1 |
| FC-20 | SQD | Order invalid | Wheel segment flash | Soft error | Wheel | Never closes silently on invalid | `Feedback.Command.Invalid` | P1 |
| FC-21 | SQD | Squad state change (engage, guard, retreat, reform) | Marker order icon | — (spam rule) | Strip icon | Current command state readable (§28.1) | — | P1 |
| FC-22 | SQD | Squad low strength (§28.3) | Marker warning color | Distinct positional cue | Strength pips warning | Once per crossing | `Feedback.Squad.LowStrength` | P1 |
| FC-23 | SQD | Squad reinforced / respawned | Spawn VFX at retreat point | Small cue | Pips refill | — | `Feedback.Squad.Reinforced` | P1 |
| FC-24 | SQD | Squad wiped (§34.4) | Marker greyed | Wipe cue | Strip shows respawn timer | Told even when off-screen | `Feedback.Squad.Wiped` | P1 |
| FC-25 | SQD | Hidden stuck-recovery teleport (§13.6) | — | — | — | Never visible; only off-screen | — | P1 |
| FC-26 | SYN | Armor Broken applied (§28.3) | Shard burst + state loop VFX + icon | Dedicated armor-break sound | — | Armored plates visibly drop (production plan) | `Feedback.State.ArmorBroken.Applied` (`.Removed`) | P1 |
| FC-27 | SYN | Any state active (Staggered, Armor Broken, Marked) | Row icon + color + loop VFX from `DT_CombatStatePresentation`, max 3 by priority | — | — | Identical on all units and sources (§28.2) | — (presentation table) | P1 |
| FC-28 | SYN | Marked applied | Reticle icon loop | Mark ping | — | Prototype source perk/debug only (A-06) | `Feedback.State.Marked.Applied` (`.Removed`) | P1 |
| FC-29 | DEF | Placement placed / denied | Ghost valid/invalid, "blocks lane" indicator | Confirm / deny | Reason text, build count | Player knows why a spot is refused | `Feedback.Build.Placed`, `Feedback.Build.Denied` | P2 |
| FC-30 | DEF | Structure hit | Hit flash, debris per material | Wood/stone impact | HP marker appears, hides after a few s | Which structure is attacked readable in 1–2 s (G2) | `Feedback.Structure.Hit` | P2 |
| FC-31 | DEF | Structure destroyed (§34.4) | Collapse VFX | Collapse sound | Marker removed; toast with lane | Distinct from hit sound | `Feedback.Structure.Destroyed` | P2 |
| FC-32 | DEF | Blocked path opened (§34.4) | Highlight along reopened lane | Breach cue | Lane strip pulse (`PulseLane`) | Lane known even off-screen | `Feedback.Lane.PathOpened` | P2 |
| FC-33 | DEF | Tower critical HP (§28.3) | Cracked look, marker critical | Positional alarm | Marker pulses | Once per crossing; audible off-screen | `Feedback.Structure.Critical` | P2 |
| FC-34 | DEF | Core under attack (§28.3) | Core hit flash | 2D alarm, throttled | Core bar flash + attacker's lane pulse | ≤ 1 cue per throttle window | `Feedback.Core.UnderAttack` | P2 |
| FC-35 | DEF | Tower fire / impact | Ballista bolt tracer; Bombard arc + splash decal sized to radius | Per-role fire / impact | — | Role readable by shape/VFX (§28.2) | `Feedback.Tower.Fire.Ballista/Bombard`, `Feedback.Tower.Impact.Ballista/Bombard` | P2 |
| FC-36 | ZON | Wheel aims at a zone | Zone outline + label | — | Suggested command | Only while aiming | — (ZON UI) | P2 |
| FC-37 | ZON | Squad holds zone, bonus active | Zone decal ally tint | — | Bonus icon on marker/strip | Subtle (§15.4) | — | P2 |
| FC-38 | RUN | Prep / intermission window opens | — | Soft stinger | Phase label + countdown | Readable at a glance | `Feedback.Run.WindowOpen` | P2 |
| FC-39 | RUN | Wave countdown last 10 s | Countdown pulse | Tick per second | Countdown highlight | Heard without looking | `Feedback.Run.WaveCountdown` | P2 |
| FC-40 | RUN | Wave start | Lane direction flash | War horn | "Wave n/5" banner | Distinct from boss horn | `Feedback.Run.WaveStart` | P2 |
| FC-41 | RUN | Wave cleared | — | Victory sting | Toast | Toast never covers Core bar | `Feedback.Run.WaveCleared` | P2 |
| FC-42 | RUN | Run resource gained / spend refused | Counter bump / red flash | Soft tick (throttled) / error | Reason text | Throttle 4/s; refusal always says why | `Feedback.Run.ResourceGained`, `Feedback.Run.CannotAfford` | P3 |
| FC-43 | RUN | Mid-run pressure event (Q-17) | — | Alarm sting | Banner with modifier name | Visible ≥ minimum warning time | `Feedback.Run.PressureEvent` | P3 |
| FC-44 | RUN | Boss incoming | Boss entry marker | Unique boss horn | "Boss" banner + countdown | Different from wave horn | `Feedback.Run.BossIncoming` | P3 |
| FC-45 | DIR | Forecast revealed (§7) | Forecast panel | Soft chime | Low/Med/High per type, icons, lane, modifier | Before every wave (G3); modifier stated (§8.3) | `Feedback.Encounter.ForecastUpdated` | P3 |
| FC-46 | RUN | Core critical HP (§34.4) | Core bar pulses red | Core critical alarm | Warning text | Threshold crossing only | `Feedback.Run.CoreCritical` | P3 (RUN `T-RUN-13`) |
| FC-47 | RUN | Core destroyed | Destruction VFX, camera on Core | Collapse sting | → resolve | — | `Feedback.Run.CoreDestroyed` | P2 |
| FC-48 | RUN | Run won / lost (§33) | — | Music sting | Resolve screen, reason + summary | Loss reason in one line | `Feedback.Run.Victory`, `Feedback.Run.Defeat` | P2 / P3 |
| FC-49 | RUN | Siege Site boundary warning / recovery (§34.4) | Edge vignette / short fade | Warning tone | "Return to the Siege Site" | Clears on return; never looks like a death | `Feedback.Run.BoundaryWarning`, `Feedback.Run.BoundaryRecovered` | P3 |
| FC-50 | CSM | Enter Commander Spirit | Spirit post-process, camera | Spirit tone | Banner, countdown, charges | Commands available at once (§18.2) | `Feedback.Spirit.Enter` | P3 |
| FC-51 | CSM | Respawn soon / respawn | Respawn marker pulse / flash | Rising tone / respawn tone | Countdown highlight; vitals return | HUD rebinds to fresh pawn | `Feedback.Spirit.RespawnSoon`, `Feedback.Spirit.Respawn` | P3 |
| FC-52 | CSM | Revive used / denied | — | Tone / error | Charge count | — | `Feedback.Spirit.ReviveUsed`, `Feedback.Spirit.ReviveDenied` | P3 |
| FC-53 | TFM | Focus enter / exit | Time slow, camera, tactical display on/off | Slow-time mix / exit tone | Meter | Instant cancel (§29.2) | `Feedback.Focus.Enter`, `Feedback.Focus.Exit` | P3 |
| FC-54 | TFM | Meter low / depleted / ready | Meter pulse / flash / glow | Tick / tone / chime | Meter | Player knows exit is coming | `Feedback.Focus.MeterLow`, `.Depleted`, `.Ready` | P3 |
| FC-55 | TFM | Focus denied / restricted | Meter shake / frame color | Error / (boss sting) | Reason / restriction icon | Denial always shows a reason (§12.4) | `Feedback.Focus.Denied`, `Feedback.Focus.Restricted` | P3 |
| FC-56 | PRK | Perk offer | Cards | Chime | Modal 3 cards | Card readable ≤ 3 s | `Feedback.Perk.Offered` (`Feedback.Perk.Rerolled` only if T-PRK-14 is approved) | P3 |
| FC-57 | PRK | Perk chosen | Card flourish | Confirm | Tray icon added | — | `Feedback.Perk.Chosen` | P3 |
| FC-58 | PRK | Perk effect fires | Effect VFX on affected actor | Subtle cue (< alert loudness) | Tray icon pulse | Player links effect to perk | `Feedback.Perk.Triggered` | P3 |
| FC-59 | BOS | Boss spawn | Entry animation | Arrival sting | Boss bar | Boss lane known at once | `Feedback.Boss.Spawn` | P3 |
| FC-60 | BOS | Boss attack telegraph (§22.3) | Long wind-up, weapon glow | Wind-up roar | — | Every attack telegraphed; heavier = longer | `Feedback.Boss.Telegraph` | P3 |
| FC-61 | BOS | Boss phase change (§28.3) | Roar anim, shake, VFX burst | 2D phase change sound | Banner "Phase 2", bar marker | Cannot be missed off-screen | `Feedback.Boss.PhaseChange` | P3 |
| FC-62 | BOS | Boss summons | Spawn VFX at each point | Summon horn per lane | Lane pulses (`PulseLane`) | Player knows which two lanes | `Feedback.Boss.Summon` | P3 |
| FC-63 | BOS | Focus restricted by phase | — | — | Restriction on Focus meter | Player knows why Focus differs | `Feedback.Boss.FocusRestricted` | P3 |
| FC-64 | BOS | Boss defeated | Death VFX | Victory | Bar hides | — | `Feedback.Boss.Defeated` | P3 |
| FC-65 | UXF | Tag without row (dev) | On-screen warning | — | — | Never silent in dev | — | P0 |
| FC-66 | BOS | Boss reset / bug recovery (§34.4) | Recovery hidden when possible | — | — | Keeps HP/phase; never punishes (BOS R-BOS-15) | — | P3 |
| FC-67 | UXF | Alert toast feed | — | — | Max 3, 3 s, duplicates merged | Never covers center/reticle | — | P1 |
| FC-68 | MET (VS) | Unlock / reward / save states (§24, §34.6) | Resolve cards, saving icon | Stings / error | Messages | Never a silent save loss | `Feedback.Meta.UnlockAcquired`, `.RewardGranted`, `.Saving`, `.SaveFailed`, `.SaveRecovered`, `.SaveReset`, `.SaveTooNew`, `.SettingsApplied`, `.PurchaseRefused` | VS |
| FC-69 | WLD (VS) | World travel and sites (§5) | Site/danger visuals | Stings | Banners, toasts | Provisional | `Feedback.World.SiteInRange`, `.Discovered`, `.DangerArea`, `.GateLocked`, `.SiteLiberated`, `.BossDomainOpened`, `.Travel`, `.HeroRespawn` | VS |
| FC-70 | ECO (VS) | Economy and villagers (§16) | Zone/villager markers | Positional alarms | Toasts, workforce panel | Player knows to protect/evacuate | `Feedback.Economy.RaidIncoming`, `.VillagerAttacked`, `.VillagerDied`, `.ZoneInterrupted`, `.ZoneEvacuated`, `.WorkforceShort`, `.ResourceGained`, `.SpendRefused` | VS |
| FC-71 | CNV (VS) | Conversion (§17) | Channel VFX on target | Apply / expiring | Channel icon, timers | Which layer got the power is visible | `Feedback.Conversion.Applied`, `.Expiring`, `.Expired`, `.Refused`, `.InRange`, `.BuildingAttacked`, `.BuildingDestroyed` | VS |
| FC-72 | ONB (VS) | Tutorial stage / hint / gate (§34.3) | — | Chime | Prompt panel, objective progress | One system at a time | `Feedback.Tutorial.StageStarted`, `.StageCompleted`, `.StageRestart`, `.Hint`, `.ObjectiveProgress`, `.GateOpened`, `.Completed` | VS |
| FC-73 | DIR | Lane first spawn of a wave | Lane arrow flash at lane entry | Lane horn | Lane danger pulse (`PulseLane`) | Player knows which lane activates, even off-screen | `Feedback.Encounter.LaneIncoming` | P2 |
| FC-74 | DIR | Elite spawned | Elite marker on the enemy | Elite sting | — | Elite distinguishable from normal enemies (§28.1) | `Feedback.Encounter.EliteSpawned` | P3 |

UXF's own rows: FC-10, FC-65, FC-67.
