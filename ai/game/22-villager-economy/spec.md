# Villager Economy (ECO): Specification

> **Provisional — re-validate after G3.** This feature starts only after Gate G3 passes (GDD §32 P3 gate: no villager economy before a full run is fun). Re-check every rule, value and task against the G3 playtest notes before work starts.

| | |
|---|---|
| GDD sections covered | §16.1 Food / Gold / Monster Material, §16.2 villager management, §16.3 prototype economy (what ECO replaces), §3 villager anti-goal |
| Cross-references | §5.3 (Siege Site resource points), §6.3 (Collect, Prepare), §14.2 (special objective), §14.5 (Local Aggro), §15.2 (Resource Camp zone), §17 (MM goes to conversion), §25.1 (enemies attack resource zones more at high difficulty), §27.2 (resource node active), §33 (starting resources), §37 / Q-08 |
| Phase | VS only |
| Status | Provisional draft v1 (2026-10-02) |
| Replaces | The single prototype run resource from RUN (T-RUN-04, assumption A-05) |

## 1. Overview

ECO replaces the single prototype run resource with the full-game model: Food, Gold and Monster Material (MM). Food and Gold come mostly from villagers working at resource points on the Siege Site map; MM comes from enemies and feeds Conversion Buildings (CNV). Villagers are physical, anonymous workforce NPCs that enemies can attack. The player manages them only at workforce level (jobs, priority, workforce per category, evacuate), never per villager. Losing or interrupting villagers costs income, so defending resource zones matters.

## 2. Player Experience

- A second, slower question beside "where do I fight": "which income do I protect, and what do I spend it on?"
- Losing a resource zone hurts but is readable: the player sees the zone is under attack, decides to send a squad or evacuate.
- Management takes seconds: change a number, toggle evacuate, back to combat (§2.2 spirit, §3 no micro).

## 3. Core Loop

```text
Prepare: set workforce per job and priority → villagers walk to resource points
→ Waves: villagers produce Food / Gold; enemies may raid a zone
→ Zone threatened: interrupted income → protect (squad Guard on the zone) or evacuate
→ Collect: MM from kills, Gold from wave reward
→ Spend: Gold on build/repair, Food on reinforcing squads, MM in Conversion Buildings
→ Reconfigure workforce for the next wave
```

## 4. Gameplay Rules

### 4.1 Resources (§16.1)

- **R-ECO-01 (§16.1) [LOCKED, current direction]:** Three resources: Food, Gold, Monster Material.
- **R-ECO-02 (§16.1) [LOCKED, current direction]:** Food sources are farming / hunting / workforce. VS: Food is produced by villagers working at Food resource points (farm, hunting ground).
- **R-ECO-03 (§16.1) [LOCKED, current direction]:** Food is used for recruit, sustaining the army and some army upgrades. VS: Food pays for reinforcing squad soldiers (recruit). "Sustain" as an upkeep drain is not built (NEW-ECO-01). Army upgrades use Food only when such upgrades exist.
- **R-ECO-04 (§16.1) [LOCKED, current direction]:** Gold sources are mining, reward, objective. VS: villagers at Gold resource points (mine) and wave-clear rewards. Objective rewards apply when RUN/DIR define objectives.
- **R-ECO-05 (§16.1) [LOCKED, current direction]:** Gold is used for build, upgrade, repair and special services. VS: build and repair (the sinks the prototype run resource had, A-05). Upgrade and service sinks are added only by the features that define them.
- **R-ECO-06 (§16.1, §17.1) [LOCKED]:** MM comes from enemies and the boss. Most of the time MM is not spent directly; it goes through Conversion Buildings. VS: CNV is the only MM sink.
- **R-ECO-07 (§16.3) [LOCKED]:** Food/Gold/Villager are added only after G3. When ECO lands, the single prototype run resource is removed; there is no fourth run currency.
- **R-ECO-08 (§33 01:00) [unlabeled example]:** Each run starts with starting resources set in run data. GDD gives no numbers; [TUNABLE] in the run/site data.
- **R-ECO-09 Assumption NEW-ECO-06:** MM is credited automatically on kill (no pickup), amount per enemy archetype in data. Boss MM amount in boss data.
- **R-ECO-10:** Resources are integers, never negative. A spend that cannot be paid in full fails with a reason; no debt.
- **R-ECO-11:** Food, Gold and MM are run-scoped. Nothing carries over to the next run or into the meta save (meta reward is MET's job).

### 4.2 Villagers (§16.2, §3, Q-08)

- **R-ECO-12 (§16.2) [LOCKED]:** Villagers exist physically on the Siege Site map.
- **R-ECO-13 (§3, §16.2) [LOCKED]:** Villagers can be attacked and killed.
- **R-ECO-14 (Q-08 default):** Villagers are anonymous workforce NPCs: no names, traits, stats or persistence between runs.
- **R-ECO-15 Assumption NEW-ECO-04:** Villagers never fight. They have no combat behavior and no combat orders.
- **R-ECO-16 Assumption NEW-ECO-05:** Starting villager count comes from run/site data [TUNABLE; VS placeholder 8–12]. Dead villagers are not replaced during the run in VS.
- **R-ECO-17 Assumption NEW-ECO-07 (§13.6 by analogy):** A villager that cannot progress repaths; if still stuck and not visible to the player camera, it is moved to its destination. One stuck villager never blocks a zone.

### 4.3 Workforce management (§16.2)

- **R-ECO-18 (§16.2) [LOCKED]:** The player manages villagers only through: assign job, set priority, assign workforce to a building/production category, protect/evacuate a zone.
- **R-ECO-19 (§16.2, §3) [LOCKED]:** No per-villager micro: no moving individual NPCs, no picking individual farmers, no combat orders to individuals. There is no input that selects a single villager.
- **R-ECO-20 (§16.2):** Jobs are production categories. VS categories: Food and Gold. The player sets a target worker count per category.
- **R-ECO-21 (§16.2) Assumption NEW-ECO-02:** Priority is an order over categories. When alive, available villagers are fewer than the sum of targets, higher-priority categories are filled first.
- **R-ECO-22 (§16.2):** Within a category, the system assigns villagers to resource points (active, not evacuated, free slot, nearest to the shelter first). The player never assigns a villager to a point. Valid existing assignments are kept to avoid shuffling villagers around.
- **R-ECO-23 (§16.2) [LOCKED]:** Evacuate zone: villagers at that resource point stop work and leave. The allocator moves them to another valid point of the same category if one has a free slot; otherwise they go to the shelter near the Core. Lifting the evacuation re-runs allocation.
- **R-ECO-24 (§16.2, §15.2) Assumption NEW-ECO-03:** Protect zone = guard the zone with a squad through the existing Command Wheel Guard order on the zone's linked Resource Camp Tactical Zone. ECO adds no separate villager protect command.
- **R-ECO-25:** Workforce changes take effect immediately, during prep, intermission and waves. The workforce UI does not pause the game.

### 4.4 Production, interruption, defense (§16.2)

- **R-ECO-26 (§16.2) [LOCKED]:** Lost or interrupted villagers reduce income, so resource zone defense matters.
- **R-ECO-27 (§16.2):** A resource point produces only through villagers that are present and in Working state. Output = working villagers × rate per production interval [TUNABLE per point type in data; no GDD values].
- **R-ECO-28 (§16.2) "gián đoạn":** A resource point is Interrupted while any hostile is inside its threat radius [TUNABLE]. Interrupted villagers stop producing and stay at their spot until the threat leaves or the zone is evacuated.
- **R-ECO-29 (§14.5, §3) Assumption:** Villagers are valid Local Aggro targets for enemies, with the lowest priority after Hero, blocking Squad, taunt target and structures in the interaction rule. (Derived from §3 / §16.2 "can be attacked"; recorded under NEW-ECO-04 review.)
- **R-ECO-30 (§14.2, §25.1):** Authored raid groups may use a resource point as their special objective (§14.2 "objective đặc biệt"). After the point has no working villagers for a short time [TUNABLE], the raid continues along its authored route toward the Core. Raid frequency is a difficulty axis (§25.1), set in wave data.
- **R-ECO-31 (§27.2) [LOCKED]:** Which resource points are active in a run may vary within the authored set ("resource node active"). Inactive points are visible but not workable.
- **R-ECO-32 (§5.3) [LOCKED]:** Every Siege Site has resource points inside its gameplay boundary (WLD map contract).
- **R-ECO-33:** Resource points are not buildable cells; the build system cannot place structures on a resource point footprint.

## 5. Player Actions

| Action | Where | Rule |
|---|---|---|
| Set target workers per job (Food, Gold) | Workforce panel | R-ECO-20 |
| Reorder job priority | Workforce panel | R-ECO-21 |
| Evacuate / reopen a resource zone | Workforce panel | R-ECO-23 |
| Guard a resource zone with a squad | Command Wheel on Resource Camp zone | R-ECO-24 |
| Spend Gold (build, repair) | Build mode | R-ECO-05 |
| Spend Food (reinforce squad) | Squad card in workforce/build UI | R-ECO-03 |
| Spend MM | Conversion Building (CNV) | R-ECO-06 |

## 6. Success / Failure Conditions

- ECO has no win/lose of its own. Run win/lose stays RUN's (Core destroyed, §18.1).
- Bad economy outcome: all villagers dead or all zones evacuated → no Food/Gold income for the rest of the run; the run continues on wave rewards and MM.

## 7. Scope

### In Scope (VS)
- Three-resource ledger replacing the run resource; earn sources (kills → MM, waves → Gold, villagers → Food/Gold); sinks (Gold build/repair, Food reinforce).
- `AResourcePoint` + definitions (Food, Gold types), active-node variation.
- `AVillagerCharacter` with work / interrupted / evacuate / shelter behavior.
- Workforce allocation (targets, priority), evacuate, protect via Resource Camp zone.
- Enemy interaction: villagers in Local Aggro, raid routes to a resource point.
- Workforce panel, resource HUD, villager markers, `Feedback.Economy.*`.
- Villager count / perf benchmark.

### Out of Scope
- Named or persistent villagers (Q-08), villager traits, housing, happiness.
- Upkeep drain ("sustain", NEW-ECO-01), trade, markets, villager recruitment.
- Production chains or processed goods (§3 no crafting chains).
- Building new resource points during a run.
- Food/Gold carried across runs.

## 8. Anti-Goals

- Not a city builder or colony sim (§3): no zoning, no needs, no per-building staff micro.
- No per-villager selection or orders (§3, §16.2).
- No deep economy UI; the workforce panel fits on one screen.
- No economy that pulls the player out of combat for long: every action is a toggle or a number.

## 9. Dependencies

| Needs | From | Anchor / task |
|---|---|---|
| Run resource earn/spend to replace | RUN | T-RUN-04 |
| Run game mode / state, run definition | RUN | T-RUN-01, T-RUN-03 |
| Health, team, combat contract | FND | T-FND-05 |
| Enemy base + rewards field | ENM | T-ENM-01 |
| Route objective, Local Aggro | ENM | T-ENM-07, T-ENM-08 |
| Lane routes, structure costs, build zones, repair | DEF | T-DEF-02, T-DEF-04, T-DEF-07 |
| Wave lifecycle (wave-clear reward) and wave data (raids) | DIR | T-DIR-01, T-DIR-02 |
| Boss MM reward | BOS | T-BOS-05 |
| Squad spawning (reinforce) | SQD | T-SQD-01 |
| Resource Camp Tactical Zone + zone commands | ZON | T-ZON-01, T-ZON-02 |
| HUD shell, world markers, feedback | UXF | T-UXF-01, T-UXF-02, T-UXF-04 |
| Concurrent enemy cap benchmark | DEF | T-DEF-12 |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Target workers > alive villagers | Priority fill (R-ECO-21); panel shows "short by N" |
| Every point of a category evacuated or inactive | Category gets 0 workers; its villagers shelter; panel says why |
| Villager killed while commuting | Removed; allocation re-run; income loss shown |
| Shelter path blocked by enemies during evacuation | Villagers still path; they can die; no teleport while visible |
| Enemy inside threat radius but behind a wall / not hostile-active (dead) | Only living hostile pawns count |
| Spending Gold with exactly the cost | Allowed; resource becomes 0 |
| Kill credited after run resolve | Ignored (ledger closed at resolve) |
| Raid group objective point already empty | Raid goes straight to its continuation route |
| Hero dies (Commander Spirit) | Workforce panel still usable (no hero needed); evacuate works |
| Core critical HP | No automatic villager behavior; player decides |

## 11. Acceptance Criteria

- **AC-ECO-01:** In a VS run, HUD shows Food, Gold and MM; the old single run resource no longer exists in data, UI or code paths.
- **AC-ECO-02:** Building and repairing cost Gold; a build without enough Gold is refused with a reason.
- **AC-ECO-03:** Killing an enemy adds its archetype's MM amount; clearing a wave adds its Gold reward.
- **AC-ECO-04:** With 2 villagers working a Food point at rate R per interval, Food rises by 2R each interval; with one killed, by R.
- **AC-ECO-05:** Setting Food 6 / Gold 6 with 8 villagers and priority Gold > Food gives Gold 6, Food 2.
- **AC-ECO-06:** An enemy entering a point's threat radius stops its production within one interval and shows the interrupted state; production resumes after it leaves or dies.
- **AC-ECO-07:** Evacuating a point moves its villagers to another valid point of the same category or to the shelter; reopening reassigns them.
- **AC-ECO-08:** Guard on the linked Resource Camp zone sends the squad there through the normal Command Wheel flow.
- **AC-ECO-09:** A raid group authored on a raid route walks to the resource point, attacks villagers there, then continues toward the Core.
- **AC-ECO-10:** Reinforcing a squad costs Food per soldier and is refused without enough Food.
- **AC-ECO-11:** No input selects or moves an individual villager (input audit of all mapping contexts).
- **AC-ECO-12:** With the VS villager count plus the concurrent enemy cap, the VS Siege Site stays within the frame budget on the reference PC.

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| NEW-ECO-01 | Does "sustain the army" mean a Food upkeep drain? | No upkeep in VS | REQUIRED |
| NEW-ECO-02 | Exact meaning of "set priority" | Category order used when villagers are short | REQUIRED |
| NEW-ECO-03 | What is "protect zone"? | Squad Guard on the linked Resource Camp zone | REQUIRED |
| NEW-ECO-04 | Can villagers fight or flee on their own? Where do they rank in enemy Local Aggro? | No fighting; no auto-flee; lowest aggro priority | REQUIRED |
| NEW-ECO-05 | Villager count source, and can lost villagers be replaced mid-run? | Run/site data; no replacement | IMPROVEMENT |
| NEW-ECO-06 | Is MM picked up physically or credited on kill? | Credited on kill | IMPROVEMENT |
| NEW-ECO-07 | Hidden teleport for stuck villagers (like §13.6 for soldiers)? | Yes, only when not visible | IMPROVEMENT |
| Q-08 | Named/individual villagers or workforce NPCs? | Workforce NPCs | (master plan) |


## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Run-scoped resource ledger (Food, Gold, MM); villager roster, allocation and production; resource point states; enemy-villager interaction rules; economy UI data. |
| Inputs | Enemy deaths (archetype reward), wave lifecycle (wave-clear reward), boss reward; spend requests from DEF (build/repair), SQD reinforce, CNV conversion; workforce targets, priority and evacuate toggles from UI; hostile overlaps on resource points; villager deaths. |
| Outputs | Resource totals and change events; spend success/failure with reason; villager assignments and states; per-point state (Active, Inactive, Interrupted, Evacuated, Unstaffed); raid objectives for enemies. |
| State | `URunEconomyComponent` on `ARunGameState`: resource totals. `UWorkforceComponent` on `ARunGameMode`: roster, targets, priority, evacuated set, assignments; snapshot mirrored to `ARunGameState`. `AResourcePoint`: hostile count, workers present. `AVillagerCharacter`: FSM state, HP. |
| Events | `OnResourceChanged(Tag, NewValue, Delta, Source)`, `OnSpendFailed(Tag, Missing, Reason)`, `OnWorkforceChanged(Snapshot)`, `OnResourcePointStateChanged(Point, State)`, `OnVillagerDied(Point)` |
| Data model | `FResourceAmount` {Tag `Resource.Food/Gold/MonsterMaterial`, Amount}. Run/site data: starting resources, villager count. `UResourcePointDefinition`: resource tag, rate per worker per interval, max workers, threat radius, raid-continue delay. Enemy archetype: reward list (MM). Wave data: Gold reward, raid groups (route ending at a resource point). Structure data: Gold cost, repair cost rate. `UGameTuningSettings`: production interval. |
| Failure cases | §34.4: tower destroyed (repair costs Gold), squad wipe (reinforce costs Food; SQD owns wipe), blocked path opened (raid continuation uses lane rules), Core critical HP (no auto behavior). ECO-specific: villager death, zone overrun, all villagers dead, stuck villager, spend without funds. |
| Performance | Villagers are Characters: count toward actor budget. Decision timer 0.25–0.5 s, no Tick. Production on one timer. Threat detection by overlap events. Local Aggro candidate sets grow by villager count. |

## 14. Feedback Contract (GDD §34.5)

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Resource gained (`Feedback.Economy.ResourceGained`) | Small float text at source (kill, point) | Soft tick, throttled | Counter pulse | Never spammy: aggregate per second |
| Spend refused (`Feedback.Economy.SpendRefused`) | Counter flashes red | Deny cue | "Need 30 Gold" | Says which resource and how much |
| Point interrupted (`Feedback.Economy.ZoneInterrupted`) | Zone icon turns warning color; villagers cower | Alarm bell (once per event) | Panel row "Under threat" + edge-of-screen marker | Readable without opening the panel |
| Villager attacked (`Feedback.Economy.VillagerAttacked`) | Villager marker flashes | Short cry, throttled | — | Throttled so swarms do not spam |
| Villager died (`Feedback.Economy.VillagerDied`) | Marker fade | Low cue | Toast "Villager lost at <zone>" + count | Shows remaining villagers |
| Zone evacuated (`Feedback.Economy.ZoneEvacuated`) | Zone icon greyed; villagers run (flee anim) | Horn | Panel toggle state | Evacuated zones look clearly different from interrupted |
| Zone unstaffed / short of workers (`Feedback.Economy.WorkforceShort`) | — | — | Panel "short by N" | Shown only in panel and build mode |
| Raid targeting a zone (`Feedback.Economy.RaidIncoming`) | Lane/route marker toward zone | Warning cue | Forecast lane hint if DIR shows it | Warned before arrival when the wave is forecast |
