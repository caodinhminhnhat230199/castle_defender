# Foundation Readiness Review — 2026-10-04

Reviewer: Codex. Branch: `task/T-FND-foundation`. Reviewed base commit: `6c593a0`.

**Final verdict after corrections: Phase F passed on 2026-10-04 and P0 is open. All project-side defects found by this review are repaired, all AC-FND-01…09 pass, and the owner confirmed the current i5-14600KF / RTX 5060 / 16 GB machine as the reference PC used for the rendered package and profiling evidence.**

## Correction results (Codex, same date)

The findings and acceptance matrix below preserve the original audit evidence. Current resolution:

| Finding | Current status |
|---|---|
| FND-R01 | Fixed: the editor Python script wires and compiles BP_FT_Smoke. Actual functional test passes. Rerunning asset creation preserves the graph without warnings. |
| FND-R02 | Fixed: lethal state is committed before damage observers; only the hit that performs the transition broadcasts death. Both nested-hit regressions fail before the fix and pass afterward. Actual PIE reports one damage/death callback and committed death during the observer. |
| FND-R03 | Fixed: reject nonzero editor exit, missing/malformed/incomplete reports, NotRun/InProcess/non-success states, inconsistent counters and missing default spec/functional groups. 18 evaluator regression checks pass. |
| FND-R04 | Fixed: existing TestGameDefinition is registered editor-only/NeverCook; DA_FoundationSmoke is discovered, loaded and validated by a real Asset Manager integration spec. The settings metadata spec confirms category `Game` and section `Game Tuning`. Invalid required-field validation passes. No P0 gameplay class or dependency was added. |
| FND-R05 | Resolved: real F5 input switches `Combat -> Build -> Combat`, Build owns only `IMC_Build`, scripted PIE runs the debug CVar and spawn cheat, and the rendered package shows the green sphere, health label and stat unit. A late-enabled CPU workflow produces a clean 30-second CPU/GPU/memory trace on the confirmed reference PC. |
| FND-R06 | Fixed: corrected the historical handoff recipe and automated the graph. ApplyDebugHit's void result is never branched on. |
| FND-R07 | Fixed: overview/current next steps and baseline now distinguish implemented Foundation from later proposals. |
| FND-R08 | Fixed: build/package/asset .bat launchers match the existing test launcher's child-process policy. Documented commands work without persistent execution-policy changes. |

Verification after final source/config/asset edits:

- Editor Development, game Development (during packaging), Shipping and Win64 Development packaging succeed. Complete default CLI gate after the final edits: **15 succeeded, 0 succeededWithWarnings, 0 failed/NotRun/InProcess; editor and runner exit 0**. T-FND-10 is Done.
- Clean scripted PIE exits 0 with no warning/error entries; dummy starts at 100 HP, receives the reentrant lethal hit and fires death once. Build/Modal restoration returns to Combat. T-FND-05 remains Done with corrected verification.
- Headless packaged smoke and ordinary GPU-rendered package smoke both exit 0. Test content remains excluded from cooking. Packaging retains the engine's MSVC preference warning; no new project-source compiler warning was found.
- Local screenshot: `Saved/Packaged/Windows/CastleDefender/Saved/Screenshots/Windows/ScreenShot00000.png`, viewed by Codex. This is a startup frame, not a steady-state performance measurement.
- The original command line with `cpu` enabled from process startup reproducibly exits **777003** after the log closes. Channel isolation shows that CPU-at-startup is the trigger; the pinned engine names 777003 `CrashReporterCrashed`. Memory-only and GPU-only rendered runs exit 0. No project call stack identifies the exact engine-internal fault site.
- **Resolved profiling workflow:** start the rendered trace with GPU/frame/memory/bookmark/log, then enable CPU after startup. `Saved/foundation-late-cpu-trace-30s.utrace` is 1,407,815,480 bytes; the game exits 0. Pinned Insights exits 0 and completes memory analysis, CPU analysis (28 threads) and GPU analysis (3 queues). The checklist now records this order. No renderer, driver, engine or crash-reporting setting changed.

The full-memory analyzer reports the same allocation-reconciliation diagnostics in both the original readable trace and the clean-exit trace, then completes its provider. These diagnostics are recorded as an engine tooling limitation and are not attributed to project gameplay code.

The owner selected the current i5-14600KF / RTX 5060 / 16 GB machine as the reference PC. AC-FND-09 passes on the recorded evidence, T-FND-08 is Done, and the Phase F gate is passed. Commit and push were authorized in the same session.

The graph script uses the pinned engine's supported [BlueprintGraphEditor Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/BlueprintGraphEditor?application_version=5.8), checked against installed 5.8.3 headers/source.

The architecture is consistent with the intended prototype: one runtime module, domain folders, native tags, small combat components, data definitions and one player-mode owner. Builds, tests, PIE, packaging, debug input/draw and trace analysis work. No Foundation acceptance item remains open.

## Scope and method

- Read root `AGENTS.md`, `CLAUDE.md`, the session handoff, master plan (including D-01…D-20 and §8a), production plan, spec audit and project overview.
- Surveyed all 19 features through their scope, ownership, system contracts and Foundation dependencies. Later-phase implementations remain proposals; this is not a full gameplay-design re-audit of every later-phase task.
- Reviewed all 33 files under `Source/`, all build/test/asset scripts under `Tools/`, project configuration and all Foundation rules, tasks and acceptance criteria.
- Loaded all 21 original Unreal assets through the pinned editor and read controller mode configuration and combat mappings from disk. The original review did not edit binary assets; the correction pass later modified BP_FT_Smoke and created DA_FoundationSmoke through the editor.
- Applied `code-review-and-quality` and the repo's `game-development-workflow` / `ue5-project-architecture` guides, including QA, quality gates and data architecture references.
- `.codegraph/` is absent; no CodeGraph MCP or CLI was available. Inspected source directly without installing tools or generating an index.
- Normal Git/LFS status confirmed a clean checkout before documentation edits. Disabling LFS filters temporarily for read-only inspection produced apparent binary changes; normal filtering confirmed those were hydrated files, not user changes.

## Findings

| ID | Severity | Finding | Affected task / criterion |
|---|---|---|---|
| FND-R01 | Blocker | `FT_Smoke` fails by timeout; the test suite is not green | T-FND-10 / AC-FND-08 |
| FND-R02 | Critical | A reentrant lethal hit broadcasts `OnDeath` twice | T-FND-05 / death-once contract |
| FND-R03 | Major | The CLI result check accepts a report with unexecuted tests | T-FND-10 / R-FND-08 |
| FND-R04 | Blocker | Primary Asset Type registration is deferred beyond the task that gates its first consumer | T-FND-07 / AC-FND-06 |
| FND-R05 | Major | Visual/profiling acceptance is incomplete, and this machine differs from the recorded reference PC | T-FND-06/08/09 / AC-FND-05/07/09 |
| FND-R06 | Minor | The previous FT graph instructions branch on a nonexistent return value | T-FND-10 handoff |
| FND-R07 | Minor | Overview and baseline prose still say the project does not exist | Project documentation |
| FND-R08 | Minor | Documented PowerShell build/package entry points are blocked by this machine's execution policy | Build workflow |

### FND-R01 — Functional smoke test is incomplete

Evidence: `Tools/run_tests.bat` exited **1**. Its JSON report contains **10 succeeded, 0 succeededWithWarnings, 1 failed, 0 notRun**. The failing path is `Project.Functional Tests.CastleDefender.Maps.Test.FT_Smoke.FT_Smoke_DummyDies`, with `Time's Up.. Test timed out in 60.005 seconds`.

`Tools/create_foundation_assets.py:144` explicitly describes creating the Start Test event without wiring the scenario. `tasks.md:281` also leaves that work unchecked. The handoff's limitation was accurate; successful unit tests do not satisfy AC-FND-08.

Remedy: wire the smoke scenario in the editor and rerun the complete runner. Keep T-FND-10 in Review until it passes.

### FND-R02 — Death is not committed before notifying observers

Location: `Source/CastleDefender/Combat/HealthComponent.cpp:37`.

`ApplyHit` broadcasts `OnDamaged` before setting `bDead`. If a damage observer causes another hit during a lethal hit, the nested call sees `bDead == false`, broadcasts damage at zero HP and broadcasts death. The outer call then broadcasts death again. This breaks the explicit death-once contract and would allow death listeners to execute rewards, removal or respawn handling twice.

Reproduced in **headless PIE**, using the existing `SpawnTestDummy` cheat and Python delegate observers. Initial HP was **100**. The damage callback applied one additional hit of 1 damage during the initial 100-damage hit. Observed:

```json
{"damaged":2,"death":2,"dead_during_lethal_damage":false,"current_health":0.0,"dead_after_hit":true}
```

The log contains two `TestDummy_0 died` lines. The existing spec only tests a second hit after the first call returns, so it misses this case.

Remedy: commit the lethal state before broadcasting callbacks and guard the death transition against reentrancy. Add a regression spec with a damage listener that delivers another hit. T-FND-05 is still recorded Done in the task table; this review recommends reopening it. No task status was silently changed during this review-only session.

### FND-R03 — Partial test runs can produce a green exit code

Location: `Tools/run_tests.ps1:24`.

The script rejects `failed > 0` and zero passed tests, but ignores `notRun` and the native editor exit status. Replayed the **unchanged report-evaluation suffix from the real script** against a synthetic report containing one Success and one NotRun: it printed `Passed: 1 Failed: 0 Not run: 1` and exited **0**. This reproduction checks report evaluation; it is not a second live engine test run.

Remedy: reject incomplete/nonterminal results, account for the editor process exit code, and ensure the default gate run includes both the expected spec group and functional tests. A successful filtered spec run must not stand in for the full Foundation gate. Cover partial reports and an editor failure after report creation.

### FND-R04 — Asset Manager acceptance is not implemented

Locations: `Config/DefaultGame.ini:5`, `tasks.md:203`, `tasks.md:212`, `spec.md:72`.

The Asset Manager section contains only a commented example; there is no project definition type registration. The implementation checkbox says “register UGameDefinition scan paths now” but is ticked despite no scan entry. The validation and ID specs pass, which verifies the base class, not registration/discovery of content.

The handoff postpones the criterion to the first `UHeroClassDefinition` in T-CMB-01. That task is P0 and depends on T-FND-07, while Foundation acceptance requires AC-FND-06 first. Deferring the criterion creates a gate-order conflict; it is not recorded as an approved change to the acceptance criteria.

Remedy options for the project owner: (1) complete a Foundation-owned registration/discovery demonstration consistent with the canonical type convention, or (2) explicitly approve and document deferral of feature-specific registration, with a Foundation validation criterion that can be finished before P0. Do not invent future gameplay definition classes or mark the criterion passed without that decision.

Epic documents Primary Asset Types to Scan as the configuration for discovering/registering primary assets: [UE 5.8 Asset Management](https://dev.epicgames.com/documentation/unreal-engine/asset-management-in-unreal-engine). Local UE 5.8.3 source was also checked. The audit's Python wrapper could not read `AssetManagerSettings.PrimaryAssetTypesToScan`; no successful reflected settings-array inspection is claimed.

### FND-R05 — Visual checks and hardware qualification

Locations: `tasks.md:184`, `tasks.md:236`, `tasks.md:261`, `technical-plan.md:216`.

The current machine reports **Intel Core i5-14600KF, NVIDIA GeForce RTX 5060, 17,011,228,672 bytes usable physical RAM (about 16 GB)**. The owner confirmed this machine as the reference PC on 2026-10-04, replacing the earlier i5-14500 / RTX 4070 Ti SUPER / 32 GB record.

Verification on the confirmed reference PC covers package launch, console controls, visible debug draw, visible `stat unit`, a rendered 30-second CPU/GPU/frame/memory capture and completed Insights analysis.

The mode stack restored `Modal -> Build -> Combat` correctly in scripted PIE. Real F5 delivery switched `Combat -> Build -> Combat`, and Build activated only `IMC_Build`, which contains no combat action mapping. Settings registration and native tag availability are covered by editor automation.

### FND-R06 — Correct the manual graph recipe

Location: previous entry in `ai/game/progress.md:40`; API: `Source/CastleDefender/Combat/TestDummy.h:26`.

`ApplyDebugHit` returns `void`, so it has no Return Value to use as a Branch condition. The Spawn Actor Return Value is an actor, not a boolean.

Correct recipe: Start Test -> Spawn Actor TestDummy (Always Spawn) -> validate the spawned reference -> Apply Debug Hit (Damage 1000) -> Branch with **Get Health(spawned dummy) -> Is Dead** as its condition -> Finish Test Succeeded/Failed. Also finish Failed if the actor reference is invalid. Do this in the editor, compile and save both assets, then rerun the CLI tests.

### FND-R07 / FND-R08 — Documentation and entry points

`project_summary.md:33` still says there is no Unreal project and Git is not initialized; `technical-plan.md:5` says nothing exists yet. These are stale orientation statements. Unimplemented feature types should remain marked as proposals, but implemented Foundation types should not be described as nonexistent.

`powershell -NoProfile -File Tools/build.ps1` was rejected because script execution is disabled. The review invoked the installed `Build.bat` and `RunUAT.bat` with exactly the arguments defined by the repo scripts. `Tools/run_tests.bat` worked using its existing process-scoped invocation. No persistent execution-policy change was made. Prefer a documented working launcher for build/package rather than asking agents to weaken machine policy.

## Acceptance matrix

| Criterion | Result in this review | Evidence / remaining work |
|---|---|---|
| AC-FND-01 | Pass | Editor Development and game Development built, exit 0 |
| AC-FND-02 | Pass; clean-clone verification recorded below | Fresh clone, LFS materialization and editor build pass after the correction commits. Both changed/new `.uasset` paths resolve to the LFS filter; no generated path is tracked. |
| AC-FND-03 | Pass | Native taxonomy spec resolves every required root and leaf through the tag manager, which supplies editor tag pickers. |
| AC-FND-04 | Pass | Components compile; normal and reentrant lethal-hit specs pass; scripted PIE confirms committed death and one death callback. |
| AC-FND-05 | Pass | Real F5 delivery to the rendered game window logs `Combat -> Build -> Combat`; Build activates only `IMC_Build`, which has no combat action mapping. Scripted PIE verifies stack restoration. |
| AC-FND-06 | Pass | Settings category/section metadata, Primary Asset registration/discovery/load and valid/invalid Data Validation cases pass in editor automation. |
| AC-FND-07 | Pass | Rendered scripted PIE runs the CVar and spawn cheat; rendered package screenshot shows the green debug sphere and health label; Shipping builds. |
| AC-FND-08 | Pass | Final default runner: 15 succeeded, 0 warnings/failed/NotRun/InProcess; editor and runner exit 0. |
| AC-FND-09 | Pass | The owner confirmed the current machine as the reference PC. It passes rendered package, stat/debug smoke and a clean 30-second CPU/GPU/frame/memory trace with completed Insights analysis. |

Build warnings: MSVC 14.51 is newer than the engine's preferred 14.50, and clean compilations emit deprecation warnings from **engine headers**. No project-source compiler warning was identified. Packaging's cook summary reports **0 errors, 0 warnings**. The original functional timeout was an actual test error and is now repaired. Earlier temporary audit probes encountered Python exposure restrictions; those probe errors are not attributed to production code.

## Verification artifacts (local, ignored by Git)

The `foundation-review-*` rows preserve the original failing audit. Current passing evidence follows them.

| Artifact under `Saved/` | Result |
|---|---|
| `foundation-review-editor-build-elevated.log` | Editor Development build succeeded |
| `foundation-review-game-build.log` | Game Development build succeeded |
| `foundation-review-tests.log`, `Automation/CLI/index.json` | 10 pass, 1 fail; runner exit 1 |
| `foundation-review-assets.json` | 21/21 assets load; mode rows and combat key mappings read from disk |
| `foundation-review-runner-partial.json`, `foundation-review-runner-harness.ps1` | Unmodified report-check suffix accepts one NotRun test, exit 0 |
| `foundation-review-pie.json`, `foundation-review-pie-engine.log` | Actual PIE reproduces duplicate death; mode restoration passes |
| `foundation-review-package.log` | Win64 Development package succeeded, cook 0 errors / 0 warnings |
| `foundation-review-packaged-engine.log` | L_Boot, debug CVar, spawn cheat and out-of-order mode pop work |
| `foundation-review-packaged.utrace` | 644,211-byte short smoke trace; not a 30-second profiling capture |
| `Automation/CLI/index.json` | Final gate: 15 pass, 0 warnings/failed/NotRun/InProcess |
| `foundation-fix-pie.json`, `foundation-fix-pie-engine.log` | Corrected health lifecycle and mode stack pass in PIE |
| `foundation-input-window.log` | Real F5 events: Combat -> Build -> Combat and matching mapping contexts |
| `foundation-editor-input.json`, `foundation-editor-input-engine.log` | Rendered PIE runs debug CVar, spawn cheat and stat unit; one dummy present |
| `foundation-late-cpu-trace-30s.utrace` | Clean rendered 30-second CPU/GPU/frame/memory capture; game exit 0 |
| `foundation-late-cpu-trace-30s-insights-clean.log` | Insights exit 0; CPU/GPU/memory providers complete |

Shipping was rebuilt successfully after the source fixes. The post-correction fresh-clone command and result are recorded in progress.md.

## Required next actions

1. Start the next eligible P0 task using the dependency order in the feature task files.
2. Keep the clean Foundation baseline green while adding P0 gameplay.

The original review changed only this report and the required session log. The subsequent user-authorized corrections are recorded above and in progress.md.
