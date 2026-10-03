# Onboarding / Tutorial (ONB): Specification

> **Provisional — re-validate after G3.** This feature starts only after Gate G3 passes and after the systems it teaches are stable. Re-check every rule, value and task against the G3 playtest notes before work starts.

| | |
|---|---|
| GDD sections covered | §34.3 Tutorial / Onboarding |
| Cross-references | §1.3 / §10 (combine layers), §2.1 / §9 (Hero combat), §2.2 / §11 (command), §7 (forecast), §12 (Tactical Focus), §14.3–14.4 (blocking, minimum-break), §15 (Tactical Zones), §18 (Commander Spirit), §21 (towers), §28 (readability), §29.2 (input), §30 (save), §34.4 (failure cases) |
| Phase | VS only |
| Status | Provisional draft v1 (2026-10-02) |

## 1. Overview

ONB teaches the game in the fixed order of GDD §34.3, one system at a time, inside a scripted Siege Site variant built from existing systems: `ARunGameMode` with a tutorial `URunDefinition`, authored waves, and the normal Hero, Squad, Structure, Forecast and Tactical Focus code. Systems the player has not reached yet are hidden and disabled. Completion is stored in the meta profile; returning players can skip.

## 2. Player Experience

- "One new thing at a time, and I use it right away."
- Every lesson ends with the player doing the thing, not reading about it.
- The last stage shows why the game exists: Hero, Army and Tower winning together (§1.3).
- Experienced players are never forced through it again.

## 3. Core Loop (per stage)

```text
New system revealed (HUD element + input enabled)
→ short prompt with the current key binding
→ player performs the objective in a safe, authored situation
→ success feedback → next stage
```

## 4. Gameplay Rules

### 4.1 Order and pacing

- **R-ONB-01 (§34.3) [unlabeled, mandatory]:** Teaching order is fixed:
  1. Hero combat
  2. Command one squad
  3. Build one tower
  4. Read forecast
  5. Use Tactical Focus
  6. Understand blocking / path rule
  7. Combine Hero + Army + Tower
  Data validation rejects a tutorial definition whose stages are out of this order.
- **R-ONB-02 (§34.3):** Never dump all systems at once. Before its stage, a system's input is disabled and its HUD elements are hidden. Each stage opens at most one new system.
- **R-ONB-03 (§34.3):** Systems outside the §34.3 list (perks, workforce, conversion, meta unlocks) stay hidden for the whole tutorial.
- **R-ONB-04:** Each stage has an ordered list of objectives. A stage completes when all its objectives are done. Objectives are things the player does (perform an action, issue an order, place a structure, clear a wave) or an explicit acknowledge for reading steps.
- **R-ONB-05 (§29.2):** Prompts show the player's current binding for each action (rebinds respected, gamepad-ready glyph lookup).
- **R-ONB-06:** Prompts never pause the game. Reading steps happen when no enemy is active.

### 4.2 Stage content (design contract)

- **R-ONB-07 Stage 1, Hero combat (§9.1–9.7, §2.1):** Combat input only. Objectives: one light chain, one heavy attack, one dodge, one block, one parry, then defeat a small authored group of melee enemies. HUD: HP and stamina only.
- **R-ONB-08 Stage 2, Command one squad (§11.2, §11.4, §15.3):** Command Wheel enabled; one Infantry squad joins. Objectives: Follow, Guard at a Tactical Zone (Bridge), Attack a target, Retreat. Squad HUD shown.
- **R-ONB-09 Stage 3, Build one tower (§21.1, §14.6):** Build mode enabled with enough Gold for one Ballista. Objectives: place one Ballista in the build zone; an Armored enemy walks the lane and dies to Ballista + player.
- **R-ONB-10 Stage 4, Read forecast (§7.1–7.2):** Forecast panel enabled. The next wave's forecast is shown (enemy types, Low/Medium/High, lane direction). Objective: acknowledge; then the forecast wave arrives and matches the forecast; objective: clear it.
- **R-ONB-11 Stage 5, Tactical Focus (§12.2–12.4):** Tactical Focus enabled. Objectives: enter Focus; issue one squad order while in Focus. Meter visible; Focus rules unchanged (no refill while held, cancel instant).
- **R-ONB-12 Stage 6, Blocking / path rule (§14.3, §14.4):** Objectives: (a) place a Barricade on the marked corridor; enemies stop and attack it; it breaks and they continue; (b) the stage seals all routes (authored walls or the player's builds); enemies pick and break the minimum-break structure. Prompts explain both outcomes as they happen.
- **R-ONB-13 Stage 7, Combine (§1.3, §10.3, §33 06:00):** All taught systems enabled. A two-lane wave with Swarm and Armored. Prompt hints the Heavy → Armor Broken → Ballista chain. Objective: clear the wave with the Core alive.

### 4.3 Built from existing systems

- **R-ONB-14 (§34.3, brief):** The tutorial is a scripted Siege Site variant: `ARunGameMode` + `DA_Run_Tutorial` (`URunDefinition`) + authored `UWaveDefinition`s (no Director randomness) + standard gameplay actors. No tutorial-only combat, AI or pathing rules. Tutorial-specific content is data only (slower training enemy variants, stage setups).
- **R-ONB-15:** Run steps in the tutorial advance only when the current stage completes (no prep/intermission timers).
- **R-ONB-16:** The tutorial level satisfies the Siege Site map contract (WLD R-WLD-15).

### 4.4 Failure and recovery

- **R-ONB-17 (§34.4) Assumption NEW-ONB-03:** Core destroyed in the tutorial restarts the current stage from its setup. It is never a run loss. Stage setups heal the Core and restore needed structures and squads.
- **R-ONB-18 (§18, §34.4):** Hero death uses normal Commander Spirit Mode and respawn. A one-time hint explains it (R-ONB-22).
- **R-ONB-19 (§34.4):** Squad wipe or tower destroyed before the stage needs them: the stage setup restores them on stage restart. A pause-menu "Restart stage" and "Skip stage" exist as soft-lock recovery.
- **R-ONB-20 (§34.4 player leaving Siege Site):** The normal RUN boundary applies (warning + push-back).

### 4.5 Skip, completion, hints

- **R-ONB-21 Assumption NEW-ONB-01:** A profile without the completion flag is offered the tutorial at first launch, with a "Skip" choice. A profile with the flag goes to the hub. The tutorial can be skipped at any time from the pause menu (with confirm); skipping sets the completion flag. It can be replayed from the main menu.
- **R-ONB-22 Assumption NEW-ONB-02:** Systems outside §34.3 get a one-time contextual hint the first time the player meets them in normal play: perk offer, Commander Spirit, workforce panel, conversion building, hub unlock station. Seen hints are stored in the meta profile.
- **R-ONB-23 (§30, A-11):** No mid-tutorial save. Quitting restarts the tutorial from stage 1 next time (it remains skippable).
- **R-ONB-24 Assumption NEW-ONB-04:** The tutorial is its own level launched from the main menu (not a Siege Site in the world). It grants no meta reward and no campaign progress. Completing it goes to the hub.

## 5. Player Actions

| Action | Where | Rule |
|---|---|---|
| Perform the stage objective | Tutorial level | R-ONB-04, R-ONB-07..R-ONB-13 |
| Acknowledge a reading step | Prompt | R-ONB-10 |
| Restart stage / Skip stage | Pause menu | R-ONB-19 |
| Skip tutorial | First-launch dialog, pause menu | R-ONB-21 |
| Replay tutorial | Main menu | R-ONB-21 |

## 6. Success / Failure Conditions

- Success: stage 7 objective done → completion flag set → hub.
- Stage failure: Core destroyed → stage restarts (R-ONB-17).
- Abort: skip (flag set) or quit (no flag change).

## 7. Scope

### In Scope (VS)
- Tutorial definition data, stage runner, feature gates, objective watchers, prompt UI.
- Tutorial level + run definition + authored waves + training enemy variants (data).
- Seven stages per §34.3.
- Skip / completion / replay; first-exposure hints in normal play.

### Out of Scope
- Voice-over, cinematics, codex / encyclopedia.
- Teaching perks, workforce, conversion or meta inside the tutorial (hints instead, R-ONB-22).
- Difficulty selection, adaptive tutorials, per-stage save.
- Tutorial-only mechanics.

## 8. Anti-Goals

- No wall of text: one short prompt at a time.
- No system dump (§34.3).
- No special-case gameplay rules that differ from real runs (the tutorial must teach the real game).
- Never forced on returning players.

## 9. Dependencies

| Needs | From | Anchor / task |
|---|---|---|
| Hero verbs + action events | CMB | T-CMB-02, T-CMB-05..T-CMB-09 |
| Melee enemies, Armored, Swarm | ENM | T-ENM-01, T-ENM-05, T-ENM-06 |
| Commands, Command Wheel, zone context | SQD, ZON | T-SQD-04, T-SQD-05, T-ZON-02 |
| Build mode, Ballista, Barricade, route blocking, minimum-break | DEF | T-DEF-05, T-DEF-07, T-DEF-08, T-DEF-10 |
| Armor Broken → Ballista synergy | SYN | T-SYN-02, T-SYN-06 |
| Authored waves, wave events, forecast + UI | DIR | T-DIR-01, T-DIR-02, T-DIR-06, T-DIR-07 |
| Run definition, step sequence, Core destroyed | RUN | T-RUN-01, T-RUN-02, T-RUN-03 |
| Tactical Focus + commands in Focus | TFM | T-TFM-01, T-TFM-04 |
| Commander Spirit | CSM | T-CSM-01 |
| HUD shell, feedback | UXF | T-UXF-01, T-UXF-02 |
| Completion flag, seen hints, settings (bindings), main menu | MET | T-MET-04, T-MET-08, T-MET-10 |
| Travel to hub, map contract | WLD | T-WLD-03, T-WLD-10 |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Player kills the training group before doing every verb | Stage setup respawns the group until all verb objectives are done |
| Player issues commands before the prompt asks | Already-done objectives count (objectives track events from stage start) |
| Player builds the Ballista outside the marked zone | Any valid placement counts; prompt only suggests |
| Player never seals in stage 6 part (b) | The stage seals with authored walls itself |
| Hero dies during a reading step | Commander Spirit; prompt waits; respawn continues the stage |
| Player rebinds a key mid-tutorial | Prompts update on next display |
| Player opens a gated feature via a debug/cheat | Gates are UX, not security; cheats bypass them on purpose |
| Tutorial replayed by a completed profile | Runs normally; flag stays set |

## 11. Acceptance Criteria

- **AC-ONB-01:** A new profile is offered the tutorial; choosing Skip goes to the hub and sets the flag; a completed profile never sees the offer again.
- **AC-ONB-02:** Stages run in the §34.3 order; a definition with swapped stages fails data validation.
- **AC-ONB-03:** Before stage 2 the Command Wheel key does nothing and no squad HUD is visible; before stage 3 build mode cannot open; before stage 4 no forecast panel; before stage 5 Tactical Focus cannot start. Perks, workforce and conversion UI never appear in the tutorial.
- **AC-ONB-04:** Each stage completes only after all its objectives fire from the real systems (verified with Functional Tests per stage using scripted inputs or cheats).
- **AC-ONB-05:** Stage 6 shows both behaviors: enemies stop and break a route-blocking Barricade, and under a full seal they break the minimum-break structure and continue.
- **AC-ONB-06:** Core destroyed in any stage restarts that stage with Core and needed actors restored; no run-loss screen.
- **AC-ONB-07:** Every prompt shows the current binding; after rebinding Dodge in settings, the stage 1 prompt shows the new key.
- **AC-ONB-08:** Completing stage 7 sets the flag in the saved profile and travels to the hub; the tutorial grants no meta currency.
- **AC-ONB-09:** In a normal run, the first perk offer, first Hero death, first workforce panel open and first conversion building interaction each show their hint once per profile.
- **AC-ONB-10:** No gameplay rule differs from a normal run: grep/review shows no tutorial checks inside combat, AI, pathing or Director code (only gate checks at feature entry points).
- **AC-ONB-11:** New-player playtest (3–5 people who have not played): each finishes without outside help; stage completion times and confusion points are recorded.

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| NEW-ONB-01 | Who can skip, and when? | Offered at first launch with Skip; skip any time with confirm; replay from main menu | REQUIRED |
| NEW-ONB-02 | How are systems outside §34.3 introduced? | One-time contextual hints in normal play | REQUIRED |
| NEW-ONB-03 | What happens when the Core falls in the tutorial? | Restart current stage | IMPROVEMENT |
| NEW-ONB-04 | Is the tutorial its own level or a world Siege Site; does it give rewards? | Own level from main menu; no rewards | IMPROVEMENT |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Run the seven-stage tutorial in order; gate systems until taught; detect objective completion from real system events; recover from failure; record completion; show first-exposure hints in normal play. |
| Inputs | Tutorial definition; events from owners (Hero actions, squad orders, structure placed/destroyed, route events, forecast shown, Focus state, wave cleared, Core destroyed, Hero death, perk offer shown); MET flags; player acknowledge / skip / restart. |
| Outputs | Gate state on the player controller (closed gates); prompts; stage setup actions (spawn authored waves via DIR, grant squad/Gold, restore structures/Core); RUN step advance; completion flag and seen hints to MET; travel to hub. |
| State | Current stage index, objective counters (transient, tutorial level lifetime); gate container (controller); persistent flags in MET only. |
| Events | `OnStageStarted(Index)`, `OnObjectiveProgress(Index, ObjectiveIndex, Count)`, `OnStageCompleted(Index)`, `OnTutorialFinished(bSkipped)`, `OnGatesChanged` |
| Data model | `UTutorialDefinition`: ordered stages. Stage: topic (one of the 7 §34.3 topics), prompt, gate to open (≤1, `Tutorial.Gate.*`), objectives (type, parameter tag/asset, count, prompt), setup (wave to spawn, squads to grant, starting Gold, structures to ensure, Core heal), restart point. Hint list: hint ID, trigger type + parameter, prompt. |
| Failure cases | §34.4: Hero death (Commander Spirit + hint), squad wipe (restored by stage setup), tower destroyed (restored on restart), blocked path opened (taught in stage 6), Core critical / destroyed (stage restart), player leaving Siege Site (RUN boundary), soft-lock (restart/skip stage). |
| Performance | Negligible: event-bound counters, one prompt widget, small authored waves. |

## 14. Feedback Contract (GDD §34.5)

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Stage started (`Feedback.Tutorial.StageStarted`) | Newly enabled HUD element highlights | Soft sting | Prompt with binding | One prompt at a time |
| System unlocked (`Feedback.Tutorial.GateOpened`) | HUD element fades in with outline | Short cue | — | The new element is the only highlighted thing |
| Objective progress (`Feedback.Tutorial.ObjectiveProgress`) | Check tick | Tick | Counter "2 / 3" | Updates on the event frame |
| Stage complete (`Feedback.Tutorial.StageCompleted`) | Check mark burst | Success sting | Prompt clears | Pause of ~1 s before the next prompt |
| Stage restarting (`Feedback.Tutorial.StageRestart`) | Fade | Low cue | "Core fell. Restarting this step." | Says it is not a loss |
| First-exposure hint (`Feedback.Tutorial.Hint`) | Small panel | Soft cue | One-line hint + binding | Once per profile; never blocks input |
| Tutorial complete (`Feedback.Tutorial.Completed`) | Banner | Fanfare | "Tutorial complete" → hub | — |
