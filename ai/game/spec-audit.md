# Spec Audit: Completeness and Readiness Review

| | |
|---|---|
| Date | 2026-10-03 |
| Scope | All 59 docs in `ai/game/` (19 features, 271 tasks) against GDD v2 and the `game-development-workflow` / `ue5-project-architecture` skills |
| Method | (1) Script checks: IDs, dependencies, cycles, phase order, rule/AC coverage, required task fields, GDD section citations. (2) Four read-only reviews: P0–P1 features, P2–P3 features, VS + launch coverage, UE5 architecture. The high-severity findings were checked against the text before being included here. |

## 1. Verdict

**Ready to implement with fixes. Nothing blocks starting Phase F.**

- **Structure:** clean after this pass. No missing references, no dependency cycles, no phase-order violations. Every rule and acceptance criterion has a task, and every task has an objective, dependencies, steps, a test case, acceptance criteria and verification.
- **Design and architecture:** 15 high-severity issues. Each one must be fixed before the phase that needs it (section 3).
- **Biggest gap: the plan covers building systems, not shipping a game.** Playtesting with real testers, bug tracking, versioning and crash logs, the release plan, art and audio direction, content targets for 1.0 and an operating guide for coding agents have no docs yet (section 6).

## 2. Fixed in This Pass

| Fix | Files |
|---|---|
| Broke 5 dependency cycles: ENM↔SYN; SQD↔UXF; RUN↔BOS (twice); RUN→PRK→RUN. Fix: reverse the wrong edge, or turn "soft" dependencies into "Integrates with (not blocking)" lines. | 04, 08, 13 tasks |
| Aligned 8 tasks whose table dependencies differed from their detail section; moved "consumer" notes onto their own lines | 01, 08, 11, 12, 13 tasks |
| Assigned 15 rules/ACs that no task covered; recorded the deliberate exceptions ([DEFERRED] and data-hook-only rules) | 00, 01, 02, 05, 12, 13, 21, 22, 23, 24 |
| **Gate wiring:** the 4 gate tasks now depend on every QA/content task of their phase. 55 dependencies added: T-CMB-16 (G0), T-SQD-16 (G1), T-DEF-20 (G2), T-RUN-18 (G3). | 01, 03, 05, 08 tasks |
| D0/D1 demo path recomputed from the real dependency graph: **D0 = 9 tasks, D1 = 29 tasks** (was 8 / 27) | main plan §5, project_summary |
| New decision **D-19**: a single player-mode owner (a push/pop stack on `AHeroPlayerController`). Changed in T-FND-06. | main plan §7, 00-foundation |
| New decision **D-20**: clock domains. Hero action windows use the hero's own time dilation; world timers use world game time; UI uses real time. | main plan §7 |
| Foundation: Parry input left to NEW-CMB-01; armor defined as a 0–0.9 fraction; every definition derives from `UGameDefinition`, with a canonical Primary Asset Type list; packaged build must pass a debug-CVar check; key map reserved in `input-keymap.md` | 00-foundation |
| Renamed feedback tags to match the DIR, ONB and ENM docs (FC-45, FC-72, FC-73, FC-74); G1 checklist now includes "basic shared combat states" | 13 spec, main plan §3 |

## 3. High-Severity Issues (fix before the phase shown)

| # | Issue | Fix | Owner docs | Fix before |
|---|---|---|---|---|
| H1 | Hit stop slows only the hero, but the 0.2 s input buffer and the 1.0 s counter window run on world time. Each hit stop can eat up to 0.15 s of a player window. | Apply D-20: run buffer and counter-window timers in the hero's dilated time. Add hit-stop cases to `FT_Feedback_HitStop` and the CMB functional tests. | 01 tasks (T-CMB-02, T-CMB-09), 13 (T-UXF-03) | P0 |
| H2 | Player mode is stored in three places. DEF, CSM, TFM and PRK patch around a mode switch, and six features write HUD-layer flags. | Apply D-19 in consumers. Build mode, Commander Spirit, Focus and the perk modal push/pop modes. HUD layers and the tactical overlay listen to `OnPlayerModeChanged`. | 05, 09, 10, 11, 13 | P1 (the Command Wheel is the first second mode) |
| H3 | The squad target-scorer contract is defined three different ways (formula, field name, tie-break rule). Marked cannot share the Focus Target tier. | Choose one formula, field name and multi-condition tier rule in the SQD technical plan §6.3. Link it from §8a. | 03, 04 | P1 |
| H4 | The DIR stall guard (≤3 enemies alive, no removal for 90 s) destroys them. That includes the boss, and enemies breaking a sealed Barricade or the Core, so sealing a lane becomes a free wave clear. | Count damage dealt/taken or movement as "progress". Exempt the boss wave and enemies attacking structures or the Core. Add a Functional Test: sealed lane with slow stragglers. | 07 (T-DIR-02), 12, 08 | P2 |
| H5 | **Core-ring seal:** goal cells are "free cells next to the Core", so a ring of structures around the Core leaves none. Routes then return NoRoute instead of a minimum-break target. The R-DEF-10 fallback is implemented in no ENM task, and the ENM stuck-teleport can move an enemy through a seal. | Seed goal cells from every Core-adjacent cell and charge break cost on entry. Add a Core-ring case to T-DEF-05. Implement the fallback in T-ENM-07/15. Never teleport past the first obstacle; only teleport when the path query fails or the unit is off the navmesh (GDD §13.6 "nav bug"). | 05, 02, 03 (T-SQD-09) | P2 |
| H6 | `UMeleeTraceComponent` only hits Pawns. Structures and the Core use the `Structure` channel, so enemy swings at blockers never land (AC-ENM-14/15/17). | Make the trace's object types a parameter, and add `Structure` in T-ENM-09. | 01 (T-CMB-04), 02 | P2 |
| H7 | No bug log, no severity scale, no known-issues list, yet every gate requires "no blocker bugs". | Bug IDs and the Blocker/Critical/Major/Minor scale (workflow ref 11), in the playtests README (T-UXF-11). | 13, new process doc | Phase F |
| H8 | No playtest protocol with outside testers. A solo developer cannot judge "fun", "readable" or "want to replay"; G3 and AC-ONB-11 need fresh players. | Write a playtest protocol: tester pool per gate, session script, record format from workflow ref 09, consent, build delivery. | new process doc | G0 |
| H9 | No build versioning, crash logging or build notes. External playtest builds cannot be traced. | Version stamp in the build, crash logs and symbols kept per build, and a milestone note (workflow ref 12). | 00-foundation (extend T-FND-08) | First external playtest build |
| H10 | **Done 2026-10-03** (`AGENTS.md`, `CLAUDE.md`, `ai/game/progress.md`). Was: no `CLAUDE.md` / agent operating guide. Coding agents have no rules for picking tasks, running tests, updating status or following conventions. | Create the project `CLAUDE.md`: read the main plan, pick tasks in dependency order, run tests, update Status, follow D-xx and §8a, never edit the GDD. | repo root | Before T-FND-01 |
| H11 | 162 open decisions (134 NEW, 17 Q, 11 A) are spread across 19 specs, and none has a "needed by" date. | One decision register: ID, question, default, owner, needed-by phase, status. | new `decisions.md` | P0 |
| H12 | The VS "1 polished Siege Site" has no task chain, but WLD, ECO and CNV tasks load `L_SiegeSite_VS01`, `DA_Run_VS01` and `DA_Wave_VS_*`. | Add one owner chain (layout from the G2/G3 notes, then run/wave data, then polish) and make the consumers depend on it. | VS re-plan | VS |
| H13 | VS "art/audio/UI near target quality" has no tasks: no Q-13 style test, final art, biome art pass, UI skin, audio pass or music. | Turn production-plan §4 (VS order) into ART/AUDIO/UI tasks. Run the art style test right after G3. | production-plan, VS re-plan | Right after G3 |
| H14 | Commander Spirit runs on the controller for every Hero death, and the open-world GameMode also respawns the Hero, so open-world deaths have two handlers. | Enable CSM only when an `ARunGameState` exists. Record the rule in §8a. | 09, 21 | VS |
| H15 | No 1.0 content target and no content-cost data, so the VS Gate items "more content with the same approach" and "scale-up approved" have no evidence. | Add a VS task that logs hours per content unit. Write a 1.0 content roadmap with counts for every GDD §35 item. | new roadmap doc | VS Gate |

## 4. Medium Issues by Deadline

### Before P0 / G0
- **Parry:** the first success stops the montage, so a second attacker a few frames later is not parried. Keep a parry-success window on a timer, or define the rule as same-frame only. (T-CMB-09)
- **§9.8 impact sound per armor/material needs a surface value.** Add `Surface` to `FCombatHit`, filled by the trace or projectile, and to the §8a `FFeedbackContext`. (CMB, UXF)
- **AC-UXF-03** (VFX differ by hit type) is only tested with eyes closed. Add a sound-off visual identification trial. (UXF)
- **G0/G1 hypotheses have no numeric KEEP / CHANGE / DELETE bars, and "time to kill" cannot be computed.** Write the pass bars into T-CMB-16 and T-SQD-16, and log enemy spawn and kill events. (workflow ref 06)
- **Command Wheel default keys (LMB / RMB / mouse wheel) collide with combat keys** (R-CMB-36). Either allow per-mode reuse with a written priority, or move the wheel keys. Keep one key map in `00-foundation/input-keymap.md`.
- **Telemetry reads the throttled feedback stream, so gate metrics can undercount.** Log from gameplay events (`OnHeroDeath`, `OnParrySucceeded` …). (UXF T-UXF-08, TFM Focus %)
- **CMB, MET and WLD register short asset type names** (`HeroClass`, `Biome`, `MetaUnlock`). Align them with the canonical list in foundation §9.
- **Localization rule:** all player-facing text uses `FText` / String Tables, and the font covers Vietnamese diacritics. Cheap to adopt now, expensive later.
- Low: the light chain is hard-coded to 3 hits (it is [TUNABLE]). G0 has no readability hypothesis; add "telegraphs and stagger are readable".

### Before P1 / G1
- **Pick one avoidance model for all ground AI** (soldiers use DetourCrowd, enemies use RVO, and the two cannot see each other). Test it at G1, where soldiers and enemies first fight. (SQD, ENM)
- **The squad leash table has no MoveToOrder row**, so a squad attacked while moving flips between Engage and Reform. (SQD)
- **Hide Follow while the Hero is dead.** Add the rule to T-SQD-06; §8a points at T-SQD-05, which does not do it.
- Low: AC-SQD-05 (Follow leash vs Hero sprint) has no catch-up speed.

### Before P2 / G2
- **The economy changes owner every phase:** DEF casts to `ARunGameMode`, kill rewards sit in `URunDefinition`, and at VS ECO moves everything to the GameState. Build one tag-keyed ledger on `ARunGameState` in T-RUN-09 now, so ECO only adds resource types.
- **Core reference:** consumers read the Core from `ARunGameState`, but RUN keeps it on the GameMode. Add `GetCore()` and `OnCoreChanged` to the GameState.
- **Lane danger has two sources** (DIR's per-lane view and UXF's `ULaneDangerSubsystem` scanning enemies). Derive it from DIR's view on `ARunGameState` and delete the extra subsystem.
- **The enemy-cap benchmark uses 6 towers and a fixed enemy mix.** Benchmark with the maximum structures, the heaviest P3 mix and the Focus overlay on, or weight the cap per archetype.
- Low: R-ZON-12 says bonuses ≤20%, but Rally Point is +30%. Route invalidation by version polling vs per-enemy delegate. SQD "Retreat reaches Core" vs a Core-ring seal.

### Before P3 / G3
- **Repair phase:** NEW-DEF-04 defers repair to VS, but A-05 and GDD §6.3 [LOCKED] ("sửa defense") expect it in the run loop. **Decision needed.**
- **The §33 10:00 conversion decision has no P3 substitute.** P3 has no cross-layer spending trade-off; build is the only spend. **Decision needed:** add a P3 substitute (e.g. paid squad refill vs build), or mark the beat VS-only in the pacing scripts.
- **Boss Phase 2 assumes side lanes,** but `L_SiegeSite_Proto` and §33 have 2 lanes. Define the phase for 2 lanes and test it on the real map.
- **"Build assembled" (perks) is undefined.** A simulation shows the G3 perk check can fail on RNG alone about half the time. Define a pass bar (≥2 perks of one build plus a described change in play) or seed one run.
- **The perk choice widget owns gameplay** (cancels Focus, sets input mode), and its 1/2/3 keys bypass Enhanced Input, so they cannot be remapped or used on a gamepad. Use a mode push (D-19) and Input Actions.
- **Boss snapshot/respawn has no owning class.** Put it in the Director's boss mode and add a Boss row to the D-07 table.
- **Mid-run save path (D-14):** the run state is spread over 5+ owners with two random seeds. Use one `RunSeed` in `FRunStateData` and an `FRunSnapshot` that collects each owner's `ToRunState()`.
- **Swarm icons in the Focus overlay** would add per-enemy cost after the P2 cap is set. Use aggregated lane or cluster icons (NEW-TFM-6).
- Low: the DIR "Hard" difficulty asset in P3 is scope creep (difficulty is launch scope). BOS 2/3 Layer Test thresholds. T-RUN-17 ±2 min vs W5 at 18:00. BOS summon "lane caps" claim. T-TFM-06 should depend on T-DEF-16.

### Before the VS re-plan
- Run-start squad roster (squads are level-placed today; MET and CNV assume a roster).
- MET unlock catalog conflicts:
  - locking a recipe vs CNV's 3 options;
  - the 4th tower and Spearman are low-priority tasks;
  - "Core HP / starting Gold" upgrades match the §24.1 anti-examples.
- CNV reinvents stat tags and a remove API that PRK and DEF already provide.
- Repair should use the Gold ledger.
- Decide whether reinforcement costs Food.
- Raid targets must appear in the forecast (§7.1 [LOCKED]).
- In-run "upgrade" (§6.3 Prepare, §16.1) has no owner. Log it as a Q.
- The boss domain has no level.
- Define "perk family" in PRK.
- Warlord rally/charge are [LOCKED] in §19.1, so T-CMB-18 must not DELETE them without a GDD revision.
- Front end: pause menu owner, whether pausing stops the simulation, the mid-run quit rule, video settings.
- The CommonUI/gamepad decision blocks the menu tasks.
- Accessibility baseline (shake scale, Focus toggle, subtitles for barks, text size).
- Audio and music direction (about 30 VS cues have no plan).
- Site → run definition should be a soft reference.
- Second-class path: `HeroPawnClass`, `CounterTags` and `HeroClassId` in run setup.
- Final-art performance test at the enemy cap.
- Low: POI ID keys collide across levels; ONB is missing its T-ECO-01 dependency.

## 5. Systemic Improvements (rules to adopt)

| # | Rule | Where to record |
|---|---|---|
| S1 | Every §8a contract lists the exact signature, the provider task, the consumer tasks, and a "consumer verified" column. Several contracts drifted on names (`StateScoreWeights` vs `StatePreferenceWeights`, `RequestOffer` vs `GenerateOffer`, a nonexistent `DIR SpawnWave`). | main plan §8a |
| S2 | One shared definition of "progress" and one recovery order for stuck/stall/respawn. Today SQD, ENM, ECO, DIR and BOS each have their own ladder, and they interfere with each other. Extract one owner-agnostic stuck helper. | foundation §7 + ENM/SQD/DIR |
| S3 | Clock domains (D-20) | done in main plan; align CMB/UXF/SYN |
| S4 | One player-mode owner (D-19) | done in FND; align consumers |
| S5 | Metrics come from gameplay events, never from the presentation/feedback stream | foundation §10, UXF |
| S6 | One key map, owned by FND (`input-keymap.md`) | foundation |
| S7 | Refresh the D-07 state ownership table: Focus meter, stat modifiers, lane danger, boss snapshot, VS ledger, workforce, conversions, tutorial state. Fix the writer rule (CSM writes revive charges). | main plan §7 |
| S8 | Gate evidence must come from `L_SiegeSite_Proto` at maximum allowed scale, not only from small test maps | gate tasks |
| S9 | VS hooks into prototype features go into §8a with owner review. Prototype assumptions carried into VS (level-placed squads, CSM on the controller, a single run resource) each get a migration task. | VS re-plan |

## 6. Missing Lifecycle Pieces (to reach a shipped game)

Checked against `game-development-workflow` references 01–14 and GDD §26.3 ("launch must feel like a complete product").

| # | Missing piece | Why it matters | Plan by |
|---|---|---|---|
| L1 | ~~Project `CLAUDE.md` (agent operating guide)~~ **Done 2026-10-03:** `AGENTS.md` (all agents) + `CLAUDE.md` (imports it) | AI agents are the team; they need the rules in every session | Before T-FND-01 |
| L2 | Decision register (`decisions.md`) | 162 open decisions; defaults must be visible and dated | P0 |
| L3 | Progress tracking and velocity log (task Status updates, or Jira since Atlassian is connected). **Partly done:** `ai/game/progress.md` session log; velocity not tracked yet. | GDD §32: no deadlines until velocity is measured, which requires recording it | P0 |
| L4 | Bug tracking and severity policy, known-issues list | Every gate requires "no blocker bugs" | Phase F |
| L5 | Playtest protocol and tester pool (fresh testers per gate, script, consent, record format) | A solo developer cannot validate fun or readability alone | G0 |
| L6 | Prototype hypothesis cards with numeric KEEP / CHANGE / DELETE bars | Workflow ref 06; gates are opinions without them | Each gate, G0 first |
| L7 | Build versioning, crash logs and symbols, milestone build notes | Workflow ref 12; external builds must be traceable | First external build |
| L8 | Performance budget doc: target frame time on the reference PC, per-system budgets, worst cases, a 25-minute soak test | Workflow ref 10; only the enemy cap is planned | T-FND-08, before the P2 benchmark |
| L9 | Prototype front-end shell: pause, restart run, quit, sensitivity, volume | Testers need it from the first external build | G0 (minimal), VS (full menus) |
| L10 | Localization policy (languages, FText, fonts) | Retrofitting text is expensive | Now (foundation convention) |
| L11 | Accessibility baseline | GDD relies heavily on color and audio cues (§28) | VS re-plan |
| L12 | Art direction (Q-13 style test) and audio/music direction | VS needs near-final quality | Right after G3 |
| L13 | VS content tasks: Siege Site chain, art/audio/UI passes, final-art performance test | VS proves the slice, not just the systems | VS re-plan |
| L14 | 1.0 content roadmap with counts and a content-cost log | Needed for the scale-up decision | VS Gate |
| L15 | Launch feature specs: Ranger and class selection, campaign completion/ending, difficulty selection and Ascension, unlock systems (weapon archetype, alternative tower mode, starting loadout) | GDD §35 launch scope has one-line mentions only | VS Gate |
| L16 | CI or a nightly build and test run | Regression safety as the test count grows | VS (earlier if tests get slow) |
| L17 | Release plan: Shipping-config tests, store (Steam), Early Access vs 1.0 (§34.6 vs §26.3), save migration policy, licenses and credits, min-spec | Nothing about shipping is planned | VS Gate |

## 7. Recommended Order

1. **Before T-FND-01:** create `CLAUDE.md` (L1), `decisions.md` (L2) and the bug/severity policy (L4). Answer Q-15.
2. **During Phase F:** add the localization rule (L10), versioning and crash logs (L7) and the performance budget (L8) to the foundation tasks.
3. **Before P0 coding:** fix H1, the "Before P0" mediums, the playtest protocol (L5) and the G0 hypothesis cards (L6). Decide NEW-CMB-01 (parry).
4. **Before P1:** fix H2 and H3, pick the avoidance model, decide NEW-SYN-02 (Armor Broken baseline vs perk).
5. **Before P2:** fix H4, H5 and H6, the economy ledger, the Core reference and the lane-danger source. Decide NEW-DEF-02 (maze vs break-through).
6. **Before P3:** decide on repair and the §33 10:00 substitute, fix the boss 2-lane model and the perk pass bar.
7. **After G3:** VS re-plan with H12–H15 and L11–L17.

## 8. Decisions Needed from the Project Owner

| Decision | Default if unanswered | Needed by |
|---|---|---|
| Q-15: project name, UE version, reference PC | — | Phase F |
| NEW-CMB-01: what a parry does; separate input or timed block | Own input; parry deals poise damage (low-poise enemies stagger) and always opens a counter window | P0 |
| NEW-SYN-02: Armor Broken is a baseline Warlord Heavy effect (§10.2/§19.1) or a perk (§33) | Baseline; a perk may extend it | P1 |
| Whether to share keys between combat and the Command Wheel (per-mode reuse) | Proposed: reuse allowed while the wheel is open (D-19 mode stack makes it safe) | P1 |
| NEW-DEF-02: maze vs break-through | Enemies walk any open path inside their lane corridor | P2 |
| Repair in P3 or VS (NEW-DEF-04 vs A-05 vs §6.3) | Move a minimal repair into P3 | P3 |
| §33 10:00 conversion beat in P3 | Paid squad refill vs build as the P3 trade-off | P3 |
| Tester pool: who plays G0–G3 builds | Developer + 3–5 outside testers per gate | G0 |
