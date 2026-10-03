# Commander Spirit (CSM): Specification

| | |
|---|---|
| Feature | CSM, folder `09-commander-spirit` |
| GDD sections covered | §18.1–18.4, §34.4 (Hero death), §29.2, §28.1, §25.1 (respawn pressure axis), §37 (Q-06, Q-07), §32 P3, §36 Full Run DoD |
| Phases | P3 |
| Status | Draft v1 |
| Related | RUN (run phases, `ARunPlayerState`, site bounds), SQD (Command Wheel, `UCommandComponent`), DEF (`UStructurePlacementComponent`), TFM (tactical overlay, Focus exit), CMB (`OnHeroDeath`, hero pawn), UXF (HUD, feedback, telemetry) |

## 1. Overview

When the Hero dies, the player does not wait at a black screen. The view rises into a commander presentation: the player keeps commanding squads, can still place defenses, watches the whole battlefield with the tactical overlay, and prepares for the respawn. A respawn timer [TUNABLE] that grows slightly with death count (and optionally wave) brings the Hero back; a limited revive charge allows an instant return. Hero death never loses the run (§18.1).

Player value: death is a tempo loss and a change of role, not a stop. Pillar B (Command at a Glance) stays active even without the Hero body.

## 2. Player Experience

- "I fell, but my army still listens." The player switches from fighter to commander within ~2 s.
- Pressure stays: the Hero is out of the fight, so squads and towers must hold. Repeated deaths cost more time (§18.3, §25.1 respawn pressure).
- Hope: a revive charge is a rare, deliberate comeback tool, not a purchase (§18.4).
- Readability: from above, the player reads squads, lanes, tower HP and the respawn point at a glance (§28.1).

## 3. Core Loop

```text
Hero dies → death presentation → commander view
→ Observe (overlay, lanes, Core HP) → Decide → Command squads / place defense
→ Watch countdown, optionally spend a revive charge → Respawn → back to combat
```

## 4. Gameplay Rules

All rules are P3.

- **R-CSM-01** (§18.1) [LOCKED]: Hero death never loses or pauses the run. CSM never resolves the run; only RUN does (R-RUN-06).
- **R-CSM-02** (§18.2) [LOCKED]: On Hero death the death presentation plays for `DeathPresentationSeconds` ([TUNABLE], placeholder 1.5 s), then the view blends (`SpiritBlendSeconds`, placeholder 0.75 s) into the commander presentation: a raised, angled tactical camera starting above the death location. Control is available as soon as the blend ends.
- **R-CSM-03** (§18.2) [LOCKED]: The player keeps commanding squads with the same Command Wheel flow (§11.4). Aim comes from the commander camera's screen centre. Guard, Attack and Retreat are available for every squad. Follow Hero is unavailable while the Hero is dead; squads already on Follow switch to Guard at their current position (SQD NEW-SQD-05).
- **R-CSM-04** (§18.2) [LOCKED; scope of "some interactions" OPEN → NEW-CSM-3]: Limited tower/defense interaction: build placement works from the commander camera with the normal placement rules, zones and costs (DEF R-DEF-34). Hero-proximity Interact, including repair (DEF T-DEF-25), is unavailable.
- **R-CSM-05** (§18.2, §28.1) [LOCKED]: Watch the battlefield: the camera pans, rotates (yaw) and zooms anywhere inside the Siege Site bounds (RUN R-RUN-22). The tactical overlay is on (squad icons, lane pressure, enemy class icons, tower HP, predicted routes, tactical zones; TFM T-TFM-03). Core HP and the forecast stay on the HUD.
- **R-CSM-06** (§18.2) [LOCKED]: Prepare for respawn: the respawn countdown, the respawn point marker and the revive charge count with its prompt are always visible. Spirit mode is never a countdown-only screen.
- **R-CSM-07** (§18.3, A-10, Q-06) [TUNABLE]: Respawn time = clamp(`RespawnBaseSeconds` + `RespawnPerDeathSeconds` × prior deaths this run + `RespawnPerWaveSeconds` × (wave index − 1), `RespawnBaseSeconds`, `RespawnMaxSeconds`). Placeholders: base 10 s, per death 2 s, per wave 0 s (A-10 uses death count only; the wave term exists because §18.3 names both), max 20 s. Values in `UGameTuningSettings` (Respawn category). No GDD numbers exist; formula is final only after playtest (Q-06).
- **R-CSM-08** (§18.3; respawn location Assumption NEW-CSM-1): When the timer ends, the Hero respawns at the Siege Site respawn point (`APlayerStart` tagged `HeroRespawn`, near the Core) with full HP and stamina and no combat states. Normal combat control returns. The respawn waits while the Command Wheel is open; an open placement preview is cancelled.
- **R-CSM-09** (§18.4, A-10, Q-07) [TUNABLE]: Each run starts with `StartingReviveCharges` (placeholder 1). In spirit mode the player may spend one to respawn immediately at the respawn point. Charges live on `ARunPlayerState`. Rewards or perks may add charges through one API ("rare reward for instant revive", §18.4); no prototype content is required to use it.
- **R-CSM-10** (§18.4) [TUNABLE direction]: No Gold or run-resource revive in the prototype. Adding one later requires playtest evidence that one death does not remove all comeback ability.
- **R-CSM-11** (§34.4): Death cleanup happens in the death frame: an open Command Wheel closes without issuing an order; a placement preview is cancelled; Tactical Focus force-exits (TFM R-TFM-15); lock-on is released (CMB); the dead Hero stops being a valid target for enemies.
- **R-CSM-12** (§18.1, §6.2): The respawn timer runs in every run phase. The PerkChoice UI works in spirit mode. At Resolve the timer stops and no respawn happens.
- **R-CSM-13** (§29.2, D-12): Spirit mode uses `IMC_CommanderSpirit` with device-agnostic actions (pan, rotate, zoom, Command Wheel, build, revive). Tactical Focus is not available in spirit mode (Assumption NEW-CSM-2, aligned with TFM).
- **R-CSM-14** (§34.2, D-07): Death count, revive charges, perks and run stats live on `ARunPlayerState` and survive death and respawn. Command authority stays on the player controller (SQD R-SQD-10).
- **R-CSM-15** (§18.3, §18.4, §37): Telemetry records each death (wave, phase, killer unit tag, computed respawn time), time spent dead, commands and placements issued while dead, and revive charges used, so Q-06 and Q-07 can be answered with data.

## 5. Player Actions

| Action | Input (action, KBM default proposed) | Notes |
|---|---|---|
| Pan camera | `IA_SpiritPan` (WASD) | Camera-yaw relative |
| Rotate camera | `IA_SpiritRotate` (mouse X) | Yaw only, pitch fixed |
| Zoom | `IA_SpiritZoom` (mouse wheel) | Command Wheel context has priority while open (SQD uses the wheel to cycle commands) |
| Command squads | `IA_CommandWheel` (same as combat) | Guard / Attack / Retreat |
| Place a structure | DEF build actions (`IMC_Build`) | Returns to spirit mode on exit |
| Spend revive charge | `IA_Revive` | Only when charges > 0 |
| Choose a perk | PRK UI | If PerkChoice opens while dead |

## 6. Success / Failure Conditions

| Situation | Result |
|---|---|
| Hero dies | Spirit mode; run continues |
| Timer ends | Respawn |
| Revive spent | Immediate respawn, charge −1 |
| Core destroyed while dead | Run lost (RUN rule, not caused by death) |
| Run won while dead | Run won; no respawn |
| Success of the feature | G3: Hero death stays playable (§36); players report having meaningful things to do while dead |

## 7. Scope

### In Scope
Death → spirit flow, commander camera pawn, commanding from spirit mode, placement from spirit mode, respawn timer formula, respawn, revive charges, spirit HUD, overlay reuse, feedback and post-process, telemetry, Functional Tests, G3 checks.

### Out of Scope
Hero death animation and death event (CMB T-CMB-11), squad Follow → Guard conversion (SQD), overlay rendering (TFM), placement rules (DEF), repair (DEF T-DEF-25, VS), Gold revive (R-CSM-10), choosing a respawn location (NEW-CSM-1), spawn protection (NEW-CSM-4), Warden revive support (§19.4 DEFERRED), co-op revive (§3).

## 8. Anti-Goals

- Not a spectator screen: if a playtester only watches the countdown, the feature failed.
- Not a full RTS mode: no box select, no per-soldier orders, no new commands (§3, §11.2).
- Not a free power: spirit mode does not slow time and does not allow Hero-only actions.
- No paid revive (§18.4).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| `AHeroCharacter::OnHeroDeath`, death montage, input ignore on death | CMB T-CMB-11 | Entry trigger |
| `UCommandComponent` on the controller, view-target aim, Follow → Guard on Hero death | SQD T-SQD-04, T-SQD-05, T-SQD-06 | R-CSM-03 |
| `UStructurePlacementComponent` on the controller | DEF T-DEF-07 | R-CSM-04 |
| Run phase, `ARunPlayerState`, site bounds | RUN T-RUN-01, T-RUN-07 | R-CSM-05, R-CSM-12, R-CSM-14 |
| Tactical overlay request API | TFM T-TFM-03 | R-CSM-05 |
| Focus force-exit on Hero death | TFM T-TFM-01 | R-CSM-11 |
| World markers, HUD shell, feedback, telemetry | UXF T-UXF-02, T-UXF-04, T-UXF-01, T-UXF-08 | HUD, §14, R-CSM-15 |
| Mapping context switching, settings, cheats | FND T-FND-06, T-FND-07, T-FND-09 | Input, data, `KillHero` |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Death while Tactical Focus is active | Focus exits in the death frame (time scale 1.0), then normal spirit flow |
| Death with Command Wheel open | Wheel closes, no order issued |
| Death during placement preview | Preview cancelled, no structure placed, nothing spent |
| Timer ends while the Command Wheel is open in spirit mode | Respawn waits until the wheel closes |
| Timer ends during placement preview in spirit mode | Preview cancelled, then respawn |
| Revive pressed during the death presentation | Ignored until spirit mode starts |
| Second death right after respawn | Counts as a new death; timer uses the higher prior-death count |
| PerkChoice opens while dead | Perk UI usable; timer keeps running |
| Run resolves while dead | Timer stopped, no respawn, resolve screen |
| Respawn point occupied by enemies | Respawn anyway (no spawn protection, NEW-CSM-4); observe in playtest |
| Respawn point blocked by geometry or a structure | Spawn with collision adjustment; DEF build zones should exclude the respawn point (note for level layout) |
| No `HeroRespawn` start in the map | Fall back to the GameMode default player start; log a warning |
| Site bounds missing | Camera not clamped; log a warning |
| All squads wiped while dead | Wheel shows no squads (SQD); camera, overlay, placement and revive still work |
| Boss phase change while dead | No special handling; BOS feedback plays |

## 11. Acceptance Criteria

- **AC-CSM-01**: `KillHero` during a wave → the commander camera is active and accepts input within `DeathPresentationSeconds + SpiritBlendSeconds` (≤ 2.5 s with defaults); run phase and result unchanged.
- **AC-CSM-02**: In spirit mode, Guard, Attack and Retreat can be issued to each of 3 squads through the Command Wheel and are executed; Follow Hero is not offered.
- **AC-CSM-03**: A squad on Follow when the Hero dies holds its position as Guard.
- **AC-CSM-04**: The camera reaches every corner of the Siege Site and cannot leave its bounds; zoom stays within limits.
- **AC-CSM-05**: The tactical overlay is visible during spirit mode and hidden after respawn.
- **AC-CSM-06**: A structure placed from spirit mode follows the same rules and cost as a normal placement; no Interact prompt appears while dead.
- **AC-CSM-07**: With default data, consecutive deaths give respawn times 10, 12, 14, … capped at 20 s; an Automation Spec covers the formula, including the per-wave term and the cap.
- **AC-CSM-08**: After respawn the Hero is at the respawn point with full HP and stamina, no combat states, combat input active and HUD bars bound to the new pawn.
- **AC-CSM-09**: With 1 charge, pressing Revive respawns within 0.5 s and leaves 0 charges; with 0 charges it plays the denied feedback and nothing else happens.
- **AC-CSM-10**: Death during Focus restores time scale to 1.0 in the death frame; death with the wheel open issues no order.
- **AC-CSM-11**: Resolve while dead: no respawn occurs; the resolve screen shows.
- **AC-CSM-12**: A perk can be chosen while dead.
- **AC-CSM-13**: Telemetry contains `hero_death`, `hero_respawn` (time dead, revive used) and per-spirit-period command/placement counts.
- **AC-CSM-14** (§36): In the G3 playtest, Hero death never stops a run from being playable, and testers answer "yes" to "Did you have something useful to do while dead?" (pass rule set in T-CSM-10).

## 12. Open Questions / Assumptions

| ID | Item | Default used |
|---|---|---|
| A-10 / Q-06 | Respawn timer formula | Base + per death (+ per wave, default 0), capped (R-CSM-07) |
| A-10 / Q-07 | Instant revive: token or Gold | Token / revive charge (R-CSM-09) |
| NEW-CSM-1 | Respawn location. GDD does not say. Default: one authored respawn point near the Core; no player choice. | REQUIRED to confirm at G3 |
| NEW-CSM-2 | Tactical Focus in spirit mode. Default: unavailable (the view is already tactical; avoids free slow time). | Confirm with TFM NEW-TFM-3 |
| NEW-CSM-3 | Which "tower/defense interactions" (§18.2) are allowed. Default: build placement only; no Interact/repair. | Revisit when repair lands (T-DEF-25) |
| NEW-CSM-4 | Spawn protection after respawn. Not in GDD. Default: none. | IMPROVEMENT if playtest shows spawn deaths |
| Placeholders | Death presentation 1.5 s, blend 0.75 s, base 10 s, +2 s/death, +0 s/wave, max 20 s, 1 starting charge, respawn warning 3 s | No GDD values; all in `UGameTuningSettings` |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Run the Hero death → Commander Spirit → respawn flow; own the respawn timer and revive charges rules; keep commanding and placement usable while dead |
| Inputs | `OnHeroDeath` (CMB); run phase and wave index (`ARunGameState`); site bounds (RUN); `IA_Spirit*`, `IA_Revive`; tuning settings |
| Outputs | Possession change (hero ↔ spirit pawn), input mode change, overlay request (TFM), respawn via GameMode, `Feedback.Spirit.*` events, telemetry |
| State | `ESpiritState` (Alive, Dying, Spirit, Ended), respawn end game time, deferred-respawn flag on `UCommanderSpiritComponent`; `HeroDeaths`, `ReviveCharges` in `FRunPlayerData` (RUN struct) |
| Events | `OnSpiritStateChanged(State)`, `OnRespawnScheduled(EndGameTime)`, `OnReviveChargesChanged(New)` (on `ARunPlayerState`) |
| Data model | `UGameTuningSettings` Respawn category: `DeathPresentationSeconds`, `SpiritBlendSeconds`, `RespawnBaseSeconds`, `RespawnPerDeathSeconds`, `RespawnPerWaveSeconds`, `RespawnMaxSeconds`, `StartingReviveCharges`, `RespawnWarningSeconds`, spirit camera values (pan speed, zoom min/max, pitch, yaw speed) |
| Failure cases (§34.4) | Hero death (this feature), death during Focus / wheel / placement, resolve while dead, missing respawn point, missing bounds, blocked spawn |
| Performance | One pawn and one widget while dead; timers only; the spirit pawn ticks for camera movement only while possessed |

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Hero death (`Feedback.Hero.Death`, CMB) | Death montage | Death sound | Hero bars fade | Owned by CMB; CSM starts after it |
| Enter spirit (`Feedback.Spirit.Enter`) | Camera rise + desaturated post-process on the spirit camera | Whoosh + muffled battle mix | "Commander Spirit" banner, countdown, charges, reticle | Must read as "you are dead, still in command", never as a pause |
| Respawn soon (`Feedback.Spirit.RespawnSoon`, last 3 s) | Respawn point marker pulses | Rising tone | Countdown highlight | Player can finish the current order |
| Respawn (`Feedback.Spirit.Respawn`) | Light pillar at the respawn point; camera returns to third person | Respawn sting | Hero bars return; spirit UI hides | Hero location obvious after the camera returns |
| Revive used (`Feedback.Spirit.ReviveUsed`) | Same as respawn, stronger flash | Distinct horn | Charge count −1 | Different from a timed respawn |
| Revive denied (`Feedback.Spirit.ReviveDenied`) | Charge icon shakes | Error blip | "No revive charges" | Always says why |
| Order issued while dead | SQD order marker | SQD acknowledgement (§28.3) | SQD wheel feedback | Same feedback as when alive |
| Respawn point (persistent while dead) | World marker at `HeroRespawn` | — | Marker with countdown | Visible at any zoom level |
