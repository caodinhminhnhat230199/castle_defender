# Enemies (ENM): Specification

| | |
|---|---|
| Feature ID | ENM |
| GDD sections covered | §20 (all), §14.1–14.5 (enemy side), §9.4/§9.5/§9.7 (enemy side of combat), §10.2 (Staggered / Armor Broken consumers), §8.1–8.2 (data read by DIR), §25.1–25.2 (difficulty hooks only), §28.1–28.2, §31.2, §34.4 (enemy cases), §36 (Defense Core DoD, enemy part) |
| Phases | P0 (base + 1 melee enemy) → P1 (Swarm, Armored) → P2 (Giant/Siege, Route Objective, Local Aggro, leash, Path Obstacle Target) |
| Owner of | Enemy body, archetype data, enemy decision logic, enemy attacks/reactions/death |
| Not owner of | Lanes, routes, blocking detection, minimum-break search (DEF), spawning and wave composition (DIR), damage math and state timers (FND/SYN), feedback rows (UXF), boss (BOS) |
| Status | Draft v1 |

Label note: rules taken from GDD sections without a decision label are marked `[UNLABELED]` and treated as current design direction. `Assumption` marks a rule with no direct GDD source; it links to an `A-xx` (master plan) or a `NEW-ENM-n` question (Section 12).

## 1. Overview

Enemies are the pressure the player answers with all three layers. Each enemy marches along a lane toward the Core like a MOBA lane minion: it has a clear route, stops to fight what blocks it, breaks route-blocking structures, then continues (§14.1). Inside a small radius it fights the Hero or squads that threaten it, but it never chases past its leash or abandons its lane (§14.5).

Archetypes ask different tactical questions (§20.1): Swarm tests AoE and formation, Armored tests Heavy / Armor Break / Ballista, Giant/Siege tests structure placement. P0 adds one melee enemy so Hero combat can be tested alone (§32 P0).

## 2. Player Experience

- **Readable threat.** Every enemy attack has a visible wind-up so dodge and parry timing is learnable (§9.4, §9.5, §28.1).
- **Weight.** Hits land with reaction; poise break gives a clear Staggered opening (§9.7, §10.2).
- **Lane logic you can exploit, but not for free.** Enemies stop at a Barricade and hit it; the player uses that time (§14.3, §21.1 Barricade). They do not get "too smart" and walk around every defense (§14.1).
- **Class at a glance.** Swarm, Armored and Giant/Siege are recognizable from far away (§28.2); siege/elite are readable within 1–2 s (§28.1).

## 3. Core Loop

Player view:

```text
Read (class silhouette, telegraph, route) → Decide (dodge/parry/heavy, block lane, send squad)
→ Act → Feedback (stagger, structure being hit, enemy leashing back) → Adapt
```

Enemy view (P2):

```text
Spawn on lane → Follow route → [Local Aggro in radius ↔ leash back to lane]
→ [Path Obstacle Target: stop, attack, re-route] → Objective (Core): attack
```

## 4. Gameplay Rules

### P0 scope: base enemy + 1 melee enemy

- **R-ENM-01** (§32 P0, §20.1) [UNLABELED roadmap]: P0 ships exactly one melee enemy archetype used to test Hero combat. It is a sandbox/test archetype, not one of the §20.1 archetypes (see NEW-ENM-1).
- **R-ENM-02** (§34.1, §38) [LOCKED]: Every enemy is defined by an Enemy Archetype definition (data). Adding or retuning an archetype needs no core code change. All [TUNABLE] values below live in that definition or in `UGameTuningSettings`.
- **R-ENM-03** (§10.1, D-05) [LOCKED]: Enemies receive damage, poise damage and combat states only through the shared combat contract, so Hero, Army and Tower hits use one pipeline.
- **R-ENM-04** (§1.1) [LOCKED]: Enemies are hostile to the Hero, soldiers, player structures and the Core, and to nothing else.
- **R-ENM-05** (§9.4, §9.5, §28.1) [LOCKED intent, TUNABLE timing]: Every enemy attack has a telegraph (wind-up) before its hit window. Once the wind-up starts, the attack plays through (wind-up → hit window → recovery) unless the enemy becomes Staggered or dies. Wind-up length, rotation tracking during wind-up and cooldowns are data.
- **R-ENM-06** (§9.5) [LOCKED]: Enemy attacks can be blocked and parried by the Hero. The Hero-side rules are owned by CMB; the enemy must react to a parry by entering Staggered (or the vulnerability window CMB defines).
- **R-ENM-07** (§9.7, §10.2) [LOCKED]: Each enemy has Poise (value can be low). Poise break → Staggered: the current attack is cancelled with no damage, and the enemy takes no action until the state ends. Poise values and Staggered duration are [TUNABLE] per archetype.
- **R-ENM-08** (§2.1, §9.8) [LOCKED]: Every damaging hit shows a readable hit reaction (visual + audio). A hit reaction alone does not cancel the enemy's action; only Staggered or death does (§9.7: poise break is what opens follow-ups).
- **R-ENM-09** (§8.2, §31.2) [LOCKED]: On death the enemy stops acting, stops blocking movement, reports removal exactly once (for alive counts and rewards), and its body despawns after a short delay [TUNABLE]. Removal is also reported for non-death removal (despawn, out of world).
- **R-ENM-10** (§14.5 Local Aggro) [LOCKED]: An enemy attacks hostile units inside its Local Aggro Radius [TUNABLE]. In P0 the only hostile unit is the Hero.
- **R-ENM-11** (§14.2, §31.2) [LOCKED]: Enemy decisions run at a fixed decision interval [TUNABLE, default 0.2 s], not every frame. No full repath per frame.

### P1 scope: Swarm + Armored (A-03)

- **R-ENM-12** (§20.1, A-03) [LOCKED]: **Swarm**: many units, low HP, pressure on AoE and formation. Example threat cost 2 (§8.1, illustrative, [TUNABLE]).
- **R-ENM-13** (§20.1, A-03) [LOCKED]: **Armored**: slower, high armor and poise; exists to test Heavy, Armor Break and (P2) Ballista. Example threat cost 8 (§8.1, [TUNABLE]).
- **R-ENM-14** (§10.2) [UNLABELED]: While Armor Broken is active, the enemy's armor is reduced (rule owned by SYN, T-SYN-02). Armored is the main test target.
- **R-ENM-15** (§14.5) [LOCKED]: Enemies fight soldiers that block or attack them, using the same Local Aggro rule as for the Hero.
- **R-ENM-16** (§14.5 Route Objective, Assumption NEW-ENM-2): Before lanes exist (P1), an enemy can be given an authored goal (one or more waypoints) and advances toward it, fighting hostiles it meets. This is the P1 sandbox form of Route Objective and uses the same route-following behavior P2 uses.
- **R-ENM-17** (§28.2) [UNLABELED]: Swarm, Armored and (P2) Giant/Siege are distinguishable from a distance by silhouette. Placeholder art must already respect this (scale, shape, color).

### P2 scope: Giant/Siege + lane behavior

- **R-ENM-18** (§14.2) [LOCKED]: Each enemy has an Assigned Lane, a Current Route, a Local Aggro Radius and an Objective (Core, or a special objective passed at spawn; none exists in prototype).
- **R-ENM-19** (§14.2) [LOCKED]: An enemy (re)selects its route only when: it spawns, it passes a major checkpoint, its route becomes invalid, a blocking structure it targeted is destroyed, or a special ability / event forces a repath.
- **R-ENM-20** (§14.1, §14.5 Route Objective) [LOCKED]: Default behavior is to advance to the Objective along the current lane route. At the end of the route the enemy attacks the Objective (Core).
- **R-ENM-21** (§14.3) [LOCKED]: If a structure on the current valid route blocks the corridor, the enemy moves to it, stops, acquires it as **Path Obstacle Target**, and attacks it. When it is destroyed or removed, the enemy gets its next route and continues to the Core. Which structure blocks the route is decided by the lane layer (DEF).
- **R-ENM-22** (§14.3) [LOCKED]: An enemy does not detour around a blocking structure just because another path exists, unless the current route is marked invalid by a design rule, the archetype has a special behavior (none in prototype), or the Director/an event changes its lane.
- **R-ENM-23** (§14.4, §36) [LOCKED]: When no walkable route to the Core exists, the enemy attacks the minimum-break structure chosen by the lane layer, then continues. An enemy never stands still bugged.
- **R-ENM-24** (§14.6) [LOCKED]: A Combat Tower whose footprint truly blocks the route is a Path Obstacle exactly like a Blocker. The enemy treats every route-blocking structure the same way.
- **R-ENM-25** (§14.5 Local Aggro) [LOCKED]: Inside the Local Aggro Radius an enemy may switch to: the Hero causing threat, a soldier blocking or attacking it, a defensive structure allowed by its archetype's interaction rule. Taunt targets are supported only when a taunt source exists (none in prototype scope, NEW-ENM-4). "Causing threat" is defined in NEW-ENM-3.
- **R-ENM-26** (§14.5) [LOCKED]: An enemy never chases past its Leash [TUNABLE] and never abandons its lane indefinitely. On leash break it disengages, returns to its lane and resumes the Route Objective.
- **R-ENM-27** (§14.5, Assumption NEW-ENM-5): Which target kinds an archetype may pick and in which order (Hero, soldier, Path Obstacle, combat tower, blocker, Objective) is an ordered list in archetype data [TUNABLE].
- **R-ENM-28** (§20.1) [LOCKED]: **Giant/Siege**: prioritizes structures, deals high damage to towers/walls, puts pressure on placement. Example threat cost 25 (§8.1, [TUNABLE]).
- **R-ENM-29** (§8.2, §31.2) [LOCKED]: Enemies are created only through the spawner, which enforces the concurrent enemy cap (first value from T-DEF-12, enforced by DIR). ENM never spawns enemies on its own.
- **R-ENM-30** (§28.1) [LOCKED]: Siege/elite enemies are identifiable within 1–2 s (silhouette + class marker from UXF).

### Not built

- **R-ENM-31** (§20.2) [DEFERRED]: Flying enemies (own navigation, targeting, anti-air) are not built.
- **R-ENM-32** (§20.3) [DEFERRED]: Biome special enemies (healer, burrower, stealth, summoner, artillery) are not built.
- **R-ENM-33** (§25.1 [LOCKED], §25.2 [TUNABLE]): Difficulty is not built in ENM. ENM only keeps every value a difficulty axis would touch in data (Section 13, "Difficulty hooks"). HP/damage scaling is support only, never the core difficulty lever.

## 5. Player Actions (that affect enemies)

| Action | Effect on enemies |
|---|---|
| Dodge / block / parry a telegraphed attack | Avoid or absorb damage; parry staggers the enemy (CMB) |
| Heavy attack | High poise damage → Staggered; applies Armor Broken (SYN) |
| Enter an enemy's aggro radius / hit it | Pulls it off its route within leash |
| Retreat past the leash | Enemy returns to its lane |
| Send a squad to block/attack | Enemies fight the soldiers (Local Aggro) |
| Build a structure on the route | Enemies stop and break it (Path Obstacle) |
| Seal all routes | Enemies break the minimum-break structure |
| Place towers near a lane | Giant/Siege pull off route to break them (within leash) |

## 6. Success / Failure Conditions

- Enemy "succeeds" by reaching and damaging the Core (RUN decides the loss at Core destruction, T-RUN-02).
- Enemy "fails" by dying. Each removal is reported once, so wave/alive counts and kill rewards stay correct.
- Partial: an enemy that breaks a blocker delays the player's plan even if it dies afterwards.

## 7. Scope

### In Scope
- P0: `AEnemyCharacter` base, archetype definition, timer-driven brain, melee attack with telegraph, hit reaction, Staggered, death/despawn, debug draw, one melee enemy content.
- P1: Swarm and Armored content, waypoint advance (sandbox goal), fighting soldiers.
- P2: Giant/Siege, lane route following, re-route triggers, Local Aggro with priority list, leash return, Path Obstacle Target, minimum-break consumer, stuck recovery, enemy cost profiling feeding T-DEF-12.

### Out of Scope
- Flying, biome specials (§20.2, §20.3 DEFERRED).
- Enemy blocking/shielding, attack-token systems, ranged enemies (no GDD source; see NEW-ENM-1, NEW-ENM-6).
- Wave composition, threat budget, concurrent cap enforcement (DIR).
- Route computation, blocking detection, minimum-break search, nav updates (DEF).
- Boss behavior (BOS reuses ENM).
- Monster Material drops (VS, ECO/CNV).

## 8. Anti-Goals

- Enemies are not "smart" path solvers that route around every defense (§14.1).
- No per-enemy global pathfinding every frame; no per-frame heavy work (§14.2, §31.2).
- No HP-sponge difficulty (§25.1).
- No individual full AI where group/lane logic is enough (§31.2).
- No new enemy types that only add stats (§20.3: each special must create a new tactical question).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| Combat contract, tags, settings, asset types, debug, tests | FND (T-FND-04, -05, -07, -09, -10) | Health, poise, states, team, data validation |
| Shared hit window + hit dispatch | CMB (T-CMB-04) | Enemy melee uses the same hit-window and trace helper as the Hero |
| Hero block/parry/hit reaction | CMB (T-CMB-08, -09, -11) | Enemy attacks must be blockable/parryable |
| Poise → Staggered logic, Armor Broken | SYN (T-SYN-01, -02, -04) | State timers and presentation |
| Soldiers as targets | SQD (T-SQD-02) | Local Aggro vs soldiers |
| Lane layer, route query, blocking + minimum-break, invalidation events, structures, Core, Barricade | DEF (T-DEF-02..06, -08) | Route Objective and Path Obstacle |
| Concurrent cap | DEF (T-DEF-12) → DIR (T-DIR-04) | §8.2, §31.2 |
| Spawner sets lane/objective | DIR (T-DIR-01) | Spawn init |
| Feedback playback, hit feedback, class markers | UXF (T-UXF-01, -03, -04) | §28, §34.5 |

Depended on by: CMB (target to hit), SQD (enemies to fight), DEF (enemies that break structures), DIR (archetype data, threat cost), RUN (kill events, P3 reward), BOS (boss is an enemy subclass).

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Target dies during the enemy's wind-up | Attack plays through (commitment), then re-evaluates |
| Hero dies (Commander Spirit) | Hero is no longer a valid target; enemy resumes route |
| Squad wiped while engaged | Targets invalid; enemy resumes route |
| Staggered while moving or attacking a structure | Stops; after the state ends it re-evaluates and resumes |
| Path Obstacle destroyed while many enemies queue at it | All on that lane get the invalidation event and re-route; queued enemies are not "stuck" |
| Structure built ahead of a walking enemy | Lane invalidation → enemy re-routes and treats it as Path Obstacle if it blocks |
| Structure ends up behind an enemy's progress | Ignored |
| Several blockers in a row | Broken one at a time in route order |
| Lane layer returns no route and no obstacle (bug) | Enemy moves toward the Objective on navmesh, logs an error, never idles |
| Obstacle unreachable (nav modifier closes the area) | Attacks from the nearest reachable point in range; if out of range for too long, re-route, then stuck recovery |
| Enemy pushed off navmesh / stuck | Stuck recovery: re-move → re-route → hidden teleport to next route point when off-screen (NEW-ENM-7) |
| Enemy falls out of the world | Removal reported (reason: out of world); counts stay correct |
| Player leaves the Siege Site (§34.4, Q-16) | Hero ends up past leash; enemies return to lanes |
| Core destroyed | Run ends (RUN); enemies have no target and idle |
| Core at critical HP (§34.4) | No enemy behavior change (RUN/UXF feedback only) |
| Tactical Focus time dilation (P3) | Decision timers run on game time and slow down with the world |
| Enemy spawned without a lane (P0/P1, cheats) | Sandbox behavior: idle at spawn or walk the authored goal |
| Two hostiles tie on priority | Recent attacker first, then nearest |

## 11. Acceptance Criteria

### P0
- **AC-ENM-01**: In `L_CombatSandbox`, the melee enemy idles until the Hero enters its Local Aggro Radius, then approaches and starts an attack once in range.
- **AC-ENM-02**: Every attack shows its telegraph (wind-up animation + `Feedback.Enemy.Telegraph`) before damage can apply; measured wind-up ≥ the authored minimum.
- **AC-ENM-03**: The Hero can block and parry enemy attacks; a successful parry puts the enemy in Staggered (with CMB T-CMB-08/-09).
- **AC-ENM-04**: Poise break → Staggered cancels the current attack with no damage; the enemy takes no action until the state ends, then resumes.
- **AC-ENM-05**: Normal hits play a hit reaction and impact feedback without cancelling the enemy's attack.
- **AC-ENM-06**: On death: no further actions, no pawn collision, removal reported exactly once, actor gone after the despawn delay.
- **AC-ENM-07**: The brain does not tick per frame; decisions run at the data interval; changing the value in the Data Asset changes cadence without code change.
- **AC-ENM-08**: `game.debug.Enemy 1` shows state, target, aggro radius and leash; Visual Logger records state changes.
- **AC-ENM-09**: G0 item "one melee enemy is enough for 3–5 minutes of combat" is evaluated and recorded (master plan §3 G0).

### P1
- **AC-ENM-10**: Swarm and Armored are told apart at ~30 m in the P1 map (silhouette check, screenshot in playtest notes).
- **AC-ENM-11**: A cheat-spawned Swarm group advances to an authored goal, engages soldiers and the Hero it meets, and dies fast to Hero attacks.
- **AC-ENM-12**: Armored takes clearly less damage and poise damage from Light than Heavy; while Armor Broken it takes more damage (debug numbers show the difference).
- **AC-ENM-13**: When its current target dies, an enemy resumes its advance within one decision interval.

### P2
- **AC-ENM-14**: On an open lane the enemy follows the route to the Core and damages it.
- **AC-ENM-15**: A Barricade on the route makes the enemy stop and attack it even when a detour exists elsewhere.
- **AC-ENM-16**: After the Barricade is destroyed the enemy re-routes and continues to the Core.
- **AC-ENM-17**: Full seal: the enemy attacks the structure the lane layer chose as minimum-break, then continues; no enemy idles longer than the stuck threshold.
- **AC-ENM-18**: A structure built on the route ahead of walking enemies becomes their Path Obstacle without any per-frame polling.
- **AC-ENM-19**: The Hero pulls an enemy off its route within the aggro radius; retreating past the leash sends the enemy back to its lane, and it resumes the route.
- **AC-ENM-20**: Giant/Siege leaves its route (within leash) to attack a Combat Tower in its radius; Swarm ignores that tower unless it blocks the route.
- **AC-ENM-21**: A Ballista whose footprint blocks the corridor is attacked as a Path Obstacle.
- **AC-ENM-22**: Every removal path (killed, despawned, fell out of world) reports exactly once; spawner alive count matches the actors in the world after a 3-wave run.
- **AC-ENM-23**: Per-enemy game-thread cost for Swarm on a lane is measured in a packaged Development build on the reference PC and handed to T-DEF-12.
- **AC-ENM-24**: G2 enemy checks pass: stop-and-attack, route continues after destruction, full seal never bugged, structure breaking easy to read (master plan §3 G2).

## 12. Open Questions / Assumptions

| ID | Question / assumption | Class | Default until answered |
|---|---|---|---|
| A-03 | P1 archetypes are Swarm + Armored; Giant/Siege in P2 | Master plan | Followed |
| NEW-ENM-1 | P0 melee enemy is not one of the §20.1 archetypes. The master plan P0 summary says the enemy "blocks/telegraphs"; GDD §32 only says "1 melee enemy". | IMPROVEMENT | Telegraph only, no enemy block. Kept as sandbox/test archetype, not in wave pools. Revisit at G0 if the enemy is boring |
| NEW-ENM-2 | P1 has no lanes, but G1 needs enemies that advance on squads | REQUIRED | Authored goal waypoints (degenerate route), same code as P2 route following |
| NEW-ENM-3 | Meaning of "Hero causing threat" (§14.5) | REQUIRED | Hero inside the Local Aggro Radius; a recent attacker wins ties |
| NEW-ENM-4 | Taunt target (§14.5) has no source in prototype scope | FUTURE | Not built until a taunt source exists |
| NEW-ENM-5 | Do Swarm/Armored local-aggro onto Combat Towers in radius ("defensive structure in interaction rule")? | REQUIRED | No: Swarm/Armored attack structures only as Path Obstacle or Objective; Giant/Siege may attack towers/blockers in radius. Data list per archetype |
| NEW-ENM-6 | Limit on how many enemies attack the Hero at once (attack tokens)? Not in GDD | IMPROVEMENT | Not built; add only if G0/G1 playtests report unreadable gang-ups |
| NEW-ENM-7 | Enemy stuck last resort. §13.6 covers soldiers only; §36 requires enemies never stand still bugged | REQUIRED | Same ladder as soldiers: re-move → re-route → hidden teleport to the next route point when off-screen; logged as a bug |
| NEW-ENM-8 | Which archetypes count as "elite" (§7.2 warning, §8.2 elite cap, §25.1)? | REQUIRED (DIR needs it in P3) | Giant/Siege only, via tag `Unit.Enemy.Elite` in data |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Enemy body and lifecycle; decision logic (Route Objective, Local Aggro, leash, Path Obstacle Target, Objective); enemy attacks, hit reaction, Staggered behavior, death/removal; archetype data |
| Inputs | Archetype definition; spawn init (lane, objective) from the spawner; `FCombatHit` received; combat state events (Staggered, Armor Broken); lane route results and route invalidation events (DEF); hostile actors in radius; debug cheats |
| Outputs | `FCombatHit` sent to Hero, soldiers, structures, Core; feedback plays through `UFeedbackSubsystem`; enemy removed event (killed / despawned / out of world); route re-query requests to the lane layer |
| State (per enemy, runtime only) | Brain state, current target + reason (Local Aggro / Path Obstacle / Objective), leash anchor, assigned lane, route + progress index, attack cooldowns, stuck counters. Not saved (D-14, A-11) |
| Events | Fires: `OnEnemyRemoved(Enemy, Reason)`, `OnBrainStateChanged` (debug, BOS). Listens: `OnDamaged`, `OnDeath`, `OnStateAdded/Removed`, `OnRouteInvalidated` |
| Failure cases | See Section 10 and the §34.4 table below |
| Performance | Timer brain, event-driven aggro, shared per-lane routes, measured caps (below) |

### Data model: Enemy Archetype definition (design level)

| Field | Phase | Example / default | Label |
|---|---|---|---|
| Archetype tags (`Unit.Enemy.Swarm/Armored/Siege/Melee`, optional `Unit.Enemy.Elite`) | P0 | Melee enemy: `Unit.Enemy.Melee` | Data |
| Display name | P0 | "Raider" (placeholder) | Data |
| Max health | P0 | per archetype | [TUNABLE] |
| Armor | P1 | Armored high, others 0 | [TUNABLE] |
| Max poise, poise regen delay/rate | P0 | Swarm low, Armored high | [TUNABLE] |
| Staggered duration | P0 | ~1–1.5 s placeholder | [TUNABLE] |
| Walk speed | P0 | Armored slower (§20.1) | [TUNABLE] |
| Attack list: animation, range, damage, poise damage, heavy flag, play rate, cooldown, weight | P0 | Melee enemy: 1–2 attacks | [TUNABLE] |
| Min time between attacks, turn rate during wind-up | P0 | — | [TUNABLE] |
| Decision interval | P0 | 0.2 s (Swarm may use 0.3 s) | [TUNABLE] |
| Local Aggro Radius | P0 | ~6–8 m placeholder | [TUNABLE] (§14.2) |
| Leash radius | P2 | ~12–15 m placeholder | [TUNABLE] (§14.5) |
| Local Aggro priority list (target kinds in order) | P2 | Swarm: Hero, soldier, Path Obstacle, Objective. Siege: Path Obstacle, Combat Tower, Blocker, soldier, Hero, Objective | [TUNABLE] |
| Structure damage multiplier | P2 | Siege ~3× | [TUNABLE] (§20.1) |
| Threat cost (read by DIR) | P1 | Swarm 2, Armored 8, Siege 25 (§8.1 illustrative) | [TUNABLE] |
| Kill reward (read by RUN, A-05) | P3 | small int | [TUNABLE] |
| Body despawn delay | P0 | ~3 s | [TUNABLE] |
| Local avoidance on/off | P2 | from T-ENM-16 result | Perf setting |

Global enemy tunables in `UGameTuningSettings`: stuck check time, max stuck attempts, obstacle queue radius, route rejoin distance, minimum telegraph time (validation).

### Difficulty hooks (§25.1, §25.2): data only, no code in prototype

| Difficulty axis | Data a harder variant would change |
|---|---|
| Siege targets structures smarter | Local Aggro priority list, aggro radius, structure damage multiplier |
| Tighter timing | Attack play rate, min time between attacks |
| Elite earlier / smarter composition | `Unit.Enemy.Elite` tag + threat cost (DIR uses them) |
| Stat scaling (support only) | Max health, damage (variant Data Asset or DIR modifier) |
| Enemies attack resource zones more often | Not applicable until resource zones exist (VS) |

### Failure / recovery cases (§34.4, enemy side)

| §34.4 case | Enemy behavior |
|---|---|
| Hero death | Drop Hero target, resume route |
| Squad wipe | Drop soldier targets, resume route |
| Tower destroyed | If it was the Path Obstacle: re-route and continue |
| Blocked path opened | Lane invalidation → re-route → continue |
| Core critical HP | No change |
| Player leaving Siege Site | Leash returns enemies to lanes |
| Boss reset / bug recovery | Owned by BOS; uses ENM stuck recovery |

### Performance considerations (§31.2, §14.7)
- Brain runs on a timer with a random start offset so enemies do not all decide in the same frame.
- Hostile scan only when the enemy has no combat target or is on a non-combat target (route, obstacle, objective). Damage-driven aggro is event based.
- Routes are shared per lane (DEF caches); each enemy only tracks its progress index.
- No per-enemy Tick in C++; animation and movement settings tuned in T-ENM-16.
- Final enemy count is [OPEN/TUNABLE] (Q-04); the first cap comes from T-DEF-12.

## 14. Feedback Contract (GDD §34.5)

| State / event | Visual | Audio | UI | Readability rule | Tag |
|---|---|---|---|---|---|
| Attack wind-up | Wind-up pose + weapon flash | Wind-up whoosh | — | Starts before the hit window by at least the minimum telegraph time; heavy attacks look different from light | `Feedback.Enemy.Telegraph`, `Feedback.Enemy.Telegraph.Heavy` |
| Hit taken (normal / armored) | Flinch + impact VFX by material | Impact SFX by material | — | Armored hits look and sound different (§9.8) | `Feedback.Hit.*` (UXF-03) |
| Staggered | Stagger animation + state VFX/icon | Stagger SFX | State icon | Same icon/VFX on every unit (§28.2) | `Feedback.State.Staggered` (SYN-04) |
| Armor Broken | State VFX/icon | Armor break SFX (§28.3) | State icon | Same as above | `Feedback.State.ArmorBroken` (SYN-04) |
| Death | Death animation, body stops blocking | Death SFX | — | Body never blocks lanes | `Feedback.Enemy.Death` |
| Attacking a structure | Attack animation facing the structure | Structure impact SFX (DEF/UXF) | Structure HP marker (UXF-04) | Player can tell which structure is being broken (G2 check) | `Feedback.Structure.*` (DEF/UXF) |
| Class identity | Silhouette per archetype | — | Class marker (UXF-04, Tactical Focus overlay) | Siege/elite readable in 1–2 s (§28.1) | `Unit.Enemy.*` |
| Leash return, re-route | None (deliberately quiet) | — | — | Debug draw only | — |
