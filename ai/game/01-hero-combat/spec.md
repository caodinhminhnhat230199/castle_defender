# Hero Combat (Warlord): Specification

| | |
|---|---|
| Feature | CMB (`01-hero-combat`) |
| GDD sections covered | §2.1, §9.1–9.8, §19.1, §29.1–29.2, §28.2–28.3 (hero part), §32 Prototype 0, §36 Combat Core DoD |
| Cross-references | §3 anti-goals, §10.2 (states consumed/produced), §18.1 (hero death), §34.1, §34.4, §34.5, §38 |
| Phases | **P0A** (combat feel slice) · **P0B** (defense/lock-on + G0 gate) · **P2** (Interact) · **VS** (provisional polish, re-plan after G3) |
| Assumptions used | A-02 (lock-on + sprint in P0), A-01 (Tactical Focus is TFM/P3, not here) |
| Status | Draft v2 |

Related docs: [technical-plan.md](technical-plan.md), [tasks.md](tasks.md), master plan [§3 G0 checklist](../main_implement_plan.md#gate-checklists).

## 1. Overview

The Warlord is the prototype hero: a third-person sword-and-shield fighter with move, sprint, a 3-hit light chain, a heavy attack, dodge, block, parry, stamina and lock-on. Prototype 0 asks one question: is this combat responsive, readable and satisfying for a few minutes with no army or towers on screen (§2.1, §32 P0)? If not, nothing else gets built (G0).

P0 is delivered internally in two slices to reduce implementation risk without changing the master-plan gate: **P0A** proves movement, attack timing, hit registration and basic feel; **P0B** adds the complete defensive loop, lock-on and feedback hardening. **G0 remains a single gate at the end of P0B.**

Hero combat also produces the first Battlefield Synergy inputs: Heavy and Parry deal poise damage that leads to Staggered (owned by SYN), and Heavy will carry Armor Broken from P1 (§9.3, §9.7, §10.2).

## 2. Player Experience

- **Heavy and committed** (§2.1): attacks have weight; once started they play out until a cancel window. No unconditional cancel.
- **Readable risk / reward** (§2.1, §9.5): block is safe but drains stamina; parry is narrow, punishes a miss, and pays off with an opening.
- **Stamina pressure without sluggishness** (§9.6): stamina gates defensive and heavy actions, not every action.
- **Clear feedback** (§9.8): every hit tells the player what happened (normal, armored, blocked, parried, staggered) through hit stop, reaction, shake, sound and VFX.
- **Fun on its own** (§2.1): the sandbox with one melee enemy holds attention for 3–5 minutes (§36).

## 3. Core Loop

```text
Read enemy telegraph / position
→ Decide: attack, dodge, block, parry, reposition
→ Commit (animation + stamina)
→ Feedback (hit stop, reaction, stamina bar, poise/stagger)
→ Adapt (stamina left, enemy state, lock-on target)
→ Punish openings (Staggered enemy, parry counter window)
```

## 4. Gameplay Rules

Labels follow the GDD. Every [TUNABLE] value lives in `DA_HeroClass_Warlord` (`UHeroClassDefinition`) or in montage notify windows; starting values are in the table at the end of this section.

### 4.1 P0 scope and delivery slices

P0 is one master-plan phase but is implemented in two internal slices. The split is sequencing only; it does not create a new production gate or change GDD ownership.

| Slice | Purpose | Required contents | Exit condition |
|---|---|---|---|
| **P0A — Combat Feel Slice** | Prove the moment-to-moment melee foundation before investing in the complete defensive stack | Move, Sprint, camera, Light ×3, Heavy, Dodge, hit registration, basic stamina plumbing, directional hit reaction, death/reset, attack timing contract, attack rotation assist, combat debug overlay, one simple hostile/test enemy | Light/Heavy/Dodge are responsive and readable; trace/timing bugs are diagnosable; no blocker requires redesign of the action pipeline |
| **P0B — Defense + G0 Slice** | Complete the intended Warlord loop and gather G0 evidence | Full stamina tuning, Block/Block Break, Parry/Counter, Lock-on/switch, complete feedback routing, automated/functional tests, G0 playtest | Every P0 AC passes and G0 receives a KEEP / CHANGE / DELETE decision |

P0A may use placeholder animation/audio/VFX and a minimal hostile as long as timing and hit behavior are testable. P0B must use the same action pipeline; it must not replace P0A with a second combat implementation.

**Verbs and pillar**
- R-CMB-01 (§9.1) [LOCKED]: The prototype Warlord has Move, Sprint, Light Attack, Heavy Attack, Dodge, Block, Parry, Lock-on, Interact, Command Wheel and Tactical Focus. CMB owns all except Command Wheel (SQD), Tactical Focus (TFM, P3 per A-01) and Interact (CMB, P2). Skill/class abilities come only after later gates.
- R-CMB-02 (§2.1, §32 P0) [LOCKED]: Combat must be fun with all RTS/TD layers removed from the test screen. This is the G0 hypothesis.

**Movement and camera**
- R-CMB-03 (§9.1) [LOCKED]: Move is camera-relative third-person movement. Speeds and turn rate are [TUNABLE] data. Facing (user decision, 2026-10-05): the hero always faces the camera yaw and strafes or backpedals, like God of War. S moves backward without turning. Data: `Movement.bFaceCameraDirection` (default true).
- R-CMB-04 (§9.1, A-02) [LOCKED]: Sprint is held to move faster. Starting any combat action ends sprint. Sprint speed is [TUNABLE].
- R-CMB-05 (§9.6) [TUNABLE]: Sprint stamina drain is a data field, default 0 (Assumption, NEW-CMB-02).
- R-CMB-06 (§9.1, §28.1): The third-person camera orbits freely and keeps the hero readable; arm length, lag and lock-on framing are [TUNABLE].

**Commitment, cancel and input**
- R-CMB-07 (§2.1, §9.8) [LOCKED]: Every attack, dodge and parry has commitment. It can only be interrupted by (a) its own cancel window, which lists the actions allowed to cancel it, (b) taking an unblocked hit (R-CMB-32), (c) death.
- R-CMB-08 (§36 "responsive"): A combat press that arrives while the current action cannot be cancelled is buffered for `InputBufferTime` [TUNABLE] and runs at the first window that allows it. Only the latest buffered press is kept.
- R-CMB-09 (§29.2, §3): Combat uses a small set of action-based inputs. No combo needs more than one button per step; no input sequence beyond "press Light again".

- R-CMB-43 (§2.1, §9.8, §36) [LOCKED]: Every montage-driven combat action follows one timing contract. Attacks expose **Startup → Active/Hit → Recovery**, plus optional **Chain**, **Cancel**, **Rotation Assist**, **I-frame** or **Parry** windows as applicable. Exact temporal boundaries live in montage notify windows; they are not duplicated as hard-coded timers.
- R-CMB-44 [LOCKED]: `FCombatActionTiming` is the runtime/debug representation of those authored windows. It may cache/inspect montage timing for validation and debug display, but montage notify windows remain the single source of truth for exact action timing.
- R-CMB-45 [LOCKED]: Missing, overlapping in an invalid way, or impossible required timing windows fail `IsDataValid` for the combat data/montage pair. In non-editor runtime, an invalid action refuses to start and logs one actionable error instead of entering a partial action state.

**Stamina**
- R-CMB-10 (§9.6) [LOCKED]: Stamina is spent by Dodge, Block (stamina damage on blocked hits), Heavy Attack and listed special mobility actions. Light Attack, Move, Lock-on and Interact cost no stamina by default; Light has a cost field that must stay below Heavy's (§9.2). Default 0 (NEW-CMB-02).
- R-CMB-11 (§9.4, §9.6): An action whose stamina cost is higher than current stamina does not start, and the player gets an "insufficient stamina" cue (Assumption, NEW-CMB-02).
- R-CMB-12 (§9.6) [TUNABLE]: Stamina regenerates at `RegenRate` after `RegenDelay` since the last spend, up to `MaxStamina`. While Block is held, regen uses `BlockingRegenMultiplier` (Assumption, NEW-CMB-03).

**Light Attack**
- R-CMB-13 (§9.2) [LOCKED/TUNABLE]: Light is fast, medium/low damage, keeps pressure. It is a 3-hit chain: pressing Light inside the current hit's chain window plays the next hit. The chain resets after hit 3, when the chain window closes without input, or when any other action starts.
- R-CMB-14 (§9.2) [TUNABLE]: Damage, poise damage and stamina cost per chain hit are data. Light poise damage stays lower than Heavy.

**Heavy Attack**
- R-CMB-15 (§9.3) [LOCKED]: Heavy has a clearly readable startup, higher damage than any Light hit, and high poise damage. It costs stamina.
- R-CMB-16 (§9.3, §10.2, §19.1) [LOCKED]: Heavy carries a data list of states to apply on hit. In P0 the list is empty. From P1 the Warlord Heavy applies Armor Broken; the state effect is owned by SYN (T-SYN-02).

**Attack orientation and rotation assist**
- R-CMB-46 (§2.1, §9.1) [TUNABLE]: Light and Heavy may rotate toward a valid hostile during an authored `RotationAssist` window so small aim errors do not make melee feel unreliable. Assist is limited by `AttackAssistMaxAngle`, `AttackAssistDistance` and `AttackAssistRotationRate`; it never teleports/snap-turns the hero outside those limits.
- R-CMB-47 [LOCKED]: In free camera, rotation assist chooses only a hostile near the hero's intended attack direction/camera aim. While locked-on, the locked target is preferred. If no valid target exists, the attack preserves player-facing intent and receives no hidden turn.
- R-CMB-48 [LOCKED]: Rotation assist changes facing only. It does not pull/translate the hero toward a target. Any future attack magnetism that moves the hero requires a separate rule and G0/GDD approval.

**Hit registration**
- R-CMB-17 (§9.8, §2.1): A hero attack only registers hits during its hit window. Each target can be hit at most once per swing. Several targets inside the same swing are each hit once.
- R-CMB-18 (Assumption, NEW-CMB-05): No friendly fire. Hero attacks only affect actors on the hostile team.

**Dodge**
- R-CMB-19 (§9.4) [LOCKED]: Dodge consumes stamina and has an invulnerability (i-frame) window. Exact i-frame timing is [TUNABLE] and authored as a notify window in the dodge montage.
- R-CMB-20 (§9.4) [LOCKED]: Dodge cannot be spammed indefinitely: each dodge costs stamina and has a recovery before the next dodge is allowed.
- R-CMB-21 (§9.4) [LOCKED]: Dodge direction follows movement input relative to the camera. With no input the hero dodges backward relative to its facing. With camera facing (R-CMB-03), the hero keeps its facing and plays the F/B/L/R clip closest to the input (S + Dodge = backward clip).

**Block**
- R-CMB-22 (§9.5) [LOCKED]: While Block is held, hits arriving inside the front block arc lose `BlockDamageReduction` of their damage. The blocked force becomes stamina damage (`Damage × BlockStaminaPerDamage`). Hits from outside the arc are not blocked. Arc width is [TUNABLE] (Assumption, NEW-CMB-03).
- R-CMB-23 (§9.5) [LOCKED]: Block break: if a blocked hit drops stamina to 0, block ends and the hero enters Staggered (`State.Combat.Staggered`, shared state, SYN) for `BlockBreakStaggerDuration` [TUNABLE]. The hero cannot act until it ends.
- R-CMB-24 (§2.1, §9.5): Block has feedback when it absorbs force (blocked-hit reaction, sound, stamina bar change).
- R-CMB-49 (§9.5) [TUNABLE]: A successful block restarts a short `BlockRegenSuppressAfterHit` timer before blocking-regeneration can resume. This prevents permanent guard from becoming the dominant low-risk loop while preserving Block as the safe defensive option.

**Parry**
- R-CMB-25 (§9.5, §2.1) [LOCKED]: Parry is high risk / high reward. It is its own input with a short active window (narrower than Block's always-on guard), followed by a recovery when nothing was parried. During that recovery the hero cannot block or dodge (Assumption: separate input, NEW-CMB-01).
- R-CMB-26 (§9.5, §9.7, §10.2) [LOCKED]: The **first valid hostile hit** landing inside the parry window and the front arc is parried: the hero takes no damage, the attacker takes `ParryPoiseDamage` through the poise pipeline (which leads to Staggered if poise breaks, SYN), and the hero gets a Counter Window (NEW-CMB-01 for the "stagger or vulnerability window" reading).
- R-CMB-27 (§9.5) [TUNABLE]: During the Counter Window the next hero attack is a parry counter: it is flagged as a counter (`bIsParryCounter`) and its damage is multiplied by `CounterDamageMultiplier`. If a counter montage is set in data, Light or Heavy pressed in the window plays it.
- R-CMB-28 (§9.5, §23.2): A successful parry is a Battlefield Synergy source. It raises a public "parry succeeded" event that later consumers (perks, squads) can listen to. No consumer is built in P0.
- R-CMB-50 [LOCKED]: Base Warlord Parry is **consumed by the first successful parry in that action**. Further hits during the remaining authored active window resolve normally (Block does not automatically take over). Multi-parry is reserved for an explicit future perk/trait and is not baseline behavior.

**Lock-on**
- R-CMB-29 (§9.1, A-02): Lock-on toggles onto the best hostile target: alive, within `LockOnRange`, in line of sight, closest to screen centre. A switch input moves to the next target left or right on screen.
- R-CMB-30 (A-02): Lock-on releases when the target is farther than `LockOnBreakDistance` or out of line of sight longer than `LockOnLOSGraceTime`. If the target dies, lock-on moves to the next valid target in range, else releases.
- R-CMB-31 (A-02, §28.1): While locked, the camera keeps the target in frame and the hero faces the target while moving (strafe). Sprint and Dodge still follow input direction. A lock-on marker shows on the target.

**Taking damage and death**
- R-CMB-32 (§2.1, §9.8): An unblocked, unparried, non-evaded damaging hit normally interrupts the hero's current action and plays a directional hit reaction (front/back). Actions may resist interruption only through the explicit interrupt-resistance rule R-CMB-51; there is no implicit hyper armor.
- R-CMB-33 (§18.1) [LOCKED]: Hero death does not lose the run. At 0 HP the hero raises one death event and stops accepting input. In the P0/P1 sandbox the hero respawns after a short delay (sandbox-only behavior). From P3 the Commander Spirit flow owns what happens next (CSM T-CSM-01).

- R-CMB-51 (NEW-CMB-04) [TUNABLE/OFF BY DEFAULT]: The combat pipeline supports data-defined interrupt resistance for authored action windows (initial consumer candidate: Heavy committed phase). Default P0 value is **off**, preserving current behavior. If enabled during G0 tuning, the hit carries an interrupt strength/category and the action window carries the resistance needed to continue; damage still applies unless another rule prevents it.
- R-CMB-52 (§10.2, §38) [LOCKED]: Shared gameplay states such as `State.Combat.Staggered` are authoritative in `UCombatStateComponent` (SYN). CMB must not own an independent Staggered timer/boolean. `CanStartAction()` and action interruption query the shared state; CMB may expose a derived presentation/debug label but never a second source of truth.

**Combat feel**
- R-CMB-34 (§9.8) [LOCKED]: Big impacts get short hit stop; hits cause hit reactions; camera shake is controlled; impact sound varies by target armor/material; VFX differ for normal / armored / parry / stagger hits; animation commitment holds. Photorealism is not required, readability and impact are.
- R-CMB-53 (§34.5) [LOCKED]: CMB emits one **combat-resolution event** for every resolved incoming/outgoing hit attempt that reaches combat resolution (hit, blocked, block break, parried, evaded, ignored-by-state as applicable). UXF receives a separate **feedback event** only when an audiovisual/UI response should play. An evade is therefore visible in telemetry/debug resolution data but may intentionally emit no UXF feedback event.
- R-CMB-54 (§34.1, §36) [LOCKED]: `game.debug.Combat 1` shows a compact combat debugger for the hero: current action, timing phase/windows, buffered input, stamina, lock-on/assist target, shared combat states and active defensive window. `game.debug.CombatTrace 1` additionally draws attack sweeps/hit points and per-swing already-hit actors. Debug UI is development-only.

**Input**
- R-CMB-35 (§29.1) [LOCKED]: Keyboard + mouse is primary. Every combat verb is an Enhanced Input action so gamepad can be mapped later without code change.
- R-CMB-36 (§29.2): Combat bindings never share a key with squad selection, Command Wheel or Tactical Focus. Movement stays active while the Command Wheel is open (§29.2 "usable while moving"; Command Wheel owned by SQD).

**Data**
- R-CMB-37 (§34.1, §38): All CMB tunables live in `UHeroClassDefinition` (one asset per class) or montage notify windows. None are hard-coded. Changing a value in the asset during PIE takes effect on the next action without a code change.
- R-CMB-38 (§19.1) [LOCKED]: Warlord identity in P0 = heavy melee, block/parry, stagger. Warlord class traits (proximity buff, rally, melee synergy, armor break, charge/hold-line support) arrive later: armor break in P1 via SYN, the rest provisional at VS.

### 4.2 P2 scope

- R-CMB-39 (§9.1): Interact: the hero focuses the best interactable object in range and in front of the camera, a prompt shows its action name, and pressing Interact triggers it once. The object defines what happens (first consumer: DEF build zones, T-DEF-07). Interact cannot start during attack, dodge, parry, hit reaction or Staggered.

### 4.3 VS scope (provisional, re-validate after G3)

- R-CMB-40 (§19.1, §32 VS "Warlord more complete"): Warlord proximity buff: allied squads near the hero get a light stat bonus ("Army fights with you"). Values and stat list are design work at VS.
- R-CMB-41 (§19.1): Rally and charge/hold-line support are Warlord traits with no rules in the GDD yet. They need a design spike before any implementation (NEW-CMB-06).
- R-CMB-42 (§32 VS): Combat feel polish with production animation: montages retimed to the windows tuned at G0.

### 4.4 Tunable starting values

Placeholders so the first data asset can be authored. **Not GDD numbers.** Tune at G0, one value at a time.

| Value | Start | Lives in |
|---|---|---|
| MaxHealth | 200 | `UHeroClassDefinition.Health` |
| Jog / Sprint speed | 450 / 700 cm/s | `.Movement` |
| Block move speed multiplier | 0.5 | `.Block` |
| MaxStamina / RegenDelay / RegenRate | 100 / 0.8 s / 30 per s | `.Stamina` |
| BlockingRegenMultiplier / Sprint drain | 0.5 / 0 per s | `.Stamina` |
| Light chain damage / poise / stamina | 10-10-14 / 5-5-10 / 0 | `.LightChain[i]` |
| Heavy damage / poise / stamina | 30 / 40 / 25 | `.Heavy` |
| Dodge stamina | 20 | `.Dodge` |
| Dodge i-frames | ~0.10–0.35 s of a ~0.6 s montage | `AM_Warlord_Dodge_*` notify window |
| BlockDamageReduction / BlockStaminaPerDamage / block arc | 0.8 / 1.0 / **140°** | `.Block` |
| BlockRegenSuppressAfterHit / BlockBreakStaggerDuration | **0.6 s** / 1.2 s | `.Block` |
| Parry window / whiff recovery | ~0.15 s / ~0.5 s | `AM_Warlord_Parry` notify windows |
| Parry stamina / ParryPoiseDamage | 0 / 60 | `.Parry` |
| CounterWindow / CounterDamageMultiplier | 1.0 s / 1.5 | `.Parry` |
| InputBufferTime | 0.2 s | `.Input` |
| AttackAssistMaxAngle / Distance / RotationRate | **35° / 400 cm / 720°/s** | `.AttackAssist` |
| Interrupt resistance | Off by default | per-action data / authored notify window |
| LockOnRange / BreakDistance / LOS grace | 1500 / 2000 cm / 1.0 s | `.LockOn` |
| Sandbox respawn delay | 3 s | `BP_SandboxGameMode` variable |
| InteractRange (P2) | 250 cm | `.Interact` |

## 5. Player Actions

| Action | Meaning | Phase |
|---|---|---|
| Move / Sprint | Position, close distance, escape | P0A |
| Light Attack (×3 chain) | Keep pressure, cheap damage | P0A |
| Heavy Attack | Break poise, punish openings, (P1) break armor | P0A |
| Dodge | Avoid a hit through timing or distance | P0A |
| Block (hold) | Safe defense paid with stamina | P0B |
| Parry | Risky defense that creates an opening | P0B |
| Lock-on / switch target | Focus one enemy, keep it framed | P0B |
| Interact | Use a world object (build zone, …) | P2 |

## 6. Success / Failure Conditions

- Exchange success: hero lands hits, avoids or absorbs incoming hits, staggers the enemy.
- Exchange failure: hero gets hit (hit reaction), guard breaks (Staggered), runs out of stamina, dies.
- Hero death is never a run loss (§18.1). Sandbox: respawn after delay. P3+: Commander Spirit Mode (CSM).
- Feature success = P0A exits without an action-pipeline redesign, P0B completes the intended defensive loop, and G0 passes at the end of P0B (master plan §3).

## 7. Scope

### In Scope
- P0A: `AHeroCharacter` (Warlord), locomotion + sprint, camera, light chain, heavy, dodge, basic stamina plumbing, attack timing contract/validation, rotation assist, hit registration and hit dispatch, hero hit reaction and death/reset, combat debugger, `L_CombatSandbox`, one simple hostile/test enemy.
- P0B: full stamina tuning, block, block break, parry + counter window, lock-on, combat-resolution/feedback routing, sandbox enemy respawner, combat Automation/Functional Tests, G0 gate playtest.
- P2: Interact verb and interface.
- VS (provisional): proximity buff, design spike for rally/charge/hold-line, animation polish.

### Out of Scope
- Poise math and Staggered/Armor Broken/Marked state logic (SYN). Enemy AI, attacks and stagger behavior (ENM). Hit stop, camera shake, VFX/SFX playback (UXF). HUD bars (UXF T-UXF-02). Command Wheel (SQD). Tactical Focus (TFM). Death → Commander Spirit flow (CSM). Perk effects (PRK). Gamepad bindings (launch production). Other classes (Ranger etc., master plan §9).

## 8. Anti-Goals

- Not a Souls clone (§3): no stamina on every action, no punishing input lag, no deep weapon/moveset trees.
- No dozen-button combos (§3): one chain, one heavy, one parry.
- No hero so strong the army is pointless (§1.1 "not strong enough to win alone"); tuning for P1+ must keep this true.
- No unconditional animation cancel (§2.1).
- No photorealistic combat requirement (§9.8).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Project, input base, `AHeroPlayerController`, `IMC_Combat` | FND T-FND-01, T-FND-06 | Character and input |
| `FCombatHit`, `UHealthComponent`, `UCombatStateComponent`, team interface | FND T-FND-05 | Shared damage contract (D-05) |
| Definition pattern, tuning settings | FND T-FND-07 | `UHeroClassDefinition` |
| Cheats, CVars, test dummy, test harness | FND T-FND-09, T-FND-10 | Debug and verification |
| Poise → Staggered | SYN T-SYN-01 | Heavy/parry results, block break state |
| Melee enemy that attacks with telegraph and reacts to stagger | ENM T-ENM-01..04 | Something to fight in the sandbox |
| Feedback playback, HUD HP/stamina, telemetry | UXF T-UXF-01, -02, -03, -08 | Combat feel and G0 evidence |

Provided to others: hit dispatch (`UCombatLibrary::DeliverHit`, used by ENM/SQD/DEF for every hit), hero death event (CSM), parry/block/hit-landed events (PRK, SQD), Interact interface (DEF, later CNV/WLD).

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Two enemies hit the hero inside one parry window | The first valid hit is parried and consumes the Parry; the second hit resolves normally unless another defense/state applies |
| Hit arrives the same frame block is released | Hit resolves against the state at hit time (released = not blocked) |
| Stamina 5, blocked hit costs 30 stamina | Stamina goes to 0 → block break; the breaking hit is still reduced |
| Dodge pressed with stamina below cost | Dodge does not start; insufficient-stamina cue; press is not buffered |
| Lock-on target dies mid-swing | Swing finishes; lock moves to next valid target or releases |
| Lock-on target goes behind a wall briefly | Lock holds for `LockOnLOSGraceTime`, then releases |
| Hero hit during hit stop | Hit queues in game time; resolves normally when time resumes |
| Hero dies during an attack's hit window | Hit window closes, no further hits from that swing |
| Montage interrupted by another system (death, hit reaction) | Action state returns to a valid state; no stuck "Attacking" |
| Swing hits a target with no health component | Ignored, no feedback |
| Same target overlapping trace on several frames | Damaged once per swing (R-CMB-17) |
| Low frame rate / hitch | Hit traces sweep between frames so fast swings do not skip targets |
| Required montage timing notify is missing | `IsDataValid` reports the action/montage/window; runtime action refuses to start and logs once |
| SYN applies/removes `State.Combat.Staggered` while CMB is idle | CMB action eligibility follows `UCombatStateComponent` immediately; no local Staggered timer is started |
| Heavy has interrupt resistance disabled | Any normal unblocked damaging hit interrupts as in R-CMB-32 |
| Heavy interrupt resistance enabled for its committed window | Hits below the configured interrupt threshold deal damage but do not force HitReact during that window; stronger hits still interrupt |
| Rotation assist has no target inside cone/range | No assist; player-facing attack direction is preserved |
| Player leaves the Siege Site (§34.4) | Not CMB; RUN boundary rule (Q-16) |

## 11. Acceptance Criteria

### P0
- AC-CMB-01: In `L_CombatSandbox` the Warlord spawns, moves camera-relative, sprints and orbits the camera; editing speeds in `DA_HeroClass_Warlord` during PIE changes behavior after the `ReloadHeroTuning` cheat, with no code change.
- AC-CMB-02: Three Light presses inside the chain windows play hits 1→2→3; waiting past the window resets to hit 1; a 4th press after hit 3 starts hit 1 after recovery.
- AC-CMB-03: A Dodge pressed during a non-cancelable part of a Light swing runs when the cancel window opens if that is within `InputBufferTime`; otherwise it is dropped.
- AC-CMB-04: A swing that overlaps a target for several frames damages it exactly once; a swing through two targets damages each once; allies are never damaged.
- AC-CMB-05: Heavy shows a readable wind-up, deals more damage and poise damage than any Light hit, costs stamina, and staggers the P0 melee enemy within the number of hits implied by data.
- AC-CMB-06: Dodge spends stamina; a hit landing inside the i-frame window does no damage; the same hit outside the window does; with stamina below cost, Dodge does not start and the cue plays.
- AC-CMB-07: Dodge goes in the input direction relative to camera; with no input it goes backward.
- AC-CMB-08: A frontal hit on a blocking hero is reduced by `BlockDamageReduction` and drains `Damage × BlockStaminaPerDamage` stamina; the same hit from behind is not reduced.
- AC-CMB-09: When a blocked hit empties stamina, block drops, the hero has `State.Combat.Staggered` for `BlockBreakStaggerDuration`, cannot act, and the block-break feedback plays.
- AC-CMB-10: A hit inside the parry window: hero takes 0 damage, attacker receives `ParryPoiseDamage` (P0 melee enemy becomes Staggered), parry feedback with its own sound plays (§28.3), and the next hero hit within `CounterWindow` is flagged counter with multiplied damage. A parry with no incoming hit leaves the hero unable to block or dodge for the recovery.
- AC-CMB-11: Lock-on picks the hostile target closest to screen centre in range; switch moves left/right; lock releases past break distance or after LOS grace; target death moves lock to the next target or releases; a marker shows on the locked target.
- AC-CMB-12: With interrupt resistance disabled (the P0 default), an unblocked hit interrupts the current action with a front or back hit reaction; at 0 HP the death event fires once, input stops, and the sandbox respawns the hero after the delay.
- AC-CMB-13: Every resolved hit attempt raises exactly one `FCombatResolutionEvent` with the correct result/context. Outcomes that require presentation (light/heavy hit, blocked, block break, parried, hero damaged, etc.) raise exactly one matching UXF feedback event; a successful evade is recorded by combat resolution/debug but may remain presentation-silent.
- AC-CMB-14: Every combat verb is an Input Action in `IMC_Combat`; no key overlaps the keys reserved for Command Wheel / Tactical Focus in the FND key map.
- AC-CMB-15: Stamina Automation Spec passes (spend, reject, regen delay, regen rate, blocking multiplier, clamp).
- AC-CMB-16: All combat Functional Tests in `L_Test_HeroCombat` pass from the command-line runner.
- AC-CMB-17: G0 gate playtest record exists in `ai/game/playtests/` with every G0 checklist item answered and a KEEP / CHANGE / DELETE decision.

**Additional P0 hardening criteria**  
(IDs 18–19 remain assigned to P2/VS below for compatibility with existing references.)

- AC-CMB-20: Each Light/Heavy/Dodge/Parry montage required by its action passes timing validation. `game.debug.Combat 1` visibly transitions through the authored timing phases/windows; removing a required notify makes validation fail and the action refuse to start instead of becoming stuck.
- AC-CMB-21: During an authored rotation-assist window, an eligible target inside the configured angle/range causes a bounded turn toward that target; a target outside either limit causes no hidden snap or translation. Locked-on attacks prefer the locked target.
- AC-CMB-22: Applying `State.Combat.Staggered` through `UCombatStateComponent` blocks action start for exactly the shared state's lifetime. CMB contains no independent Staggered duration/timer that can disagree with SYN.
- AC-CMB-23: A combat-resolution event is recorded for a successful evade, but no UXF feedback event is required for that outcome; Light/Heavy/Block/BlockBreak/Parry/HeroDamaged still emit the expected feedback event exactly once.
- AC-CMB-24: When two valid hostile hits arrive during one Parry action, only the first successful hit is parried and raises `OnParrySucceeded`; the Parry is then consumed and the later hit resolves normally.
- AC-CMB-25: Interrupt resistance is data-driven and defaults off. With it off, Heavy is interrupted by a normal unblocked hit. In the test configuration with a committed Heavy window enabled, a below-threshold hit deals damage without forcing HitReact, while an above-threshold hit interrupts.
- AC-CMB-26: `game.debug.CombatTrace 1` shows the active sweep and already-hit set; a fast swing under a low-FPS test still hits an intersected target once and never more than once per swing.

### P2
- AC-CMB-18: Near two interactables, the prompt shows the one closest to the camera aim; pressing Interact triggers it once; with none in range no prompt shows; Interact is ignored during attack, dodge, parry, hit reaction and Staggered.

### VS (provisional)
- AC-CMB-19: Allied squads inside the proximity radius show the buff and get the data-defined stat bonus; leaving the radius removes it.

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| A-02 | Lock-on and Sprint in P0 | Yes (master plan) | Assumption |
| NEW-CMB-01 | §9.5 says parry "causes stagger **or** opens a vulnerability window". Which, and is Parry its own input or a timed Block press? | Own input (high-risk whiff recovery). First valid hit consumes the Parry. Parry deals `ParryPoiseDamage`: low-poise enemies stagger, high-poise enemies do not, and the hero always gets a Counter Window | REQUIRED, confirm at G0 |
| NEW-CMB-02 | Stamina costs for Light, Sprint, Parry; can an action start below its cost? | Light/Sprint/Parry cost 0 (fields exist); no start below cost | REQUIRED, tune at G0 |
| NEW-CMB-03 | Block arc and regen while blocking (GDD silent) | Start at front **140°**, regen ×0.5 while blocking, and suppress blocking regen for **0.6 s** after absorbing a hit | REQUIRED, tune at G0 |
| NEW-CMB-04 | Do hits interrupt every hero action (no hyper armor on Heavy)? | Pipeline supports authored interrupt resistance, but it is **off by default**. G0 decides whether Heavy enables it during a committed window and what hit threshold breaks it | REQUIRED, decide at G0 |
| NEW-CMB-05 | Friendly fire between hero and own squads | None | REQUIRED for P1 |
| NEW-CMB-06 | Rules for Warlord rally and charge/hold-line support (§19.1 lists them, no rules) | Not built; design spike at VS (T-CMB-18) | FUTURE |
| NEW-CMB-07 | Proposed default keys: Light LMB, Heavy RMB, Block (hold) Left Ctrl, Parry E, Dodge Space, Sprint (hold) Left Shift, Lock-on Middle Mouse, switch target Mouse Wheel, Interact F | Final key map owned by T-FND-06; rebinding UI is VS/launch | IMPROVEMENT |
| NEW-CMB-08 | How much attack rotation assistance is needed to make melee reliable without feeling magnetic? | Start at 35° / 400 cm / 720°/s, rotation-only; tune at G0 | REQUIRED, tune at G0 |
| NEW-CMB-09 | Should baseline Parry ever handle multiple simultaneous attackers? | No. First successful parry consumes the action; multi-parry is reserved for an explicit perk/trait | DESIGN DECISION, revisit only if G0 proves necessary |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Warlord locomotion, camera, combat actions, stamina, lock-on, hero-side hit resolution (i-frames, block, parry), outgoing hit registration, hero damage/death events, Interact (P2) |
| Inputs | Enhanced Input actions from `IMC_Combat`; incoming `FCombatHit` via `UCombatLibrary::DeliverHit`; `DA_HeroClass_Warlord`; montage notify windows; target state (Staggered, Armor Broken, Marked) from `UCombatStateComponent` (P1+) |
| Outputs | `FCombatHit` to hostile targets; parry poise damage to attackers; `FCombatResolutionEvent` for resolved hit attempts; UXF feedback events (`Feedback.*`) only for outcomes with presentation; HUD-facing values (HP via `UHealthComponent`, stamina, lock-on target) |
| State | CMB-owned action state (Idle, LightAttack, HeavyAttack, Dodge, Block, Parry, HitReact, Dead); light chain index; buffered input; counter window timer; stamina current/regen timer; lock-on/assist target; hit set per swing; consumed-parry flag; focused interactable (P2). Shared states such as Staggered live only in `UCombatStateComponent` (SYN) and are queried by CMB |
| Events | `OnActionStateChanged`, `OnCombatResolved(FCombatResolutionEvent)`, `OnHitLanded(Target, Result)`, `OnParrySucceeded(Attacker)`, `OnBlockBroken`, `OnStaminaChanged`, `OnStaminaSpendFailed`, `OnLockOnTargetChanged`, `OnHeroDeath`; plus `UHealthComponent.OnDamaged/OnDeath` |
| Data model | `UHeroClassDefinition`: Health (max), Movement (jog, sprint, turn rate), Camera (arm, lag, lock-on framing), Stamina (max, regen delay, rate, blocking multiplier, sprint drain), LightChain[3] and Heavy (`FHeroAttackData`: montage, damage, poise damage, stamina cost, trace radius, applied states, state duration, state damage multipliers P1, interrupt-resistance config), AttackAssist (max angle, distance, rotation rate), Dodge (directional montages, stamina, recovery), Block (reduction, stamina per damage, arc, regen-suppress-after-hit, break stagger duration, hit/break montages), Parry (montage, stamina, poise damage, counter window, counter multiplier, optional counter montage), HitReact (front/back montages), Death montage, LockOn (range, break distance, LOS grace), Input (buffer time), CombatStateConfig (hero poise off), Interact (range, aim cone, P2). Exact action timing remains in montage notify windows; `FCombatActionTiming` is derived for validation/debug |
| Failure cases (§34.4) | Hero death → death event, sandbox respawn (P0) / CSM (P3). Lock-on target destroyed → retarget/release. Montage interrupted externally → state reset. Missing montage or notify in data → `IsDataValid` error, action refuses to start with a log line. Tuning asset missing → hero falls back to class defaults and logs an error. Player leaving Siege Site, boss reset: not CMB |
| Performance | One hero. Traces only while a hit window is open. Lock-on candidate query only on acquire/switch plus a low-rate validity check. No per-frame work outside movement, camera and open windows |

## 14. Feedback Contract (GDD §34.5)

Feedback tags are proposed leaves under `Feedback.*`; rows live in `DT_Feedback` (UXF T-UXF-01/03). Final tag names are owned by UXF.

`FCombatResolutionEvent` and UXF feedback are intentionally separate contracts: combat resolution/telemetry may record outcomes that should remain audiovisually silent (for example an evade). The table below lists presentation behavior only.

| State / event | Visual | Audio | UI | Readability rule | Tag |
|---|---|---|---|---|---|
| Light hit lands | Hit spark (normal or armored variant), target reaction | Impact by target material | — | Distinct from heavy at gameplay distance | `Feedback.Combat.Hit.Light` |
| Heavy wind-up | Clear startup pose | Wind-up whoosh (montage notify) | — | Readable ≥ 0.3 s before impact | (montage only) |
| Heavy hit lands | Bigger spark, short hit stop, light camera shake | Heavy impact by material | — | Hit stop only on heavy/large impacts (§9.8) | `Feedback.Combat.Hit.Heavy` |
| Hit on armored target | Armored spark variant (dull, metal) | Metal impact | — | Must differ from flesh hit (§9.8) | context `bTargetArmored` on Hit.* |
| Hit blocked by hero | Shield flash, block-hit reaction | Shield impact | Stamina bar drops | Always visible even at full stamina | `Feedback.Combat.Block` |
| Block break | Break burst, guard-break pose | Break sound | Stamina bar flashes empty | Distinct from normal block | `Feedback.Combat.BlockBreak` |
| Parry success | Parry flash on weapon contact | Dedicated parry sound (§28.3) | — | Unique; never reused by another event | `Feedback.Combat.Parry` |
| Parry whiff | Recovery pose | none | — | Player must see the recovery | (montage only) |
| Dodge | Dodge animation | Whoosh (montage notify) | — | i-frames shown only in debug view | (montage only) |
| Insufficient stamina | — | Soft fail sound | Stamina bar flashes | Must not be confused with block break | `Feedback.Hero.StaminaInsufficient` |
| Hero damaged | Hit reaction, screen-edge hint | Hero hurt sound | HP bar drops | Front/back reaction readable | `Feedback.Hero.Damaged` |
| Hero death | Death animation | Death sound | HP empty | Clear end state; respawn cue in sandbox | `Feedback.Hero.Death` |
| Lock-on acquired / switched / lost | Marker on target | Optional click | Marker widget | Marker never hides target state icons | (UI widget) |
| Enemy staggered by hero | Stagger VFX | Stagger sound | — | Owned by SYN | `Feedback.State.Staggered.Applied` |
| Prompt (P2) | — | — | Interact prompt with action name | One prompt at a time | (UI widget) |


## 15. Draft v2 Change Summary

This revision hardens P0 implementation sequencing and combat feel without adding new player verbs.

- Split implementation sequencing into **P0A Combat Feel Slice** and **P0B Defense + G0 Slice**; G0 remains one master-plan gate.
- Added a single-source **combat action timing contract** and validation/debug representation (`FCombatActionTiming`).
- Added bounded **attack rotation assist** (rotation only; no hidden translation/magnet pull).
- Added data-driven **interrupt resistance / Heavy hyper-armor support**, disabled by default and explicitly left for G0 tuning.
- Made `UCombatStateComponent` / SYN the sole authority for shared states such as Staggered.
- Changed baseline Parry from multi-parry to **first successful hit consumes the Parry**.
- Reduced the provisional Block arc from 180° to 140° and added a provisional post-block regeneration suppression window; both remain G0 tunables.
- Split **combat-resolution events** from **UXF feedback events**, removing the semantic conflict around silent evades.
- Expanded `game.debug.Combat` into a required combat timing/state debugger and added trace visualization.
- Added acceptance criteria and edge cases for all new contracts while preserving existing rule/AC IDs where possible.
- Updated the Player Actions phase column to show P0A vs P0B sequencing; this is an internal implementation split only.
