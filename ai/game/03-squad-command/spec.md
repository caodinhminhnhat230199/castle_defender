# Squad Command: Specification

| | |
|---|---|
| Feature | SQD (`03-squad-command`) |
| GDD sections covered | §2.2, §11 (all), §13 (all), §29.2, §28.1, §28.3 (order acknowledged, squad low strength), §34.4 (squad wipe), §36 Command Core DoD, §32 Prototype 1 |
| Cross-references | §1.1, §3, §10.2–10.3, §12.2, §15.3, §18.2, §31.2, §34.1–34.2 |
| Phases | **P1** core (gate G1) · **P2** Core retreat, structure-defense rule, zone hook used by ZON · **VS** Spearman + squad abilities (provisional) |
| Status | Draft v1 |

## 1. Overview

The player commands up to 3 squads with 4 orders through a Command Wheel used in third-person combat. The player gives intent ("hold here", "hit this", "follow me", "fall back"); squad AI handles formation, target choice, leash and recovery. This is Pillar B, Command at a Glance (§2.2), and the Army layer of "You are not controlling an army. You are fighting inside your army." (§1.1).

## 2. Player Experience

- Ordering a squad is a 1–3 second interruption, then the player is back in the fight (§2.2).
- Squads feel competent: they hold a spot, pick sensible targets, come back when pulled away, and never need the player to fix one soldier (§13.1).
- The player can always tell where each squad is and what it is doing, by marker and by sound (§28.1, §28.3).
- Hero + squads must play clearly better than the Hero alone (G1, §32 P1). The Hero alone should struggle with the G1 encounter (§1.1: the player is not strong enough to win alone).

## 3. Core Loop

```text
Fight → notice a need (flank, threat, position) → hold wheel + aim → (pick squad) → release
→ back to fighting → read marker / hear acknowledgement → adjust later
```

## 4. Gameplay Rules

### P1 scope

| ID | Source | Label | Rule |
|---|---|---|---|
| R-SQD-01 | §11.1 | [LOCKED for prototype] | Max **3 active squads**. The cap is a setting (default 3). Full game 3 vs 4 is Q-03. |
| R-SQD-02 | §32 P1 | [LOCKED] | P1 content: 1 Infantry squad + 1 Archer squad. |
| R-SQD-03 | §2.2, §3 | [LOCKED] | Orders target a whole squad. No soldier selection, no per-soldier orders. |
| R-SQD-04 | §11.2 | [LOCKED] | Exactly four commands: **Guard / Hold Position**, **Attack / Focus Target**, **Follow Hero**, **Retreat / Return to Core**. No new command unless it creates a clearly new decision. |
| R-SQD-05 | §11.4 | [LOCKED] | Command Wheel is the main command input in third-person. Flow: hold command key → aim target or ground → choose a squad or keep the current one → choose a command → release → back to combat. |
| R-SQD-06 | §11.4 | [LOCKED] | Target interaction is context-sensitive: what is under the aim pre-selects a command and fills its target. Default mapping is Assumption NEW-SQD-09 (enemy → Attack, walkable ground → Guard, the Hero or nothing valid → Follow, Retreat never pre-selected). |
| R-SQD-07 | §29.2, §36 | [LOCKED] | The wheel works while the Hero moves. It never pauses or slows time. Squad selection keys never collide with combat keys. |
| R-SQD-08 | §2.2 | [LOCKED] | A useful order takes 1–3 s from opening the wheel to release. Measured in G1 telemetry. |
| R-SQD-09 | §29.1, §29.2 | [LOCKED] | All command inputs are device-agnostic input actions, mappable to gamepad later. No RTS hotkey grid. |
| R-SQD-10 | §18.2, §34.2 | — | Command authority belongs to the player, not the Hero body, so squads stay commandable while the Hero is dead (used by CSM in P3). |
| R-SQD-11 | §13.2 | Baseline | Squad FSM states: Idle, Follow, MoveToOrder, Guard, Engage, Reform, Retreat, Recover (+ Wiped, NEW-SQD-02). Transitions depend on player order, enemy in engagement range, squad strength, path availability, leash distance and (P2) tactical zone state. See §4.1. No morale system (out of scope). |
| R-SQD-12 | §13.3 | [LOCKED] | Formation is driven by a squad anchor. Only the anchor pathfinds the route; soldiers use local avoidance and slot assignment. |
| R-SQD-13 | §13.3 | [LOCKED] | Formation narrows (fewer columns, longer column) through choke points and restores its width after. |
| R-SQD-14 | §13.3 | [LOCKED] | After engagement the squad reforms when no enemy is within engagement range. |
| R-SQD-15 | §13.1 | [LOCKED] | The player never fixes micro errors: small slot errors, minor archer target choice, formation breaking due to simple pathing. |
| R-SQD-16 | §13.4 | — | **Infantry** target priority: (1) enemy touching/threatening the Guard Zone, (2) nearest melee enemy, (3) enemy attacking Core/Tower near the guard position (**P2**, R-SQD-29), (4) player Focus Target. Focus Target overrides the list while it is inside the Attack leash. |
| R-SQD-17 | §13.4 | — | **Archer** target priority: (1) player Marked / Focus Target, (2) Flying ([DEFERRED] §20.2, no entry), (3) low-HP dangerous unit (NEW-SQD-07), (4) enemy nearest the guarded spot. |
| R-SQD-18 | §13.5 | [LOCKED] | Leash: squads never chase without limit. Guard creates a leash radius. When targets leave the leash: disengage, return to anchor, reform. Leash centre per order in §4.2. |
| R-SQD-19 | §13.6 | [LOCKED] | Stuck recovery ladder per soldier: detect stuck → request repath → slot reassignment → short-range teleport only as a last resort, hidden, when the player is not looking at that soldier. One stuck soldier never freezes the squad. |
| R-SQD-20 | §10.2, §10.3 | [LOCKED] | Target rules can read shared combat states so squads exploit openings (Archer prefers Marked; squads prefer Staggered "if the rule allows"). The state-based entries are owned by SYN (`T-SYN-05`). |
| R-SQD-21 | A-06 | Assumption | Attack / Focus Target does **not** apply Marked in the prototype (Q-14). |
| R-SQD-22 | §28.1 | [LOCKED] | Squad location and current command state are readable within 1–2 s. |
| R-SQD-23 | §28.3 | — | Distinct audio events for "tactical order acknowledged" and "squad low strength". |
| R-SQD-24 | §34.4 | — | Squad wipe is a handled state, never a soft-lock (behavior NEW-SQD-02). |
| R-SQD-25 | §13.2 | Assumption NEW-SQD-01/03 | Retreat ends in Recover at the retreat point: the squad reforms and refills lost soldiers. P1 retreat point = the squad's home position (no Core yet). |
| R-SQD-26 | §34.1, §38 | [LOCKED] | Squad content is data (`USquadDefinition`). Every [TUNABLE] value lives in data, never in code. |
| R-SQD-27 | §31.2 | [LOCKED] | Squad/group logic first: decisions run per squad on a timer, not as full per-soldier AI every frame. |
| R-SQD-28 | §12.2 | [LOCKED] | Squad position, order and strength are exposed through the same squad delegates/markers so the Tactical Focus overlay (TFM, P3) can show squad icons without new squad code. |

### P2 scope

| ID | Source | Label | Rule |
|---|---|---|---|
| R-SQD-29 | §13.4 | — | Infantry priority (3) active: enemy attacking the Core or a structure near the guard position. |
| R-SQD-30 | §11.2 | [LOCKED] | Retreat target = the Core when a Core exists in the level. |
| R-SQD-31 | §13.2, §15.3 | — | An order can carry a Tactical Zone context (anchor, facing, leash, guard area). SQD provides the hook; ZON owns the behavior. |

### VS scope (provisional, re-plan after G3)

| ID | Source | Label | Rule |
|---|---|---|---|
| R-SQD-32 | §11.3 | [TUNABLE, after basic prototype] | Each squad has at most one active tactical ability: Infantry **Shield Wall**, Archer **Volley**, Spearman **Brace**. No MOBA-style skill kits. Effects NEW-SQD-12. |
| R-SQD-33 | §13.4 | — | **Spearman** priority: (1) Giant/Siege, (2) Armored, (3) Focus Target, (4) nearest valid melee. Benefits from Armor Broken (§10.2, via SYN). |
| R-SQD-34 | §32 VS | — | At most 3 squad types at VS. |

### 4.1 Squad state machine (design level)

| From | Condition | To |
|---|---|---|
| any alive | Order Guard or Attack accepted | MoveToOrder |
| any alive | Order Follow | Follow |
| any alive | Order Retreat | Retreat (disengages at once) |
| any | Strength reaches 0 | Wiped |
| Idle / Follow / Guard / Recover | Enemy in engagement range (inside guard area) | Engage |
| MoveToOrder | Squad is attacked (NEW-SQD-06) | Engage, then resume MoveToOrder |
| MoveToOrder (Guard) | Anchor arrived | Reform → Guard |
| MoveToOrder (Attack) | Focus Target inside engagement range | Engage |
| Engage | No valid target inside leash | Reform |
| Reform | Enemy in engagement range | Engage |
| Reform | Soldiers in slots, or reform timeout | Resting state of the order (Idle / Follow / Guard / MoveToOrder / Recover) |
| Retreat | Arrived at retreat point | Recover |
| Recover | Full strength and reformed | Idle |
| Attack order | Focus Target dead, invalid or outside Attack leash | Order becomes Guard at current position (NEW-SQD-04) |
| Follow order | Hero dead | Order becomes Guard at current position (NEW-SQD-05) |
| Wiped | Respawn delay elapsed | Idle at retreat point, full strength |
| any | Order destination unreachable | Order rejected, state unchanged, Invalid feedback |

Retreat never enters Engage. Soldiers still take damage while retreating.

### 4.2 Leash and engagement per order

| Order / state | Leash centre | Guard area (Infantry rule 1) | Data |
|---|---|---|---|
| Idle, Guard | Guard anchor | Engage radius around anchor (zone volume in P2, ZON) | `EngageRadius`, `GuardLeashRadius` |
| Follow | Hero position | Engage radius around squad anchor | `FollowLeashRadius` |
| Attack | Focus Target position at order time | none | `AttackLeashRadius` |
| Recover | Retreat point | Engage radius around retreat point | `EngageRadius` |

Engage radius ≤ leash radius, so squads do not flip between Engage and Reform at the border. Archers attack from their slots (NEW-SQD-08); their engage radius is their weapon range.

## 5. Player Actions

| Action | Meaning |
|---|---|
| Hold command key | Open the Command Wheel; time keeps running; Hero can still move and dodge |
| Aim (camera) | Choose target/ground; wheel shows the context and pre-selected command |
| Select squad (1–3) | Change which squad receives the order; the choice becomes the current squad |
| Change command | Cycle to another of the 4 commands |
| Release / confirm | Issue the order; wheel closes |
| Cancel | Close the wheel without an order |

Default bindings are a proposal (NEW-SQD-09) validated in G1.

## 6. Success / Failure Conditions

- Order success: squad visibly starts executing within one decision interval and the acknowledgement plays.
- Order failure: invalid target (unreachable, out of range, wiped squad, Attack with no enemy) → Invalid feedback, no state change.
- Squad wipe: squad unavailable until it respawns (NEW-SQD-02). Not a run failure (run failure is Core loss, §18.1).
- Feature failure (G1): if Hero + squads is not better than Hero alone, or commands cannot be used mid-combat, P1 iterates; P2 does not start (§32).

## 7. Scope

### In Scope
- P1: `ASquad` anchor + slots, soldiers with local avoidance, Infantry + Archer content, squad FSM, 4 commands, Command Wheel + context target, target priority, leash, reform, stuck recovery, squad strength / low strength / wipe / respawn, command feedback hooks, `L_CombinedArms` map, G1 playtest.
- P2: Retreat to Core, Infantry structure-defense rule, zone context hook.
- VS (provisional): Spearman, one ability per squad type.

### Out of Scope
- Per-soldier control, soldier selection boxes, RTS hotkey grids (§3).
- Morale (§13.2 says "if any"; none defined).
- "All squads" order, formation type selection, extra commands (§11.2).
- Flying target rule (§20.2 DEFERRED).
- Recruiting, Food upkeep, squad upgrades (§16, ECO / CNV at VS).
- Tactical Focus commands (TFM `T-TFM-04`), Commander Spirit command view (CSM `T-CSM-02`), zone hold behavior (ZON).

## 8. Anti-Goals

- Not an RTS: no micro of individual soldiers, no build queues, no unit selection drag box (§3).
- Squads are not MOBA heroes: at most one ability each, VS only (§11.3).
- The wheel is not a pause menu (§36).
- Squads are not an auto-win: they hold and support; the Hero still wins the small fights (§1.3).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Combat contract (`FCombatHit`, `UHealthComponent`, `UCombatStateComponent`, team) | FND `T-FND-05` | Soldiers deal and take hits |
| Controller, Enhanced Input contexts, tuning settings, tags, debug/test harness | FND `T-FND-04/06/07/09/10` | Command component, `IMC_CommandWheel`, data |
| Hero pawn, lock-on, death event | CMB `T-CMB-01/10/11` | Follow, context target, Follow fallback |
| Enemies (base, Swarm, Armored) that also target soldiers | ENM `T-ENM-01/05/06` | Targets and threat for squads |
| Shared states + Army consumers | SYN `T-SYN-02/03/05` | Armor Broken / Marked / Staggered rules |
| Feedback subsystem, HUD shell, world markers, audio set, telemetry | UXF `T-UXF-01/02/04/06/08` | Readability and audio rules |
| Core (P2), enemy-attacking-structure info (P2) | DEF `T-DEF-03`, ENM `T-ENM-09` | R-SQD-29/30 |

Used by: SYN (`T-SYN-05`), ZON (all), TFM (`T-TFM-04`), CSM (`T-CSM-02`), PRK (Army perks, P3).

## 10. Edge Cases

| Case | Behavior |
|---|---|
| 4th squad spawned | Refused, error logged; wheel shows max 3 |
| Order to unreachable point / beyond max command distance | Project to nearest navigable point within a small radius; if none, Invalid feedback |
| Attack with no enemy under aim | Aim assist picks the nearest enemy within a small radius of the aim point; if none, Attack shows unavailable (NEW-SQD-04) |
| Hero locked on | Lock-on target is the context enemy (Attack pre-selected) |
| Wheel held while Hero is hit / in a committed attack | Wheel stays open; attack finishes (commitment); attack/block keys are consumed by the wheel while held |
| Two squads ordered to the same spot | Accepted; local avoidance spreads soldiers. Revisit if G1 shows confusion |
| Soldier pushed off navmesh (knockback) | Stuck ladder (R-SQD-19) |
| Path changes mid-move (P2 structure placed) | Anchor re-paths; if no path remains, order becomes Guard at current position + Invalid feedback |
| Hero walks through own formation | Hero is never body-blocked by allies (NEW-SQD-10) |
| Squad wiped while holding a zone / under Attack order | Order cleared; zone hold released (ZON) |
| Hero dies (P3) | Follow → Guard at current position; other orders unchanged; commands continue from Commander Spirit (CSM) |
| Tactical Focus time dilation (P3) | Squad timers run on game time and slow with the world (D-13) |
| Player leaves Siege Site (P3) | Squads keep their orders; Follow leash keeps them inside their leash, not outside the site boundary (RUN handles the Hero) |

## 11. Acceptance Criteria

### P1
- AC-SQD-01: In `L_CombinedArms`, the player issues each of the 4 commands to each squad while moving and while enemies are attacking the Hero, with no pause or slow-motion, and each order executes (§36).
- AC-SQD-02: G1 telemetry: median wheel-open-to-release time ≤ 3 s across testers (R-SQD-08).
- AC-SQD-03: Aim at enemy → Attack pre-selected; aim at ground → Guard pre-selected; aim at sky/Hero → Follow pre-selected; release issues it to the current squad.
- AC-SQD-04: Guard: squad walks to the point, forms facing the order direction, engages enemies entering the guard area, never moves past the leash, and returns and reforms within the reform timeout after enemies leave.
- AC-SQD-05: Follow: squad trails the Hero at its follow offset, fights enemies near it, and never strays beyond the follow leash from the Hero.
- AC-SQD-06: Attack: squad goes to the Focus Target and attacks it; when the target dies, the squad holds where it is (Guard).
- AC-SQD-07: Retreat: squad stops attacking at once, reaches the retreat point, refills to full strength in Recover, then goes Idle.
- AC-SQD-08: A full Infantry squad passes a gap one soldier spacing wide and restores full formation width within the reform timeout after the gap.
- AC-SQD-09: A soldier trapped by a test cage never stops the squad from completing its order; it is teleported only while not visible; no teleport is ever visible on screen.
- AC-SQD-10: Infantry picks an enemy inside its guard area over a nearer enemy outside it; Archer picks the Focus Target over others; Archer picks the lowest-HP dangerous unit over the nearest Swarm.
- AC-SQD-11: A 4th squad cannot be registered.
- AC-SQD-12: Every accepted order plays the acknowledgement event; low strength cue fires once per threshold crossing.
- AC-SQD-13: Wiped squad: wheel shows it unavailable with a timer; orders to it give Invalid feedback; it respawns at the retreat point after the delay.
- AC-SQD-14: In G1, testers name each squad's location and current order within 2 s when asked (§28.1).
- AC-SQD-15: Changing a value in `DA_Squad_Infantry` (e.g. leash) changes behavior in PIE without code changes.
- AC-SQD-16: G1 checklist (master plan §3) is filled in, with a Hero-alone vs Hero + squads comparison and a KEEP / CHANGE / DELETE decision.

### P2
- AC-SQD-17: With a Core in the level, Retreat goes to the Core; Infantry guarding near a tower attacks the enemy hitting that tower before other non-guard-area enemies.

### VS (provisional)
- AC-SQD-18: Spearman follows R-SQD-33 priority; each squad type has one ability that can be triggered from the wheel and respects its cooldown.

## 12. Open Questions / Assumptions

| ID | Question / assumption | Default until answered | Class |
|---|---|---|---|
| NEW-SQD-01 | What does Recover do (§13.2 lists it, no detail)? | At the retreat point the squad reforms and refills one soldier per `ReplenishInterval` while no enemy is in engage range. Free in P1/P2. Whether refills cost the P3 run resource → RUN (A-05). | REQUIRED |
| NEW-SQD-02 | Squad wipe behavior (§34.4 requires coverage, no design given) | State Wiped; squad unavailable; respawns at the retreat point at full strength after `WipeRespawnDelay` | REQUIRED |
| NEW-SQD-03 | Retreat destination before a Core exists (P1) | Squad home = spawn transform | REQUIRED |
| NEW-SQD-04 | Attack target resolution and end | Requires an enemy (small aim assist); target dead or out of Attack leash → Guard at current position | REQUIRED |
| NEW-SQD-05 | Follow when the Hero is dead | Guard at current position | REQUIRED (P3) |
| NEW-SQD-06 | Do moving squads fight? | Guard/Attack MoveToOrder engages only when attacked; Retreat never engages | REQUIRED |
| NEW-SQD-07 | "Low-HP dangerous unit" (§13.4) | Among enemies with archetype tags Armored / Siege / Boss, lowest HP ratio. Tag set in data | REQUIRED |
| NEW-SQD-08 | Do archers leave formation? | No, they shoot from their slots | REQUIRED |
| NEW-SQD-09 | Wheel bindings and context mapping | Hold key opens; 1–3 squad; mouse wheel cycles command; release or LMB confirms; RMB cancels; mapping per R-SQD-06 | REQUIRED, validate in G1 |
| NEW-SQD-10 | Can allies body-block the Hero? | No; Hero passes through own soldiers | REQUIRED (feel) |
| NEW-SQD-11 | Low strength threshold and effect | 35 % alive (placeholder, data); feedback only, no auto-retreat (player intent rules, §2.2) | REQUIRED |
| NEW-SQD-12 | Ability trigger and exact effects of Shield Wall / Volley / Brace | Trigger = 5th wheel entry for the selected squad; effects proposed in tasks, confirmed at VS planning | FUTURE (VS) |
| NEW-SQD-13 | "All squads" order | Not built. Add only if G1 shows ordering 2–3 squads is too slow | FUTURE |
| Q-03 | Max squads in full game | 3 | — |
| Q-14 / A-06 | Attack applies Marked? | No | — |

Placeholder numbers (all [TUNABLE], in `DA_Squad_*` or `UGameTuningSettings`, none from the GDD): Infantry 8 soldiers (4×2), Archer 6 (3×2), engage 8 m (Archer 25 m weapon range), guard leash 15 m, follow leash 20 m, attack leash 20 m, reform timeout 4 s, replenish 4 s/soldier, wipe respawn 20 s, decision interval 0.2 s. Soldier count range follows the foundation expected scale (8–12 per squad).

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Turn player intent (4 commands) into squad behavior: movement, formation, targeting, leash, recovery; report squad state for UI/audio |
| Inputs | Orders from `UCommandComponent` (wheel, debug cheats; later TFM/CSM); Hero location and death event; hostile pawns near the squad; enemy archetype tags, HP and combat states; P2: Core location, zone context, stat modifiers (ZON) |
| Outputs | `FCombatHit` from soldiers (source layer Army); squad delegates; feedback tags; telemetry events |
| State | Per squad (owner `ASquad`, D-07): definition, current order, FSM state, soldier list with slot/target/stuck data, strength, respawn timer. Per player (`UCommandComponent`): registered squads, current squad, wheel state |
| Events | `OnSquadStateChanged`, `OnOrderChanged`, `OnStrengthChanged`, `OnSquadWiped`, `OnSquadRespawned`; `OnWheelOpened/Closed`, `OnWheelContextChanged`, `OnOrderIssued` |
| Data model | `USquadDefinition`: squad type tag, display name, icon, soldier class, soldier count, formation (columns, spacing, min columns), move speed, engage/leash radii (guard, follow, attack), follow offset, ordered target rules, max attackers per target, soldier stats (HP, armor, damage, poise damage, attack interval/range, projectile), reform timeout/ratio, low strength threshold, replenish interval, wipe respawn delay; VS: one ability |
| Failure cases (§34.4) | Squad wipe (NEW-SQD-02); Hero death (Follow fallback, commands continue); tower destroyed / blocked path opened (anchor re-paths on route change); Core critical (squads keep orders; Retreat still targets Core); player leaving Siege Site (squads keep orders); boss reset (none for squads); unreachable order; stuck soldier; 4th squad |
| Performance | ≤3 squads × ≤12 soldiers. One target query per squad per decision tick (cached candidates shared by soldiers). No per-soldier Tick logic. Wheel trace runs only while the wheel is open. Measured in `T-SQD-15` |

## 14. Feedback Contract (GDD §34.5)

Tags are proposals; rows live in `DT_Feedback` (UXF).

| State / event | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Wheel open | Reticle; target preview (ground ring / enemy outline); selected squad guard ring | Soft open tick (UI sound) | Wheel: 4 commands, squads 1–3 with state + strength, pre-selected command, context label | Readable in < 1 s; must not hide the Hero; game keeps running |
| Order acknowledged `Feedback.Command.Acknowledged` | Ping at destination / outline flash on target; squad marker pulse | Per-squad-type acknowledgement bark or horn (§28.3) | Marker command icon changes | Audible without looking at UI |
| Order invalid `Feedback.Command.Invalid` | Red cross at aim point | Short error click | Wheel entry flashes | Player knows nothing happened |
| Squad state (Guard / Follow / MoveToOrder / Engage / Retreat / Recover) | World marker above squad with order icon (`T-UXF-04`) | None (avoid spam) | Same icon in wheel | Location + order readable in 1–2 s from 40 m (§28.1) |
| Low strength `Feedback.Squad.LowStrength` | Marker warning colour | Distinct cue (§28.3) | Strength pips warning colour | Fires once per crossing |
| Wiped `Feedback.Squad.Wiped` | Marker greyed at last position for a few seconds | Cue | Wheel slot greyed + respawn timer | Player never wonders where a squad went |
| Reinforced / respawned `Feedback.Squad.Reinforced` | Spawn VFX at retreat point | Small cue | Pips refill | — |
| Ally vs enemy | Team tint / banner on soldiers (§28.2) | — | — | Ally silhouette distinct from enemies at a glance |
| Hidden stuck teleport | None by design | None | None | Must never happen on screen |
