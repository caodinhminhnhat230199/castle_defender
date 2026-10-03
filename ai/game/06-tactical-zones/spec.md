# Tactical Zones: Specification

| | |
|---|---|
| Feature | ZON (`06-tactical-zones`) |
| GDD sections covered | §15 (all), §13.2 (tactical zone state in transitions), §12.2 (zones shown in Tactical Focus), §33 (zones in the example run) |
| Cross-references | §2.3, §5.3, §11.4, §13.3–13.5, §27.1, §28.1, §34.1, §34.4 |
| Phases | **P2** (core, part of Prototype 2 scope "Tactical Zones cơ bản") · **P3** Tactical Focus visibility check |
| Gate | Contributes checks to G2 (`T-DEF-20`) |
| Status | Draft v1 |

## 1. Overview

A Tactical Zone is a hand-placed area on a Siege Site (Bridge, Gate, High Ground, Chokepoint, Resource Camp, Rally Point). Aiming anywhere inside it with the Command Wheel turns a Guard order into "Hold <Zone>": the squad picks a good anchor in the zone, forms up facing the lane, keeps a zone-sized leash and fights what enters the zone. Holding some zones gives a light bonus. Zones let the player command fast in third-person without pixel-perfect aim (§15.1) and are one of the ways the player shapes the battlefield (§2.3).

## 2. Player Experience

- "Infantry, hold the bridge" in one aim + release; the squad positions itself better than a raw ground click would.
- Zones are readable landmarks of the battlefield the player plans around in Prep (§33 00:00–01:00).
- Bonuses are a nudge, not a rule: using a zone is good, ignoring one is still viable (§15.4).
- When a defense collapses, falling back to the next zone is one order (§33 17:00).

## 3. Core Loop

```text
See zones on the site → aim at a zone with the wheel → "Hold <Zone>" → squad anchors, faces lane
→ enemies enter zone → squad engages inside zone leash → zone clear → squad reforms at anchor
```

## 4. Gameplay Rules

### P2 scope

| ID | Source | Label | Rule |
|---|---|---|---|
| R-ZON-01 | §15.1 | [LOCKED] | Aiming at any point inside a zone's area counts as aiming at the zone. No exact click needed. |
| R-ZON-02 | §15.2 | — | Zone types: Bridge, Gate, High Ground, Chokepoint, Resource Camp, Rally Point (`Zone.*` tags). A type is data; adding one is a new data asset + tag, no code. No other types without intake (master plan §10). |
| R-ZON-03 | §5.3, §27.1 | [LOCKED] | Zones are authored by hand per Siege Site. Never procedural. |
| R-ZON-04 | §15.3, §11.4 | — | With the wheel open, aiming at a zone pre-selects **Guard** and shows the zone's hold label (e.g. "Hold Bridge"). Only Guard takes a location, so Guard is the zone command for every squad type; type differences come from anchor choice (R-ZON-06) and bonuses (R-ZON-11). |
| R-ZON-05 | §11.4 | — | Context priority: an enemy under the reticle wins over the zone (more specific), the zone wins over plain ground. Follow and Retreat ignore zones. |
| R-ZON-06 | §15.3 | — | The AI chooses the anchor: from the zone's authored anchor points, the squad takes a free anchor of its preferred role (Melee for Infantry/Spearman, Ranged for Archer), else any free anchor, else it stands behind the nearest holder (NEW-ZON-06). |
| R-ZON-07 | §15.3, §13.3 | [LOCKED] | The squad forms its normal formation at the anchor (SQD formation rules). |
| R-ZON-08 | §15.3, §13.5 | [LOCKED] | The squad keeps a leash: the zone's leash radius replaces the squad's guard leash while it holds the zone. |
| R-ZON-09 | §15.3 | — | The squad faces the lane: each anchor has an authored facing that points toward the incoming lane. |
| R-ZON-10 | §13.2 | Assumption NEW-ZON-04 | Zone state (Unheld / Held / Contested) feeds squad transitions: an enemy inside the zone area makes the holding squad Engage (the zone area is the squad's Guard Zone, Infantry rule 1, R-SQD-16); when the zone is clear the squad reforms at its anchor. |
| R-ZON-11 | §15.4 | [TUNABLE] | Light bonuses while held: High Ground → ranged range (`Stat.Army.RangedRange`); Chokepoint → Infantry block efficiency (`Stat.Army.BlockEfficiency`, NEW-ZON-01); Rally Point → faster reform (`Stat.Army.ReformSpeed`, NEW-ZON-02). Bridge, Gate, Resource Camp: no bonus by default. Magnitudes in `DA_Zone_*`. |
| R-ZON-12 | §15.4 | [TUNABLE] | No bonus is big enough to make zone use mandatory. Starting guidance: each bonus ≤ ~20 %. Checked in G2. |
| R-ZON-13 | §15.4, R-PRK-01 | — | Bonuses are applied only through the shared stat modifier query (`T-PRK-02`), scoped to the holding squad. No zone-specific stat code. |
| R-ZON-14 | §15.4 | Assumption NEW-ZON-03 | A bonus applies only while a squad holds the zone (Guard order with this zone as context). It is removed when the order changes, the squad is wiped, or the zone is released. Standing inside without the order gives nothing. |
| R-ZON-15 | §33, §28.1 | [LOCKED] | Zones are readable in normal third-person play (subtle marker + outline) so the player sees them from the start of the run, and highlighted when aimed with the wheel. |
| R-ZON-16 | §33 17:00 | — | "Retreat the squad to the next Tactical Zone" is a Guard order on that zone. Moving there follows SQD MoveToOrder rules (engages only when attacked). The Retreat command still means "return to Core". |
| R-ZON-17 | §34.1 | [LOCKED] | Zone content is data: `UTacticalZoneDefinition`. Level instances only add area shape and anchors. |

### P3 scope

| ID | Source | Label | Rule |
|---|---|---|---|
| R-ZON-18 | §12.2 | [LOCKED] | Tactical Focus shows tactical zones, and zones can be aimed from the tactical camera with the same rules (TFM builds the overlay and Focus commands). |

### 4.1 Default zone data (placeholders)

| Zone | Hold label | Typical anchors | Bonus (holding squad) |
|---|---|---|---|
| Bridge | Hold Bridge | Melee at bridge mouth, Ranged behind | none |
| Gate | Hold Gate | Melee in opening, Ranged behind | none |
| High Ground | Hold High Ground | Ranged on the edge, Melee at the ramp | Archer `RangedRange` ×+0.15 |
| Chokepoint | Hold Chokepoint | Melee in the narrow, Ranged behind | Infantry `BlockEfficiency` +0.15 |
| Resource Camp | Defend Camp | Melee + Ranged | none (economy meaning arrives with ECO, VS) |
| Rally Point | Rally | 2–3 neutral anchors | all squads `ReformSpeed` ×+0.30 |

All magnitudes are placeholders (not from the GDD), live in `DA_Zone_*`, and are tuned in G2.

## 5. Player Actions

| Action | Result |
|---|---|
| Aim the wheel reticle into a zone | Zone highlights; wheel shows "Hold <Zone>", Guard pre-selected |
| Release | Squad takes an anchor, moves, forms facing the lane, holds |
| Order the squad elsewhere | Hold released, bonus removed |

No new input. Everything goes through the SQD Command Wheel.

## 6. Success / Failure Conditions

- Success: the squad reaches its anchor, faces the lane, engages enemies entering the zone and reforms afterwards; bonus visible on its marker.
- Failure: no reachable anchor → order rejected with Invalid feedback (SQD). Zone use never fails the run.
- Feature failure (G2 input): players do not use zones, or feel forced to use them → CHANGE (bonus or placement).

## 7. Scope

### In Scope
- P2: `ATacticalZone` + `UTacticalZoneDefinition`, six zone definitions, zone context in the wheel, hold behavior (anchor, formation, leash, facing, zone-area engagement), zone state, light bonuses through the stat query, zone presentation, placement in `L_SiegeSite_Proto`, Functional Tests, G2 checks.
- P3: verification that zones show and work in Tactical Focus.

### Out of Scope
- Zones affecting enemies (enemy routing, aggro, capture). Enemies ignore zones.
- Zone capture/ownership, control points, scoring.
- Visibility half of "range/visibility" (no vision or fog system in the GDD, NEW-ZON-09).
- High Ground bonus for towers (NEW-ZON-05).
- Resource Camp economy (ECO, VS); Gate as a structure (DEF Blocker list; not in P2 structure scope).
- Zone-only commands (§11.2: no new commands).

## 8. Anti-Goals

- Zones are not RTS control points; nothing is captured or scored.
- Bonuses do not turn zones into mandatory spots (§15.4).
- No procedural zone placement (§27.1).
- No extra micro: one order holds a zone; the AI does the rest (§2.2).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Command Wheel, context resolver insertion point, `FSquadOrder` context fields | SQD `T-SQD-05`, `T-SQD-06` | Zone context and label |
| Squad FSM, formation, leash, targeting | SQD `T-SQD-03`, `T-SQD-07`, `T-SQD-08`, `T-SQD-10` | Hold behavior and bonus read points |
| Stat modifier query (`UStatModifierSubsystem`) | PRK `T-PRK-02` | Bonuses |
| Lanes in `L_SiegeSite_Proto` | DEF `T-DEF-04` | Placement and facing toward lanes |
| World marker component | UXF `T-UXF-04` | Zone markers, bonus icon |
| G2 gate playtest | DEF `T-DEF-20` | Zone checks reported there |
| Tactical Focus overlay + Focus commands | TFM `T-TFM-03`, `T-TFM-04` | R-ZON-18 (P3) |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Overlapping zones at the aim point | Smallest area wins (NEW-ZON-07) |
| All anchors taken | Stand behind the nearest holder, offset by its formation depth along -facing (NEW-ZON-06) |
| Anchor unreachable (structure built on it, nav change) | Use the nearest reachable point within 3 m; else next anchor; else reject with Invalid |
| Same squad re-ordered to the zone it holds | Keeps its anchor; no duplicate bonus |
| Squad wiped while holding | Hold released, bonus removed, anchor freed |
| Two squads hold one zone, one leaves | Leaver's bonus removed only; other unaffected |
| Enemy inside zone but outside zone leash from anchor | Squad engages only up to the zone leash (R-ZON-08 wins) |
| Zone has no anchors (authoring error) | `IsDataValid`/map check error; runtime fallback: zone centre, actor facing |
| Zone placed on no lane (Rally Point) | Facing = authored anchor facing; nothing else needed |
| Hero dead (P3) | Zones work the same from Commander Spirit (same resolver) |

## 11. Acceptance Criteria

### P2
- AC-ZON-01: With the wheel open, aiming at any walkable point inside the Bridge zone highlights it, shows "Hold Bridge" and pre-selects Guard.
- AC-ZON-02: An enemy under the reticle inside a zone gives Attack context, not the zone.
- AC-ZON-03: Holding a zone with Infantry and Archer, Infantry takes a Melee anchor and Archer a Ranged anchor; both face within 10° of the anchor facing after reform.
- AC-ZON-04: The holding squad engages an enemy that enters the zone area even outside its normal engage radius, never moves past the zone leash, and reforms at its anchor when the zone is clear.
- AC-ZON-05: Zone state shows Held when a squad holds it and Contested while an enemy is inside; it updates within one squad decision interval.
- AC-ZON-06: Each bonus is present on the holding squad and removed within one decision interval after the order changes or the squad is wiped; magnitudes come from `DA_Zone_*`.
- AC-ZON-07: High Ground: the holding Archer squad hits a target that is beyond its base range but inside the bonus range. Chokepoint: Infantry takes less damage from the same hit than outside the hold. Rally Point: reform takes less time than without the hold.
- AC-ZON-08: A second squad sent to a zone with one Melee anchor taken stands behind the holder without overlapping formations.
- AC-ZON-09: All six `DA_Zone_*` pass `IsDataValid`; `L_SiegeSite_Proto` contains at least Bridge, High Ground, Chokepoint and Rally Point placed on the two lanes (§33 layout).
- AC-ZON-10: G2 notes (`T-DEF-20`) record zone use, a "zones helped me order fast" rating and a "zones felt mandatory" rating, with a KEEP / CHANGE / DELETE decision for zones.

### P3
- AC-ZON-11: In Tactical Focus all zones are shown, and a Hold order can be issued on a zone from the tactical camera.

## 12. Open Questions / Assumptions

| ID | Question / assumption | Default until answered | Class |
|---|---|---|---|
| NEW-ZON-01 | What is "Infantry block efficiency" (§15.4)? | Incoming damage reduction for Infantry soldiers: damage × (1 − `BlockEfficiency`), matches PRK stat registry | REQUIRED |
| NEW-ZON-02 | Rally Point "reinforce faster": prototype refills only in Recover at the retreat point (NEW-SQD-01) | P2 Rally Point bonus = reform speed only. Whether holding a Rally Point allows refills is open | REQUIRED (decide at G2) |
| NEW-ZON-03 | When does a bonus apply? | Only while a squad holds the zone by order | REQUIRED |
| NEW-ZON-04 | What is "tactical zone state" (§13.2)? | Unheld / Held / Contested, derived from holders and enemies inside | REQUIRED |
| NEW-ZON-05 | Does High Ground help towers (§33 puts a Ballista on high ground)? | No in P2. Small add later: DEF reads a range stat through the same query | IMPROVEMENT |
| NEW-ZON-06 | Anchor capacity | Preferred-role free anchor → any free anchor → behind nearest holder | REQUIRED |
| NEW-ZON-07 | Overlapping zones | Smallest area wins | REQUIRED |
| NEW-ZON-08 | Gate and Resource Camp in P2 | Definitions exist; no special behavior; placed only if the map has a fitting spot | REQUIRED |
| NEW-ZON-09 | "Visibility" part of High Ground | Not built: no vision system in GDD | OUT OF SCOPE |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Define authored battlefield areas, turn an aim point into a zone hold context, give holding squads an anchor, facing, leash and guard area, track zone state, and grant light bonuses through the stat query |
| Inputs | Aim point from the SQD resolver; hold claims/releases from squads; enemy-inside reports from holding squads' candidate scans; zone definitions |
| Outputs | Zone context (zone, label) for the wheel; anchor location + facing + leash for `FSquadOrder`; stat modifiers on holding squads; `OnZoneStateChanged`, `OnHoldersChanged`; presentation events |
| State | Per zone (owner `ATacticalZone`): holder per anchor (weak refs), enemy-inside flag per holder, derived state. Per squad (owner `ASquad`): held zone, claimed anchor, modifier handles |
| Events | `OnZoneStateChanged(Zone, Old, New)`, `OnHoldersChanged(Zone)`; presentation hooks `OnAimHighlightChanged`, `OnZoneStateVisualChanged` |
| Data model | `UTacticalZoneDefinition`: zone tag, display name, hold label, icon, leash radius, bonuses list (stat tag, operation, magnitude, squad type filter). Level instance: area box, anchor points (role tag `Zone.Anchor.Melee/Ranged`, facing). `USquadDefinition` gains preferred anchor role |
| Failure cases (§34.4) | Squad wipe (release hold, drop bonus); tower destroyed / blocked path opened (anchor reachability re-checked on next order or path failure); Core critical (no zone effect); player leaving Siege Site (no zone effect); Hero death (zones work from Commander Spirit); unreachable anchor; zone with no anchors |
| Performance | ≤ ~10 zones per site. Zone lookup only while the wheel is open (point-in-box over a cached list). No zone overlap events and no zone Tick: Contested comes from holding squads' existing candidate scans. Bonus reads happen at use time (attack, hit, reform) |

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Unheld | Subtle ground outline + zone icon/label marker | none | — | Visible from the site overview at run start (§33); must not clutter combat |
| Aimed with wheel | Strong highlight; preview of the anchor the selected squad would take | Wheel tick (SQD) | Wheel label "Hold <Zone>" | Instantly different from a plain ground target |
| Hold accepted | Outline in ally colour | `Feedback.Command.Acknowledged` (SQD) | Squad marker order icon | — |
| Held | Ally colour outline | none | Zone marker shows holder icon | Player sees which squad holds which zone |
| Contested | Warning colour pulse | none (combat audio carries it) | Zone marker warning | Readable in 1–2 s (§28.1) |
| Bonus active | Small bonus icon on squad marker | none | Same icon in squad strip | Subtle (§15.4); PRK spec lists this row for ZON/UXF |
| Hold rejected (no anchor) | Red cross (SQD) | `Feedback.Command.Invalid` (SQD) | Wheel entry flash | Player knows nothing happened |
| In Tactical Focus (P3) | Zone icons on overlay | — | — | Same icons as third-person |
