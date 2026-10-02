# Hub and Controlled Open World (WLD): Specification

> **Provisional — re-validate after G3.** This feature starts only after Gate G3 passes. Re-check every rule, value and task against the G3 playtest notes and the WLD spike result before work starts.

| | |
|---|---|
| GDD sections covered | §4.1 three layers, §5.1–5.5 Controlled Open World, Siege Site, Valheim exclusions, campaign progression, §27.1–27.3 procedural limits |
| Cross-references | §3 anti-goals (no free sandbox), §26.1 (replay Siege Site with other modifier/reward), §31 performance, §32 VS scope ("1 biome slice, hub loop tối thiểu, controlled open world segment nhỏ"), §33 (00:00 enter Siege Site, 25:00 return to hub), §34.4 (player leaving Siege Site), §37 / Q-09, D-17 |
| Phase | VS only |
| Status | Provisional draft v1 (2026-10-02) |

## 1. Overview

WLD builds the two outer layers of the game around the Siege Run: a minimal hub (Final Citadel) and a small handcrafted segment of one biome. The hero walks the segment, discovers locations, finds the Siege Site entry and starts a run there. Run results feed campaign progress: liberating Siege Sites opens the boss domain and, later, the next biome. A lost run never resets the world. The open world is controlled: authored terrain, authored routes, authored encounters. Whether biome/site transitions are seamless or use loading gates is decided by a spike (Q-09, D-17).

## 2. Player Experience

- "I am living in a campaign", not picking levels from a menu (§5.1).
- A journey through a biome with rising danger, meaningful landmarks and rewards for looking around (§5.1).
- Clear goals: which Siege Sites remain, what opens the boss domain (§5.5).
- Losing a run sends the player home, not back to zero (§5.5).

## 3. Core Loop

```text
Hub: review progress → spend meta (MET) → leave
→ Open world: travel route → discover landmark / resource point / blueprint source → find Siege Site
→ Enter Siege Site → run (~25 min)
→ Resolve → back to hub with progress applied
→ Liberated enough sites → boss domain opens → next biome link
```

## 4. Gameplay Rules

### 4.1 Layers and world shape

- **R-WLD-01 (§4.1) [LOCKED]:** The game has three layers: Final Citadel / Hub (permanent progression), Controlled Open World (exploration, biome progression, choosing campaign targets), Siege Run (~25 min defense/action-strategy).
- **R-WLD-02 (§5.1) [LOCKED]:** The hero moves through the world on foot in third person. The world is not a node menu.
- **R-WLD-03 (§5.2) [LOCKED]:** Each biome contains: handcrafted macro terrain, controlled routes (valley / pass), a main landmark, resource points, outposts, Siege Sites, a boss domain, a connection to the next biome.
- **R-WLD-04 (§32 VS) [LOCKED via §32]:** VS builds one small segment of one biome with at least one of each R-WLD-03 element. The next-biome connection exists but stays locked (no second biome in VS).
- **R-WLD-05 (§5.2) [LOCKED]:** The player may leave the main route to explore, but level design keeps control of performance, encounter density, navigation, line of sight, combat readability and streaming. The segment has authored playable bounds made from terrain and set dressing.
- **R-WLD-06 (§5.1) [LOCKED]:** Danger rises along the route toward the boss domain. Encounter difficulty is authored per area, not scaled at runtime.
- **R-WLD-07 (§5.1) [LOCKED] + Assumption NEW-WLD-01:** Exploration has rewards. Discovering a blueprint source grants its unlock (MET Grant). Discovering a landmark or open-world resource point grants a one-time meta currency amount (default until NEW-WLD-01 is answered).
- **R-WLD-08 Assumption NEW-WLD-02:** Outposts are discoverable locations in VS with no extra mechanic until their function is decided.
- **R-WLD-09 Assumption NEW-WLD-05:** In VS the open world is hero-only. Squads, towers, villagers and the command layer exist only inside Siege Sites.
- **R-WLD-10 (§5.2, §27.2):** Open-world encounters are authored enemy groups at authored positions using existing enemy archetypes with Local Aggro and leash (no lanes). Optional encounters may be toggled on/off per visit (§27.2 "optional encounter").

### 4.2 Hub

- **R-WLD-11 (§4.1, §5.5) [LOCKED]:** The hub (Final Citadel) is the main base and the place of permanent progression.
- **R-WLD-12 (§32 VS) Assumption NEW-WLD-04:** The VS minimal hub loop is: arrive → see last run summary and campaign progress → spend at the unlock station (MET UI) → leave through the world gate. No hub building, NPC economy or hub combat in VS.
- **R-WLD-13 (§5.5, §33 25:00) [LOCKED for lose] + Assumption NEW-WLD-07:** After resolve the player returns to the hub. A lost run returns to the hub (§33). A won run also returns to the hub in VS (default until NEW-WLD-07 is answered).

### 4.3 Siege Site

- **R-WLD-14 (§5.3) [LOCKED]:** Runs take place at Siege Sites (or Defense Sites) inside the open world.
- **R-WLD-15 (§5.3) [LOCKED]:** A Siege Site has a clear gameplay boundary and contains: a Core / base to protect, enemy spawn lanes, tactical zones, building zones and placement rules, resource points, a boss entry. A Siege Site map missing any of these fails validation.
- **R-WLD-16 (§5.3) [LOCKED]:** Presentation may stream or be seamless, but gameplay inside the boundary is a battlefield with clear borders and rules. Open-world systems (exploration encounters, discovery) are inactive inside a running Siege Site.
- **R-WLD-17 (§5.3, §34.4, Q-16):** Leaving the boundary mid-run is handled by RUN (T-RUN-07: warning + push-back, no run fail). The only ways out of a running Siege Site are resolve (win/lose) or Abandon (MET R-MET-13).
- **R-WLD-18 (§5.3):** Entering a Siege Site is an explicit player choice at the site entry (interact + confirm), showing the site name, state and whether it is required for liberation.

### 4.4 Campaign progression (§5.5)

- **R-WLD-19 (§5.5) [LOCKED]:** Exploring the biome reveals Siege Sites, resource points, blueprint sources and the boss domain. Each starts undiscovered and becomes discovered on first approach or interaction.
- **R-WLD-20 (§5.5) [LOCKED]:** Completing an important Siege Site liberates its region. A site is "important" when its definition marks it required for biome liberation.
- **R-WLD-21 (§5.5) Assumption NEW-WLD-03:** In VS, liberation changes the site state to Liberated (shown on the campaign board and at the site entry). No further world change in VS.
- **R-WLD-22 (§5.5) [LOCKED]:** When enough required objectives of a biome are liberated, the path to the boss domain opens. After the boss domain is cleared, the path to the next biome opens. "Enough" is a per-biome count in data (default: all required sites).
- **R-WLD-23 (§5.5) [LOCKED]:** A failed run does not reset the world. Discoveries, liberations, unlocks and open paths stay. The player returns to the hub and can retry.
- **R-WLD-24 (§26.1):** Liberated sites can be replayed. A replay may use different modifiers / reward, set in the site definition (optional data in VS; default = same run, no first-clear bonus).
- **R-WLD-25 (MET R-MET-19):** Campaign state is derived from stored facts (discovered, liberated, cleared) every time it is needed, so a rules or data change never leaves stale state.
- **R-WLD-26 Assumption NEW-WLD-06:** Hero death in the open world respawns the hero at the segment entrance after a short delay. Nothing is lost.

### 4.5 What we do not take from Valheim (§5.4)

- **R-WLD-27 (§5.4) [LOCKED]:** No hunger/thirst survival loop, no voxel or destructible terrain across the world, no complex free-form house construction, no deep crafting progression, no sailing/ocean system, no procedural world generation.

### 4.6 Procedural limits (§27)

- **R-WLD-28 (§27.1) [LOCKED]:** Battlefield and overworld macro layout are handcrafted or semi-handcrafted. Never random: terrain, boss arena, lane topology, camera-critical choke points.
- **R-WLD-29 (§27.2) [LOCKED]:** Allowed random variation only: wave composition, active resource nodes, reward, weather, modifier, optional encounter, selected building slot/ruin, enemy spawn variation within authored bounds. VS uses at most: optional encounter toggles (WLD), active resource nodes (ECO), wave composition (DIR).
- **R-WLD-30 (§27.3):** Any variation must keep pathfinding reliability, boss design, performance budget and art consistency. A variation that breaks a Siege Site map contract (R-WLD-15) is a bug.

### 4.7 Transition technique (Q-09, D-17)

- **R-WLD-31 (§5.2, §5.3, Q-09, D-17) [OPEN]:** Seamless vs streaming gate between hub, open world and Siege Site, and Level Streaming vs World Partition for the segment, are decided by spike T-WLD-01. Until then the default is: hub, open world segment and each Siege Site are separate levels joined by loading transitions (streaming gates).

## 5. Player Actions

| Action | Layer | Rule |
|---|---|---|
| Walk, sprint, fight (Hero verbs) | Hub, open world | R-WLD-02 |
| Interact with unlock station / campaign board / world gate | Hub | R-WLD-12 |
| Discover a location (approach / interact) | Open world | R-WLD-19 |
| Fight or avoid an authored encounter | Open world | R-WLD-10 |
| Enter a Siege Site (interact + confirm) | Open world | R-WLD-18 |
| Replay a liberated site | Open world | R-WLD-24 |

## 6. Success / Failure Conditions

- Campaign success: all required sites liberated → boss domain opens → boss domain cleared → next-biome link opens (locked placeholder in VS).
- Run failure: back to hub, world unchanged except facts already earned (R-WLD-23).
- Hero death in open world: respawn, no loss (R-WLD-26).

## 7. Scope

### In Scope (VS)
- WLD spike (Q-09, D-17).
- `UBiomeDefinition`, `USiegeSiteDefinition`, campaign rules, facts stored in MET.
- Hub level (Final Citadel minimal): unlock station, campaign board, world gate, last-run summary.
- One open world segment: macro terrain, routes, 1 landmark, ≥1 resource point, ≥1 outpost, ≥1 blueprint source, the VS Siege Site entry, boss domain link, locked next-biome gate, 2–4 authored encounters.
- Travel flow hub ↔ world ↔ Siege Site; return after resolve.
- Siege Site map contract validation.
- Streaming / traversal performance pass.

### Out of Scope
- Second biome, sailing, survival needs, terrain deformation, house building (§5.4).
- Squads or towers in the open world (NEW-WLD-05).
- Fast travel, world map UI beyond the hub campaign board, day/night cycle, weather systems (weather is an allowed variation in §27.2 but no VS work).
- Procedural terrain or layouts (§27.1).
- Hub upgrades / citadel building.

## 8. Anti-Goals

- Not a sandbox open world (§3): no free building, no survival meters.
- Not a level-select menu dressed as a world: the hero walks.
- No open-world content expansion until G3 passes (§32 P3 gate).
- No World Partition just because it is modern (UE world rule); the spike decides.

## 9. Dependencies

| Needs | From | Anchor / task |
|---|---|---|
| Hero character, Interact verb | CMB | T-CMB-01, T-CMB-12 |
| Enemies with Local Aggro + leash | ENM | T-ENM-01, T-ENM-08 |
| Run game mode, run definition, resolve | RUN | T-RUN-01, T-RUN-06 |
| Siege Site boundary handling | RUN | T-RUN-07 |
| Siege Site map elements | DEF, ZON, DIR, ECO | T-DEF-03, T-DEF-04, T-DEF-07, T-ZON-01, T-DIR-01, T-ECO-03 |
| Profile, campaign facts, unlock grants, reward | MET | T-MET-02, T-MET-04, T-MET-05, T-MET-07, T-MET-09 |
| Reference PC + profiling | FND | T-FND-08 |
| Feedback rows | UXF | T-UXF-01 |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Player enters a Siege Site that became Liberated in another way (data change) | State derived from facts; entry shows Liberated, replay allowed |
| Boss domain open condition met, then data changes the required count | Recomputed on load; an already cleared boss domain stays cleared (fact) |
| Hero dies in the open world near an encounter | Respawn at entrance; encounter resets to its authored state |
| Travel target level missing / fails to load | Return to hub with an error notice; no fact changes |
| Save fails right after a discovery | Fact stays in memory; MET retries at next safe point |
| Player walks to segment edge | Terrain/props block; no invisible-wall death |
| Optional encounter toggle leaves a required path blocked | Not allowed: optional encounters are never placed on required routes (authoring rule, checked in review) |
| Replay of a liberated site is lost | No state change; partial meta reward per MET |

## 11. Acceptance Criteria

- **AC-WLD-01:** The spike report (T-WLD-01) records a KEEP / CHANGE / DELETE decision with measured load times and memory for at least two options, and Q-09 / D-17 are answered in writing.
- **AC-WLD-02:** A new profile starts in the hub, walks to the world gate, travels to the open world, walks to the Siege Site entry and starts the VS run without using debug commands.
- **AC-WLD-03:** The open world segment contains every R-WLD-03 element; each discoverable one shows a discovery toast the first time and never again (also after restart).
- **AC-WLD-04:** Winning the required Siege Site marks it Liberated; the boss domain link opens when the biome's required count is reached; the next-biome gate stays locked with a readable message.
- **AC-WLD-05:** Losing a run returns to the hub; all prior discoveries, liberations and open links are unchanged (save file comparison of the campaign section).
- **AC-WLD-06:** Replaying a liberated site starts a run and grants no first-clear bonus.
- **AC-WLD-07:** The Siege Site map contract test fails when any required element (Core, lane + spawn, tactical zone, build zone, resource point, boss entry, boundary) is removed from a copy of the VS map.
- **AC-WLD-08:** Hero death in the open world respawns the hero at the entrance within the configured delay; no progress lost.
- **AC-WLD-09:** Traversal of the full segment and the transition into the Siege Site stay within the frame budget and load-time budget recorded by T-WLD-11 on the reference PC.
- **AC-WLD-10:** Content review finds no item from the §5.4 exclusion list and no random element outside §27.2.

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| NEW-WLD-01 | What do landmark and open-world resource point discoveries give? | One-time meta currency; blueprint sources grant their unlock | REQUIRED |
| NEW-WLD-02 | What is an outpost's function? | Discoverable location only | REQUIRED |
| NEW-WLD-03 | What visible change does region liberation cause in the world? | State change on board and site entry only | IMPROVEMENT |
| NEW-WLD-04 | What belongs in the minimal hub loop? | Summary + unlock station + campaign board + world gate | REQUIRED |
| NEW-WLD-05 | Do squads travel with the hero in the open world? | No, hero only | REQUIRED |
| NEW-WLD-06 | Where does the hero respawn after dying in the open world? | Segment entrance, no loss | IMPROVEMENT |
| NEW-WLD-07 | After a won run, return to hub or to the site entry in the world? | Hub | IMPROVEMENT |
| Q-09 | Seamless or streaming gates? | Decided by T-WLD-01; streaming gate until then | (master plan) |
| Q-16 | Leaving the Siege Site mid-run | Soft boundary, RUN T-RUN-07 | (master plan) |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Hub and open-world levels, exploration encounters, discovery, Siege Site entry and return, campaign rules (site / biome / boss domain states), travel between layers, Siege Site map contract. |
| Inputs | Campaign facts and unlock state from MET; hero position/interaction; run result (via MET `ApplyRunResult` → WLD `ApplyRunOutcome`); biome and site definitions. |
| Outputs | Travel requests (target level + site ID); new campaign facts to MET (discovered, liberated, cleared); derived states for world actors and UI; unlock grants for blueprint sources. |
| State | Persistent facts in MET `FCampaignProgress`. Transient: current layer, pending travel, encounter alive state per visit. |
| Events | `OnPoiDiscovered(PoiId)`, `OnSiteStateChanged(SiteId, State)`, `OnBiomeGateChanged(BiomeId, bOpen)`, `OnTravelStarted/Finished` |
| Data model | `UBiomeDefinition`: biome ID, site list, required liberation count, boss domain site, next biome (optional), danger notes. `USiegeSiteDefinition`: site ID, Siege Site level (soft), `URunDefinition`, biome, bRequiredForLiberation, bIsBossDomain, reward table (per wave, boss, first clear, unlock grants), optional replay modifier/reward. POI actor data: authored `PoiId` (`FName`), type (Landmark / ResourcePoint / Outpost / BlueprintSource / SiegeSiteEntry), reward or unlock ID. |
| Failure cases | §34.4 "player leaving Siege Site" (RUN boundary + Abandon only exit); hero death in open world; failed run; travel failure; save failure after discovery; data change vs stored state. |
| Performance | Streaming hitches on traversal and site transition; open-world draw distance and foliage; encounter count per area; navmesh size for the segment. Budgets measured in T-WLD-11. |

## 14. Feedback Contract (GDD §34.5)

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Location discovered (`Feedback.World.Discovered`) | Landmark highlight, short camera-free flourish | Discovery sting | Toast with name + reward | One toast per discovery, never repeated |
| Entering a dangerous area (`Feedback.World.DangerArea`) | Ambient change, enemy silhouettes visible from distance | Tension layer | Area name + danger tier | Player sees danger before aggro range |
| Siege Site entry in range (`Feedback.World.SiteInRange`) | Banner / beacon at entry | Soft cue | Prompt: site name, state, required for liberation | State readable without opening a menu |
| Site liberated (`Feedback.World.SiteLiberated`) | Banner color change at entry and on board | Fanfare | Board entry updated | Shown on next hub visit |
| Boss domain opened (`Feedback.World.BossDomainOpened`) | Gate opens / barrier fades | Heavy cue | Toast + board marker | Says where it is |
| Locked gate (`Feedback.World.GateLocked`) | Closed gate | Deny cue | "Requires: liberate N sites" / "Not in this build" | Always shows the unlock condition |
| Travel / loading (`Feedback.World.Travel`) | Loading screen with destination | None | Destination name + tip | No black screen without text |
| Hero respawn in world (`Feedback.World.HeroRespawn`) | Fade | Short cue | "Returned to <entrance>" | Nothing lost is stated |
