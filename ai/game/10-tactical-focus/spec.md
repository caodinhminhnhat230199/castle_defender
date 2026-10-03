# Tactical Focus (TFM): Specification

| | |
|---|---|
| Feature | TFM, folder `10-tactical-focus` |
| GDD sections covered | §12.1–12.4, §3 (anti-goal: Focus must not become the dominant way to play), §9.1 (Hero verb), §2.2, §28.1, §29.1–29.2, §33 (13:00 split pressure), §37 (Q-01, Q-02) |
| Decisions / assumptions | D-12 (`IMC_TacticalFocus`), D-13 (global time dilation, camera/UI compensate), A-01 (built in P3) |
| Phases | P3 |
| Status | Draft v1 |
| Related | SQD (Command Wheel), ZON (zones), DEF (routes, tower HP), UXF (markers, lane danger, HUD, telemetry), CSM (overlay reuse, death exit), BOS (restriction hook), RUN (phase) |

## 1. Overview

Tactical Focus is a short, limited slow-time view. While the player holds the Focus input, time slows to ~20–30%, the camera rises and zooms out, and an overlay shows squads, lane pressure, enemy classes, tower HP, predicted routes and tactical zones. The player issues squad commands with the normal Command Wheel, then releases to return to third-person combat instantly. A Focus Meter (~5 s) limits use; it refills only in normal play and cannot be exploited by toggling.

Player value: a few seconds of clarity for a big decision (§2.2 "tactical view supports big decisions, not continuous micro"), for example reordering squads when two lanes are attacked at once (§33 13:00).

## 2. Player Experience

- Clarity under pressure: in 1–2 s the player reads where squads are, which lane is in danger and which tower is about to fall (§28.1).
- Short and precious: the meter forces the player to plan before entering and to leave quickly.
- Third person stays the main way to play (§12.1, §3). Focus is a tool, not a mode.
- Instant control: releasing the input gives full speed and full control in the same frame (§29.2).

## 3. Core Loop

```text
Notice pressure (audio, HUD) → Hold Focus → Read overlay → Command Wheel order(s)
→ Release (or meter empties) → Fight in third person while the meter recharges
```

## 4. Gameplay Rules

All rules are P3 (A-01).

- **R-TFM-01** (§12.2, §9.1, A-01) [LOCKED]: Tactical Focus is a Hero verb used by holding `IA_TacticalFocus`. Releasing exits. It is not a toggle.
- **R-TFM-02** (§12.2, §12.3, D-13, Q-02) [TUNABLE]: While Focus is active, global time scale = `FocusTimeScale` (default 0.25; GDD "khoảng 20–30%"). Value in `UGameTuningSettings` (Focus category).
- **R-TFM-03** (§12.4) [LOCKED]: Focus never fully pauses the game. Settings validation rejects `FocusTimeScale` outside 0.1–0.9 and warns outside 0.2–0.3.
- **R-TFM-04** (§12.3, Q-01) [TUNABLE]: The Focus Meter holds `MeterCapacitySeconds` of real (undilated) time, default 5 s ("tối đa khoảng 5 giây sử dụng liên tục"). It drains 1 s per real second while active and starts each run full.
- **R-TFM-05** (§12.4) [LOCKED]: The meter never refills while Focus is active.
- **R-TFM-06** (§12.3) [TUNABLE]: The meter recharges only in normal play: Focus inactive and Hero alive. Recharge starts `RechargeDelaySeconds` after the last exit (placeholder 1.5 s) at `RechargeRatePerSecond` meter-seconds per real second (placeholder 0.33, full in ~15 s). No GDD values exist; "exact duration/recharge phải playtest".
- **R-TFM-07** (§12.4) [LOCKED rule; mechanism is Assumption NEW-TFM-1]: Toggle spam cannot exploit recharge: (a) every exit restarts the recharge delay, so tapping never recharges; (b) entering requires at least `MinMeterToEnterSeconds` (placeholder 1.0 s); (c) when the meter empties, Focus exits and stays locked until the meter is back at the entry threshold; (d) after any forced exit, re-entry needs a new press of the input.
- **R-TFM-08** (§29.2) [LOCKED]: Instant cancel. On release (or any exit), the time scale returns to 1.0 and normal input returns in the same frame. The camera return blend (`CameraExitBlendSeconds`, placeholder 0.1 s) never delays control.
- **R-TFM-09** (§12.2, D-13) [LOCKED; values TUNABLE]: The camera rises and zooms out to a tactical view above and behind the Hero (height, distance, pitch, FOV in data). Camera movement and the enter blend (`CameraEnterBlendSeconds`, placeholder 0.2 s) run in real time so the view stays responsive while the world is slow.
- **R-TFM-10** (§12.2, §28.1) [LOCKED]: The overlay shows squad icons, lane pressure, enemy class icons, tower HP, predicted routes and tactical zones. Everything on it must be readable within 1–2 s at the tactical camera distance.
- **R-TFM-11** (§12.2, §2.2, §33) [LOCKED]: All four squad commands can be issued during Focus through the normal Command Wheel (§11.4). Aim uses the tactical camera's screen centre. If Focus ends (release or empty meter) while the wheel is open, Focus exits and the wheel stays open in normal time.
- **R-TFM-12** (§29.2; Assumption NEW-TFM-2): During Focus, Hero combat actions (attacks, dodge, block/parry, lock-on, interact) are not mapped. Move and Look stay available, at dilated world speed.
- **R-TFM-13** (§12.4) [LOCKED]: A boss phase may restrict Focus (shorter capacity, weaker slowdown, slower recharge) but never disable it. Restrictions are clamped so entry is always possible: effective capacity ≥ `MinMeterToEnterSeconds`, effective recharge > 0, effective time scale < 1. Restrictions end with the phase or the boss.
- **R-TFM-14** (§12.1, §3) [LOCKED anti-goal; targets TUNABLE]: Focus must stay a minority of combat time. Two guards: (a) a design ceiling computed from data, `Capacity / (Capacity + RechargeDelay + Capacity / RechargeRate)` ≤ 25% (≈ 23% with defaults), checked by an Automation Spec; (b) playtest telemetry: median share of combat time in Focus ≤ 15% (review default), measured with T-UXF-08.
- **R-TFM-15** (§18.2; modal steps Assumption NEW-TFM-4): Focus force-exits and entry is blocked when the Hero dies, when the run enters PerkChoice or Resolve, and while Commander Spirit Mode is active.
- **R-TFM-16** (§18.2; Assumption NEW-TFM-3, aligned with CSM NEW-CSM-2): Tactical Focus is not available in Commander Spirit Mode. The meter does not recharge while the Hero is dead and keeps its value through respawn.
- **R-TFM-17** (D-13): TFM is the only system that writes global time dilation in gameplay. Gameplay timers (abilities, AI, waves, run steps) stay on game time and slow with the world. Focus meter, Focus camera and Focus UI use real time.
- **R-TFM-18** (§29.1, §29.2) [LOCKED]: Focus uses device-agnostic Enhanced Input actions so it can be mapped to gamepad later. Bindings are data.

## 5. Player Actions

| Action | Input (action) | Notes |
|---|---|---|
| Enter / stay in Focus | Hold `IA_TacticalFocus` | Needs ≥ entry threshold |
| Exit Focus | Release `IA_TacticalFocus` | Instant |
| Read overlay | — | Automatic in Focus |
| Rotate the view / aim | `IA_Look` | Yaw follows control rotation; pitch fixed |
| Move the Hero (slowed) | `IA_Move` | World is dilated |
| Command squads | `IA_CommandWheel` | Same flow as in combat |

## 6. Success / Failure Conditions

| Condition | Result |
|---|---|
| Enter with meter ≥ threshold | Focus active |
| Enter with meter below threshold | Denied feedback, nothing else |
| Meter reaches 0 | Forced exit, locked until threshold |
| Feature success | Players use Focus for big decisions (split pressure) and Focus share stays under the R-TFM-14 targets |
| Feature failure | Players keep Focus up whenever possible (share near the ceiling), or never use it |

## 7. Scope

### In Scope
`UTacticalFocusComponent` (meter, dilation, exits, restrictions), tactical camera, `UTacticalOverlayComponent` (overlay requests shared with CSM), predicted route display, Focus input context, Focus meter HUD and feedback, telemetry and acceptance metric, Functional Tests, time-dilation side-effect audit, overlay performance check, G3 checks.

### Out of Scope
Command Wheel itself (SQD), marker widgets and lane danger computation (UXF), zone actors (ZON), route computation (DEF), boss phase content that applies a restriction (BOS), onboarding step "Use Tactical Focus" (ONB, VS, §34.3), toggle/accessibility mode (NEW-TFM-5), click-to-select squads or any RTS selection (§3), pause.

## 8. Anti-Goals

- Focus is not the main mode (§3, §12.1). No upgrade, perk or setting may raise the design ceiling above the R-TFM-14 limit without a gate decision.
- No full pause, ever (§12.4).
- No RTS micro inside Focus: no box select, no per-soldier orders, no extra commands (§2.2, §3).
- No hidden refill (kills, perks, pickups) in the prototype.

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Mapping-context switching, settings, CVars, cheats | FND T-FND-06, T-FND-07, T-FND-09 | Input, data |
| `AHeroCharacter`, `OnHeroDeath`, camera-relative movement | CMB T-CMB-01, T-CMB-11 | Exit on death, camera |
| Command Wheel with view-target aim; no time slow of its own (R-SQD-07) | SQD T-SQD-05, T-SQD-06 | R-TFM-11 |
| Squad delegates/markers exposed for the overlay (R-SQD-28) | SQD | Squad icons |
| World markers with a global tactical display mode; lane danger values | UXF T-UXF-04, T-UXF-07 | R-TFM-10 |
| Zone actors with marker/icon | ZON T-ZON-01 | Zones on the overlay |
| Lane route query + route invalidation events | DEF T-DEF-05, T-DEF-06 | Predicted routes |
| Structure HP markers | UXF T-UXF-04 / DEF T-DEF-02 | Tower HP |
| Run phase | RUN T-RUN-01 | R-TFM-15, combat time |
| Telemetry log | UXF T-UXF-08 | R-TFM-14 |
| Boss phase events (apply/clear restriction) | BOS T-BOS-01 | R-TFM-13 consumer |

Consumers: CSM (overlay request API), BOS (restriction API).

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Hero dies during Focus | Exit in the death frame (time scale 1.0), then CSM flow |
| Focus released while the Command Wheel is open | Focus exits; wheel stays open in normal time |
| Meter empties while the Command Wheel is open | Same as release |
| Focus held at wave clear when PerkChoice opens | Forced exit; perk UI opens at normal time |
| Run resolves during Focus | Forced exit before the resolve screen |
| Boss restriction applied during Focus | Applies at once; if the meter exceeds the new capacity it is clamped; Focus continues if above 0 |
| Restriction removed | Capacity and rates return to normal; meter value kept |
| Hit stop during Focus | Hit stop must not write global time dilation (R-TFM-17); uses per-actor dilation |
| Key still held after a forced exit | No re-entry until released and pressed again |
| Window loses focus while holding | Meter drains to 0 and exits; Enhanced Input key flush on focus loss (verify) |
| Entering Focus mid-attack | The attack continues in slow motion; no new combat actions until exit |
| Holding Focus during Intermission | Allowed; step timers slow slightly (RUN accepts, logs real time) |
| Many enemies on screen | Overlay must stay readable; if Swarm icons clutter, aggregation is NEW-TFM-6 |
| Debug cheat `SetTimeDilation` used during Focus | Developer only; TFM restores 1.0 on exit |

## 11. Acceptance Criteria

- **AC-TFM-01**: Holding Focus with a full meter sets global time dilation to `FocusTimeScale` in the press frame; releasing restores 1.0 in the release frame and returns combat input.
- **AC-TFM-02**: Holding continuously drains a full meter in 5.0 s real time (± 0.1 s) and then exits.
- **AC-TFM-03**: No refill while active; after exit, refill starts after the recharge delay and follows the rate.
- **AC-TFM-04**: Tapping Focus every 0.5 s for 20 s never increases the meter.
- **AC-TFM-05**: With the meter below `MinMeterToEnterSeconds`, a press is denied with feedback; after depletion, entry is locked until the threshold is reached; a held key never re-enters by itself.
- **AC-TFM-06**: The camera reaches the tactical pose within `CameraEnterBlendSeconds` real time while the world is at 0.25.
- **AC-TFM-07**: The overlay shows all six element types listed in R-TFM-10 in Focus and hides them on exit (unless CSM requested it).
- **AC-TFM-08**: All four commands can be issued to each squad during Focus; aim follows the tactical camera centre; releasing Focus with the wheel open keeps the wheel open at normal time.
- **AC-TFM-09**: Attack/dodge/block inputs do nothing during Focus; Move and Look work.
- **AC-TFM-10**: A boss restriction (capacity × 0.5, time scale 0.5) changes the meter and slowdown while applied; a restriction requesting capacity 0 is clamped so entry still works.
- **AC-TFM-11**: Hero death, PerkChoice and Resolve each force-exit Focus and block entry; Focus is never available in Commander Spirit Mode.
- **AC-TFM-12**: Automation Spec: the design ceiling from current settings is ≤ 25%.
- **AC-TFM-13**: Telemetry per wave and per run contains Focus entries, denied entries, forced exits, Focus real seconds, combat real seconds, Focus share and orders issued in Focus.
- **AC-TFM-14** (§3, §12.1): G3 playtest: median Focus share of combat time ≤ 15% and no tester above 25% (review defaults, T-TFM-12).

## 12. Open Questions / Assumptions

| ID | Item | Default used |
|---|---|---|
| A-01 | Focus built in P3 | All rules P3 |
| Q-01 | Exact max duration | 5 s, data |
| Q-02 | Exact slow-time scale | 0.25, data |
| NEW-TFM-1 | Anti toggle-spam mechanism (delay restart, entry threshold, depletion lock, new press). GDD only states the rule. | REQUIRED to confirm at G3 |
| NEW-TFM-2 | Combat actions unavailable during Focus; Move/Look allowed. | Confirm in playtest |
| NEW-TFM-3 | No Focus in Commander Spirit Mode; meter frozen while dead. | Aligned with CSM NEW-CSM-2 |
| NEW-TFM-4 | Focus blocked during PerkChoice and Resolve (modal UI). | Low risk |
| NEW-TFM-5 | Toggle mode for accessibility. Not in GDD. | FUTURE (VS settings) |
| NEW-TFM-6 | Aggregate Swarm icons per lane if the overlay clutters at the enemy cap. | IMPROVEMENT after T-TFM-11 |
| Placeholders | Recharge delay 1.5 s, rate 0.33/s, entry threshold 1.0 s, enter blend 0.2 s, exit blend 0.1 s, meter-low warning 1.0 s, share targets 15% / 25% | No GDD values; all in `UGameTuningSettings` (targets in the G3 review) |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Limited slow-time tactical view: meter, global time dilation, tactical camera, overlay, command access, anti-exploit, restriction hook, telemetry |
| Inputs | `IA_TacticalFocus` (Started/Completed), Hero alive/death, run phase, spirit state, boss restriction requests, settings |
| Outputs | Global time dilation, input mode switch (`TacticalFocus` ↔ `Combat`), view target, overlay requests (UXF marker mode, routes), `Feedback.Focus.*`, telemetry |
| State | `FTacticalFocusMeter` (current, capacity, recharge delay remaining, locked flag), `bActive`, active restriction, overlay request reasons |
| Events | `OnFocusStateChanged(bool)`, `OnFocusMeterChanged(Current, Capacity)`, `OnFocusRestrictionChanged`, `OnTacticalOverlayChanged(bool)` |
| Data model | `UGameTuningSettings` Focus: `FocusTimeScale`, `MeterCapacitySeconds`, `RechargeDelaySeconds`, `RechargeRatePerSecond`, `MinMeterToEnterSeconds`, `MeterLowWarningSeconds`, camera (height, distance, pitch, FOV, enter/exit blend), `MaxDesignDutyCycle` (0.25). `FTacticalFocusRestriction` (`CapacityMultiplier`, `TimeScaleOverride`, `RechargeRateMultiplier`) authored in boss phase data |
| Failure cases | Hero death during Focus, modal steps, restriction extremes, stuck dilation, input focus loss, other writers of global dilation |
| Performance | Component ticks only while active or recharging; overlay marker count at the enemy cap is the main cost (T-TFM-11); route display rebuilt only on route invalidation |

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Enter (`Feedback.Focus.Enter`) | Desaturate + outline post-process on the tactical camera; camera rise | Slow-time whoosh, low-pass mix | Meter bar highlights and drains | Must read as "slowed", never as "paused" |
| Active | Overlay elements on | Low-pass mix held | Meter value | Overlay readable in 1–2 s (§28.1) |
| Meter low (`Feedback.Focus.MeterLow`, ≤ 1 s left) | Meter pulses | Heartbeat tick | — | Player knows the exit is coming |
| Exit by release (`Feedback.Focus.Exit`) | Colors return; camera snaps back | Reverse whoosh, mix restored | Meter shows recharge delay | Must not delay control |
| Forced exit, empty (`Feedback.Focus.Depleted`) | Meter flashes empty | Distinct "drained" sound | Lock icon until threshold | Different from a normal exit |
| Entry denied (`Feedback.Focus.Denied`) | Meter shakes | Short error blip | — | Says "not enough meter" without text spam |
| Ready again (`Feedback.Focus.Ready`) | Meter glow when above threshold after a lock | Soft chime | Lock icon removed | Quiet; once per lock |
| Restricted by boss (`Feedback.Focus.Restricted`) | Meter frame changes color, reduced capacity marked | Boss sting (BOS owns the phase sound) | Restriction icon | Player sees the limit, not a broken system |
| Order issued in Focus | SQD order marker | SQD acknowledgement (§28.3) | SQD wheel | Same as normal |
