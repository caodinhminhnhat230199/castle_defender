# Structures & Pathing (DEF): Specification

| | |
|---|---|
| Feature code | DEF |
| GDD sections covered | §2.3, §14 (all, source of truth), §21, §31, §32 Prototype 2, §33 (17:00 Structure Collapse), §34.4 (tower destroyed, blocked path opened, Core critical HP), §36 Defense Core DoD. Cross-refs: §5.3, §6.3, §9.7, §10.2–10.3, §12.2, §16.1, §16.3, §18.2, §20.1, §28, §34.5, §37 |
| Phases | P2 (main), P3 (run resource cost, Defense perk hooks), VS (provisional: 4th tower slot, repair) |
| Master plan refs | D-05, D-09, D-10, D-15, D-16; A-04, A-05; Q-04, Q-05 |
| Status | Draft v1 |

## 1. Overview

DEF owns everything the player builds and the strategic lane layer that decides how enemies react to it:

- Structures: Core, Ballista, Bombard, Barricade, with the §14.6 classification (Blocker / Combat Tower / Utility).
- Placement: authored build zones with a soft grid (A-04), a per-wave build limit in P2 (A-05).
- Lane layer: authored lane routes, route assignment, path-blocking detection, minimum-break target on full seal, local repath after destruction, dirty-region updates (D-09).
- Tower targeting and projectiles.
- The P2 performance benchmark that sets the first concurrent enemy cap (Q-04).

Enemy behavior on top of the lane layer (following the route, local aggro, attacking the Path Obstacle Target) is implemented by ENM (`T-ENM-07..10`) through the contract in Section 13.

## 2. Player Experience

- "I shape where they go." A Barricade across a choke makes the Swarm stop and hammer it right in front of the Bombard (§2.3, §33 02:00).
- Enemies feel like lane minions: they march, meet an obstacle, stop, break it, march on (§14.1). They are not clever path optimizers.
- Sealing a lane is allowed and has a cost: enemies pick the cheapest blocker to break (§14.4).
- Late in a run, defenses erode: a Siege enemy breaks the tower holding the corridor and the lane floods (§33 17:00). The player sees it coming (HP marker, audio) and has to react.
- Tower roles read instantly: Ballista = big single bolts on armored targets; Bombard = slow shells that wreck clumps; Barricade = time (§21, §28.2).

## 3. Core Loop

```text
Read lanes + forecast → Place / reposition structures in build zones → Watch route preview
→ Enemies arrive: pass gaps or stop and break blockers → Towers fire by role
→ Structure breaks → route reopens → Hero/squads react → Rebuild next intermission
```

## 4. Gameplay Rules

### P2 scope: lane layer and blocking

- **R-DEF-01 (§2.3, §36) [LOCKED]**: Structure placement changes enemy route or encounter. Placing a structure on a lane must be able to change where enemies stop and fight.
- **R-DEF-02 (§14.1) [LOCKED]**: Enemies advance along a clear lane route, stop and fight when obstructed, break the obstacle, then continue. They never route around every defense.
- **R-DEF-03 (§14.2) [LOCKED]**: Every enemy has an Assigned Lane, a Current Route, a Local Aggro Radius and an Objective (Core, or a special objective). DEF provides lane, route and objective; the aggro radius is ENM archetype data.
- **R-DEF-04 (§14.2) [LOCKED]**: An enemy (re)selects its route only on: spawn, passing a major checkpoint, current route becoming invalid, a blocking structure being destroyed, a special ability forcing a repath. No full repath every frame.
- **R-DEF-05 (§14.3) [LOCKED]**: If a structure lies on the current valid route and its footprint blocks the corridor the enemy needs to pass, the enemy moves to it, stops, acquires it as **Path Obstacle Target** and attacks it. When it is destroyed or removed: local navigation updates, the enemy gets its next route and continues to the Core.
- **R-DEF-06 (§14.3) [LOCKED]**: An enemy does not detour around a blocking structure because another route exists, unless (a) the current route is marked invalid by a design rule, (b) its archetype has a special behavior, or (c) the Director/an event changes its lane on purpose.
- **R-DEF-07 (§14.3, derived)**: "Blocks the corridor" is evaluated inside the current route's corridor and for the enemy's body size. A gap at least as wide as the enemy needs is open (the enemy walks through it, no attack); a narrower gap counts as blocked for that enemy. Size classes are an implementation choice, see NEW-DEF-01.
- **R-DEF-08 (§14.4) [LOCKED]**: Sealing a lane geometrically is allowed. If no open route to the Core remains, the lane layer finds the **minimum-break path**: the blocking structure(s) with the lowest break cost / most effective opening; enemies focus that structure and continue once it opens. There is no hard rule against full seals.
- **R-DEF-09 (§14.4) [TUNABLE]**: Break cost of a structure = its max HP / a reference break DPS, scaled by a break-cost multiplier; travel cost = distance / reference move speed. Defaults live in `UGameTuningSettings` (Lane group). Default multiplier is "strict": enemies only break when no open path exists in the corridor (literal §14.3 reading). Lowering it lets very long in-corridor detours (mazes) be treated as worse than breaking; see NEW-DEF-02.
- **R-DEF-10 (§36) [LOCKED]**: No enemy stands still bugged. If a route query returns no result, or local movement fails repeatedly, the enemy falls back to attacking the nearest player structure on or near its corridor, or walking back to its corridor.
- **R-DEF-11 (§14.7) [LOCKED design]**: Navigation updates are local: only the dirty region/cells of a built or destroyed structure update. No full-world nav rebuild per build. Route decisions are computed per route (group level), not per enemy global path. Enemy count is budgeted by the benchmark (R-DEF-30).
- **R-DEF-12 (§14.5) [LOCKED]**: Structures are valid Local Aggro targets under the archetype's interaction rule (ENM data, e.g. Giant/Siege prefers structures, §20.1). Enemies do not chase past leash and do not abandon their lane indefinitely (ENM).

### P2 scope: structures and classification

- **R-DEF-13 (§14.6) [LOCKED]**: Every structure has exactly one role: **Blocker** (Barricade; Wall/Gate later), **Combat Tower** (Ballista, Bombard), **Utility** (Forge, Alchemy, support buildings; VS). The Core has its own role `Structure.Role.Core`.
- **R-DEF-14 (§14.6) [LOCKED]**: Combat Towers have a physical footprint. A Combat Tower placed on a corridor that truly blocks the route is a Path Obstacle exactly like a Blocker.
- **R-DEF-15 (§14.6) [LOCKED]**: Utility buildings should not sit in a default lane; if enemies reach them they can still be destroyed. Enforced through build-zone role permissions (VS content, CNV).
- **R-DEF-16 (§21.1) Ballista [LOCKED role / TUNABLE numbers]**: Anti-Armor / Anti-Elite. Slow fire rate, high single-target damage, good vs Armored and Giant, can exploit Armor Broken. Example values in `DA_Structure_Ballista`.
- **R-DEF-17 (§21.1) Bombard [LOCKED role / TUNABLE numbers]**: Anti-Swarm / AoE. Slower projectile, splash damage, weak vs fast or lone targets, strong at choke points. Values in `DA_Structure_Bombard`.
- **R-DEF-18 (§21.1) Barricade [LOCKED role / TUNABLE numbers]**: Delay / Path Blocking. Deals almost no damage. Its job is to make enemies stop and attack, buying time. Values in `DA_Structure_Barricade`.
- **R-DEF-19 (§10.3, §9.7, §10.2) [LOCKED]**: Towers take part in Battlefield Synergy: heavy tower impact (Bombard) deals poise damage that can cause Staggered; Ballista gains priority/effect vs Armor Broken (and vs Marked once a source exists, A-06). Consumers are implemented by SYN `T-SYN-06`; DEF exposes the data hooks.
- **R-DEF-20 (§21.3) [LOCKED]**: A tower is added only if it defines role, preferred target, weakness, synergy and placement question. All three prototype towers carry these five items in their definition description.
- **R-DEF-21 (§32 P2, §18.1) [LOCKED]**: The Core is the protected objective at the end of every lane. Core destroyed → run lost (RUN `T-RUN-02`). Core HP is [TUNABLE] (`DA_Structure_Core`). The Core is placed in the level, never built.

### P2 scope: placement

- **R-DEF-22 (§5.3, Assumption A-04, [OPEN] Q-05)**: Structures are placed only inside authored build zones, snapped to a soft grid, with 90° rotation steps. Revisited at G2.
- **R-DEF-23 (Assumption A-05)**: P2 building is free, limited by a per-wave build allowance (default 3, `UGameTuningSettings`) and a total structure cap per Siege Site (default 15, from the foundation expected scale). Allowance refills when a wave ends; unused allowance does not carry over (NEW-DEF-03).
- **R-DEF-24 (§14.3, derived)**: A placement is invalid if any footprint cell is outside a zone allowing the structure's role, not walkable, already occupied, or overlapping a pawn (Hero, soldier, enemy). Placing on top of enemies is never allowed.
- **R-DEF-25 (§12.2, §34.3, §28.1)**: While previewing a placement the player sees whether it would block a lane (enemies will stop and attack it) and the resulting route. This is feedback for R-DEF-05/08, not a new rule.

### P2 scope: readability, failure, performance

- **R-DEF-26 (§28.1, §28.3) [LOCKED]**: Within 1–2 s the player can read which structure is about to break and the Core HP. Tower critical HP and Core under attack have their own audio events.
- **R-DEF-27 (§34.4) [LOCKED]**: "Tower destroyed" and "blocked path opened" have defined recovery: grid and navmesh update, affected enemies re-query and resume, feedback plays on the lane, no enemy stays stuck.
- **R-DEF-28 (§33 17:00) [LOCKED]**: Structure Collapse flow works end to end: Siege enemy breaks a tower blocking the corridor → local path updates → enemies flow through.
- **R-DEF-29 (§31.2) [LOCKED]**: Tower projectiles get pooling/optimization only if the benchmark shows a need (D-15).
- **R-DEF-30 (§31.2, §37, Q-04) [OPEN/TUNABLE]**: The P2 benchmark sets the first concurrent enemy cap on the reference PC. Crowd readability counts more than raw number: cap = min(performance cap, readability cap). Stored in `UGameTuningSettings::MaxConcurrentEnemies`, consumed by DIR.

### P3 scope

- **R-DEF-31 (A-05, §16.3)**: Building costs the single run resource (`UStructureDefinition::BuildCost`); not enough resource → placement denied with feedback. The per-wave allowance is switched off by data when cost is on.
- **R-DEF-32 (§23.1–23.2) [LOCKED]**: Defense perks (e.g. "Ballista shot N pierces") can change tower stats and shot behavior through DEF hooks without editing tower code. Effects are owned by PRK.
- **R-DEF-33 (§12.2)**: Tactical Focus overlay can show tower HP and the predicted route per lane; DEF provides the route preview query (TFM `T-TFM-03` consumes it).
- **R-DEF-34 (§18.2)**: Placement and structure queries work from the player controller, not only from the Hero pawn, so Commander Spirit Mode can reuse them (CSM `T-CSM-05` decides which interactions are allowed).

### VS scope (provisional, re-plan after G3)

- **R-DEF-35 (§32 VS, §21.2, §21.3)**: Vertical Slice allows 3–4 towers. A 4th tower must pass R-DEF-20; the §21.2 roles (Frost/Slow, Ward, Arc, Anti-Air) are [DEFERRED] and need an explicit re-request.
- **R-DEF-36 (§6.3, §16.1)**: Reconfigure includes repairing defense; repair is paid with Gold in the full game (run resource before ECO exists). Phase placement: NEW-DEF-04.

## 5. Player Actions

| Action | Phase | Notes |
|---|---|---|
| Enter / exit build mode | P2 | Switches to `IMC_Build`; Hero can still move |
| Pick a structure (Ballista, Bombard, Barricade) | P2 | Build bar shows remaining allowance (P2) or cost (P3) |
| Aim, rotate (90°), confirm, cancel placement | P2 | Ghost snaps to zone grid; red with reason when invalid |
| Read route preview / "blocks lane" warning | P2 | Before confirming |
| Defend structures with Hero and squads | P2 | No DEF input; structures are targets for enemies |
| Repair a damaged structure | VS (provisional) | Interact verb (`T-CMB-12`) |

## 6. Success / Failure

- Success (feature level): G2 Defense checks pass (Section 11, `T-DEF-20`).
- Run failure owned by RUN: Core HP reaches 0.
- Partial success: a structure that falls still bought time; its contribution is visible (enemies stopped at it).
- Retry: next intermission the player rebuilds within allowance/resource.

## 7. Scope

### In Scope
- Core, Ballista, Bombard, Barricade; role classification; footprints; health; nav modifiers.
- Lane routes, corridor grid, route selection, blocking detection, minimum-break search, dirty-region updates, route invalidation.
- Build zones, soft-grid placement, preview, validation, route preview, P2 build allowance, P3 build cost.
- Tower targeting, projectiles, splash.
- Structure/Core feedback events.
- `L_SiegeSite_Proto` blockout: 2 lanes, Core, build zones.
- Spike `T-DEF-01`, benchmark `T-DEF-12`, Functional/exploit tests, G2 gate playtest.

### Out of Scope
- Wall, Gate (Blocker examples, not in P2 scope), Utility building content (CNV, VS).
- Later tower roles §21.2 [DEFERRED], Flying pathing (§20.2 [DEFERRED]).
- Tower upgrades (listed in §6.3/§16.1 but not in any prototype scope: FUTURE).
- Player dismantle/sell (not in GDD; NEW-DEF-05).
- Enemy movement/attack behavior itself (ENM), squad hold behavior (SQD/ZON), wave content (DIR), run resource ownership (RUN).
- Destructible terrain, free-form construction (§5.4).

## 8. Anti-Goals

- Not a maze-builder TD where the best play is to fill the field with walls: sealing/mazing is answered by enemies breaking through (R-DEF-08/09).
- Enemies are not omniscient path optimizers that dodge every tower (§14.1).
- Not a city builder or free-form construction game (§3, §5.4).
- No tower tree with dozens of variants (§3): one definition per tower in prototype.
- No full-world navmesh rebuild when building (§14.7).
- No per-enemy global pathfinding every frame (§14.2).

## 9. Dependencies

| Needs | From | For |
|---|---|---|
| `UHealthComponent`, `FCombatHit`, team interface | FND `T-FND-05` | Structure HP, tower damage |
| Tag roots, settings, data asset pattern, debug CVars, test harness | FND `T-FND-04/07/09/10` | Everything |
| `IMC_Build` context | FND `T-FND-06` | Placement input |
| Enemy base, route following, local aggro, obstacle attack, Giant/Siege | ENM `T-ENM-01, 07, 08, 09, 10` | Consumers of the lane layer |
| Swarm, Armored | ENM `T-ENM-05, 06` | Tower role tests |
| Tower state consumers | SYN `T-SYN-06` | Ballista vs Armor Broken, Bombard poise |
| Wave events, spawner | DIR `T-DIR-01, 02` | Build allowance refill, benchmark spawns |
| Core loss | RUN `T-RUN-02` | Run lost on Core destroyed |
| Run resource | RUN `T-RUN-04` | P3 build cost |
| Feedback subsystem, structure HP marker, Core HP/lane danger HUD | UXF `T-UXF-01, 04, 07` | Feedback contract |
| Stat modifier query, perk manager | PRK `T-PRK-01, 02` | P3 Defense perk hooks |

Consumers of DEF: ENM, ZON (zones near lanes), DIR (lane tags, cap), RUN (Core), TFM (route preview), CSM (structure interaction), BOS (structure-breaking priority), UXF.

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Gap narrower than a Giant but wide enough for Swarm | Swarm passes through; Giant treats it as blocked and attacks the cheapest blocker (R-DEF-07) |
| Two blockers in series | After the first falls, enemies get the second as Path Obstacle without idling |
| Player builds a blocker right in front of walking enemies | Route version bumps; enemies re-query on their next decision tick and stop/attack instead of pushing into it |
| Player rings the Core | Treated as a seal: minimum-break target on each route |
| Player encloses a group of enemies | Placement over pawns is rejected; if a ring still forms, the min-break search gives the enclosed enemies a structure to break out |
| Enemy knocked or lured off its corridor | Next query projects it back to the nearest corridor cell |
| Structure destroyed while enemies mid-swing | Swing hits nothing; enemies re-query on the destroy event |
| Navmesh tiles not rebuilt yet after a destroy | Enemy retries movement after a short delay; lane layer stays the authority for route decisions |
| Structure in the shared corridor of two routes | Both routes update |
| Current route invalidated by design and no valid route in the lane | Lane layer falls back to any valid route of any lane and logs an error (authoring bug) |
| Tower target dies while projectile flies | Ballista bolt flies on and expires; Bombard shell lands at its aim point |
| Placement preview while a nearby structure is destroyed | Preview re-validates on the grid change event |
| Allowance reached mid-wave | Build bar greys out; confirm gives deny feedback |
| Player walls in own Hero/squads | Allowed; no ally pass-through in prototype; squads rely on SQD stuck recovery (NEW-DEF-06) |

## 11. Acceptance Criteria

### P2
- **AC-DEF-01**: In `L_SiegeSite_Proto`, a Barricade spanning the full Lane Left corridor makes arriving Swarm stop within attack range and attack it; the preview showed "blocks lane" before confirming.
- **AC-DEF-02**: A blocker layout leaving a 1-cell gap: Swarm walk through the gap without attacking the blocker (unless local aggro triggers); a Giant attacks a blocker instead of squeezing in.
- **AC-DEF-03**: When the blocking structure is destroyed, every enemy that was attacking or waiting resumes toward the Core within 1 s; no enemy idles more than 2 s.
- **AC-DEF-04**: Full seal of one lane: enemies on that lane attack the structure with the lowest break cost on their corridor; enemies on the other lane are unaffected; nobody switches lanes.
- **AC-DEF-05**: Full seal of both lanes plus a Core ring, 3 minutes of waves: 0 enemies idle-stuck (speed < 10 cm/s and not attacking for > 3 s), counted by a Functional Test.
- **AC-DEF-06**: Two blockers in series: enemies break the first, then attack the second without idling.
- **AC-DEF-07**: A structure built in front of walking enemies is attacked within one decision tick + recompute, with no enemies pushing against it for more than 1 s.
- **AC-DEF-08**: Every `DA_Structure_*` has exactly one `Structure.Role.*` tag and the five §21.3 items for towers; `IsDataValid` fails otherwise.
- **AC-DEF-09**: Placement ghost snaps to the zone grid; invalid placements (outside zone, wrong role, occupied, pawn overlap, allowance or cap reached) show red with a reason; confirming an invalid placement does nothing but deny feedback.
- **AC-DEF-10**: After the per-wave allowance is used, placement is denied until the current wave ends; values come from data.
- **AC-DEF-11**: With Swarm and Armored both in range, Ballista targets the Armored; Ballista kills an Armored in fewer shots than Bombard (data comparison in a test map).
- **AC-DEF-12**: One Bombard shell hits ≥ 3 clustered Swarm at a choke; vs a single fast target moving across its line, Bombard hit rate is clearly below Ballista's (logged over 20 shots).
- **AC-DEF-13**: Barricade deals no damage; a Barricade holds a Swarm group for roughly its HP / their DPS (logged), giving squads/towers time.
- **AC-DEF-14**: Enemies that reach the end of the route attack the Core; Core HP shows on the HUD; Core at 0 HP fires the Core-destroyed event (RUN ends the run).
- **AC-DEF-15**: Build/destroy updates only the affected nav tiles and routes; lane field recompute ≤ 1 ms per route and no hitch > 5 ms from a structure change on the reference PC (Insights trace).
- **AC-DEF-16**: The benchmark report states the concurrent enemy cap with method and numbers; the value is in `UGameTuningSettings` and reported for Q-04.
- **AC-DEF-17**: Feedback per Section 14 fires for structure hit, critical, destroyed, path opened, Core under attack (throttled), Core critical.
- **AC-DEF-18**: `game.debug.Lanes 1` draws corridors, blocked cells, route paths, Path Obstacle targets; Visual Logger records each enemy route query.
- **AC-DEF-19**: Structure Collapse (§33 17:00) scenario passes as a Functional Test: Giant breaks a Ballista blocking the corridor; enemies continue past its position.
- **AC-DEF-20**: G2 gate checklist (master plan Section 3) passes for the DEF items.

### P3
- **AC-DEF-21**: Build cost is deducted from the run resource; insufficient resource denies placement with feedback; allowance is off.
- **AC-DEF-22**: A test Defense perk (Ballista every 3rd shot pierces) works through DEF hooks with no change to tower classes.

### VS (provisional)
- **AC-DEF-23**: A 4th tower, if approved, is added as data + Blueprint only, with its five §21.3 items documented.
- **AC-DEF-24**: Repairing a damaged structure restores HP, costs resource, and updates the HP marker.

## 12. Open Questions / Assumptions

| ID | Item | Default until answered |
|---|---|---|
| A-04 / Q-05 | Placement: soft grid in build zones vs free vs sockets | Soft grid (A-04); decided at G2 (`T-DEF-20`) |
| A-05 | P2 free + per-wave limit; P3 run resource | As stated in R-DEF-23 / R-DEF-31 |
| Q-04 | Max concurrent enemies | Placeholder 40 until `T-DEF-12` |
| NEW-DEF-01 | Body-size classes for gap passability (R-DEF-07) | Derived from capsule diameter vs cell size; 2 classes expected (small, large) |
| NEW-DEF-02 | Should a very long in-corridor maze make enemies break through instead of walking it? | Strict: walk any open path (multiplier high); revisit after exploit tests `T-DEF-19` |
| NEW-DEF-03 | Does unused P2 build allowance carry over? Is building allowed during a wave? | No carry-over; building allowed in any phase |
| NEW-DEF-04 | Repair: P3 (A-05 says "build/repair", §6.3 Reconfigure) or VS (lead plan)? | Provisional VS task `T-DEF-25`; pull into P3 if the G3 run needs a Reconfigure spend |
| NEW-DEF-05 | Can the player dismantle own structures? (§14.3 says "destroyed/removed") | Cheat-only removal; same code path as destroyed |
| NEW-DEF-06 | Can Hero/squads pass through own Barricades? | No ally pass-through in prototype |
| NEW-DEF-07 | Bombard splash friendly fire? | No friendly fire |
| NEW-DEF-08 | Ballista needs line of sight? | Data flag, on for Ballista, off for Bombard |

Proposed new Gameplay Tag root `Lane.*` (e.g. `Lane.Left`, `Lane.Right`) needs an entry in `00-foundation/technical-plan.md` (request to FND `T-FND-04`). New leaves under existing roots: `Structure.Type.Core|Ballista|Bombard|Barricade`, `Feedback.Structure.*`, `Feedback.Core.*`, `Feedback.Lane.*`, `Feedback.Tower.*`, `Feedback.Build.*`.

## 13. System Contract

| Item | Content |
|---|---|
| Responsibility | Own structures (state, footprint, HP, role), placement, the strategic lane layer (routes, blocking, min-break, dirty updates), tower weapons. Answer "where should this enemy go next and what must it break". |
| Inputs | Level-authored `ALaneRoute`, `ABuildZone`, `ACoreStructure`; `UStructureDefinition` assets; placement requests from `AHeroPlayerController`; damage via `FCombatHit`; wave-ended event (DIR) for allowance; run resource (RUN, P3); stat modifiers (PRK, P3) |
| Outputs | Route query results for ENM; route versions/events; `OnCoreDestroyed`, Core HP; structure HP/critical/destroyed events; feedback tags; route preview polylines (TFM, preview UI); `MaxConcurrentEnemies` value (benchmark) |
| State | Structure: definition, HP, cells, rotation, alive/destroyed. Lane layer: grid cells (walkable, occupant per size class), per-route corridor + cost field + version + blocked flag. Placement: mode, selected definition, preview cells, allowance remaining (P2) |
| Events | `AStructureBase::OnStructureDamaged / OnStructureCritical / OnStructureDestroyed`; `ACoreStructure::OnCoreDestroyed / OnCoreCritical`; `ULaneNavigationSubsystem::OnRouteChanged(Route, bOpened)` and per-route `Version`; `UStructurePlacementComponent::OnPlacementChanged / OnStructurePlaced / OnPlacementDenied(Reason)` |
| Failure cases | See table below |
| Performance | Route field recompute ≤ 1 ms per route; only dirty routes recompute, coalesced per frame; enemy queries are O(path length) walks of a cached field; tower acquisition on timers (0.2–0.25 s); nav modifiers rebuild only overlapped tiles; no Tick on structures; projectiles pooled only if measured |

### Stop-and-attack contract with ENM

| Step | DEF provides | ENM does (`T-ENM-07/09`) |
|---|---|---|
| Spawn | `SelectRoute(LaneTag, Rng)` → route handle | Assign lane from spawner, select route |
| Move | `QueryRoute(Route, From, Size)` → status (Clear / Blocked / NoRoute), waypoints, obstacle, attack location, version | Walk waypoints with navmesh MoveTo |
| Re-query triggers | Route `Version` changes; major checkpoint distances; obstacle `OnStructureDestroyed` | Re-query only on these + §14.2 triggers, checked on the brain's decision tick |
| Blocked | Obstacle `AStructureBase`, `GetClosestPointOnFootprint(From)` | Move to attack location, stop, set Path Obstacle Target, attack until destroyed or re-query returns Clear |
| Objective | `GetObjective(Route)` → Core | Attack the Core at route end |
| Lane change | — | `AssignLane(NewLane)` from Director/event (R-DEF-06c) |
| Fallback | `GetStructuresInRadius(Location, Radius)` | NoRoute or 3 failed moves → attack nearest structure, else move to nearest corridor point; Visual Logger warning (R-DEF-10) |

### Data model (design level)

| Definition / actor | Fields |
|---|---|
| `UStructureDefinition` (`DA_Structure_*`) | DisplayName, Icon, RoleTag (`Structure.Role.*`), TypeTag (`Structure.Type.*`), StructureClass, MaxHealth, Armor, FootprintCells (W×L), bCanRotate, CriticalHealthFraction (default 0.25), BuildCost (P3), DesignNotes (role, preferred target, weakness, synergy, placement question), optional `FTowerWeaponParams` |
| `FTowerWeaponParams` | Range, FireInterval, Damage, PoiseDamage, SplashRadius (0 = single target), ProjectileClass, ProjectileSpeed, bArcing, LeadFactor (0–1), AimSpreadDegrees, bRequiresLineOfSight, TargetPriority (list of tag + weight), DistanceWeight, Fire/Impact feedback tags |
| `ALaneRoute` (level actor) | LaneTag (`Lane.*`), Spline (spawn → Core), CorridorHalfWidth, MajorCheckpoints (spline distances), SelectionWeight, bValid (design flag) |
| `ABuildZone` (level actor) | Box extent, AllowedRoles (`Structure.Role.*` container) |
| `ACoreStructure` | Uses `DA_Structure_Core` (MaxHealth, critical fraction) |
| `UGameTuningSettings` (Lane/Build groups) | GridCellSize (default 100 cm), RefMoveSpeed, RefBreakDPS, BreakCostMultiplier (strict default), WaypointSpacing, MoveRetryDelay, BuildAllowancePerWave (3), MaxStructuresPerSite (15), bUseBuildAllowance, CoreUnderAttackThrottle, MaxConcurrentEnemies (from benchmark) |

### Failure cases (GDD §34.4)

| Case | Handling |
|---|---|
| Tower destroyed | Unregister from lane layer first (cells freed, routes dirty), disable collision + nav modifier, play collapse feedback, destroy actor after debris delay. Projectiles in flight continue. No refund. |
| Blocked path opened | Route recompute sets `bOpened`; version bump; enemies re-query; `Feedback.Lane.PathOpened` on that lane; lane danger indicator (UXF) |
| Core critical HP | `OnCoreCritical` once per threshold crossing; `Feedback.Core.Critical`; HUD Core bar critical state |
| Core destroyed | `OnCoreDestroyed` → RUN lose flow; lane layer stops issuing routes |
| Boss reset / bug recovery | Structures keep their state; boss re-uses normal route queries (BOS) |
| Player leaving Siege Site | Build mode cancels when the player leaves the playable bounds (placement trace finds no zone); boundary itself is RUN (Q-16) |
| Invalid authoring (route not reaching Core, zone outside grid) | Logged as error at BeginPlay with the actor name; route marked invalid |

## 14. Feedback Contract

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Placement valid / invalid | Ghost tinted valid/invalid | Confirm `Feedback.Build.Placed`, deny `Feedback.Build.Denied` | Reason text under crosshair; build bar count | Player knows why a spot is refused without trial and error |
| Placement blocks a lane | Ghost shows "blocks lane" icon; route preview line stops at the ghost | — | Lane name in the warning | Player knows enemies will stop and attack this structure |
| Structure hit | Hit flash, debris puff per material | `Feedback.Structure.Hit` (wood/stone variant) | HP marker appears, hides after a few seconds without damage (UXF `T-UXF-04`) | Player sees which structure is under attack in 1–2 s |
| Structure critical HP | Cracked look, marker turns critical | `Feedback.Structure.Critical` (§28.3 tower critical HP) | Marker pulses | Audible off-screen |
| Structure destroyed | Collapse VFX, debris | `Feedback.Structure.Destroyed` | Marker removed | Distinct from hit sound |
| Blocked path opened | Brief highlight along the reopened lane | `Feedback.Lane.PathOpened` | Lane danger indicator pulses (UXF `T-UXF-07`) | Player learns a lane is breached even off-screen |
| Core under attack | Core hit flash | `Feedback.Core.UnderAttack`, throttled (§28.3) | Core HP bar flashes | Never spams; at most one cue per throttle window |
| Core critical HP | Core damage state | `Feedback.Core.Critical` | Core bar critical color | Distinct from structure critical |
| Ballista fire / impact | Long bolt silhouette, tracer | `Feedback.Tower.Fire.Ballista`, `Feedback.Tower.Impact.Ballista` | — | Role readable by shape/VFX (§28.2) |
| Bombard fire / impact | Arcing shell, splash decal sized to radius | `Feedback.Tower.Fire.Bombard`, `Feedback.Tower.Impact.Bombard` | — | Splash area readable at a glance |

Rows live in `DT_Feedback` (UXF). Placeholder assets in P2; final art/audio per `production-plan.md`.
