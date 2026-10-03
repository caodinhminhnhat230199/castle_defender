# Foundation: Specification

| | |
|---|---|
| GDD sections | Header (platform, engine, implementation direction, team), §29, §31, §34.1, §34.2, §38 |
| Phase | F (before P0) |
| Status | Draft v1 |

Foundation has no player-facing gameplay. Its "player" is the developer and the coding agents. This spec defines what the empty project must do before any gameplay task starts.

## 1. Overview

A UE5 C++ project that builds, runs, tests and profiles, with the shared contracts every feature depends on: folder layout, Gameplay Tags, input, the combat damage contract, tuning settings, debug tooling and the automation harness.

## 2. Developer Experience

- Any agent can open the repo, read `main_implement_plan.md`, build, and run tests from the command line.
- Adding a content item (enemy, tower, perk) means adding a Data Asset, not editing core code (GDD §34.1).
- Debug views for combat, AI and lanes are one console command away.

## 3. Core Loop (development)

```text
Pick task → Implement → Build → Run Automation/Functional tests → PIE check in sandbox map → Commit
```

## 4. Rules

- R-FND-01 (header) [LOCKED]: Project targets UE5, PC, single-player.
- R-FND-02 (header) [LOCKED]: Gameplay core in C++; UI, VFX, authoring and tuning in Blueprint/data.
- R-FND-03 (§34.1) [LOCKED]: Content types are defined as data so new content needs no core code change.
- R-FND-04 (§38) [LOCKED]: Every [TUNABLE] value is exposed in a Data Asset or `UGameTuningSettings`.
- R-FND-05 (§34.2) [LOCKED]: Each state category from §34.2 has exactly one owner (master plan D-07 table).
- R-FND-06 (§29.1, §29.2) [LOCKED]: Input is action-based (Enhanced Input) so every core command can be mapped to gamepad later; KBM is the primary target.
- R-FND-07 (§31) [LOCKED]: Profiling happens in packaged Development builds on a recorded reference PC.
- R-FND-08 (§38) [LOCKED]: Every task can be verified independently (automation or scripted PIE check).
- R-FND-09 (README) [LOCKED]: Generated folders and `.codegraph/` are never committed.

## 5. Developer Actions

Build editor/game targets, run tests from CLI, toggle debug views, spawn test actors through cheats, package a Development build, capture an Insights trace.

## 6. Success / Failure

Success: all `AC-FND-*` pass. Failure: any gameplay task starting before `T-FND-01..07` are done.

## 7. Scope

### In Scope
Project creation, VCS, folders, tags, input skeleton, combat contract skeletons, tuning settings, asset manager types, debug tools, test harness, packaging smoke test, reference PC.

### Out of Scope
Gameplay logic (owned by features), CI server (add when a second machine or contributor appears), editor module, plugins, SaveGame, CommonUI.

## 8. Anti-Goals
- No framework building "for later": no event bus, no generic manager layer, no plugin per feature.
- No GAS setup (D-04).

## 9. Dependencies
None. Everything else depends on this.

## 10. Edge Cases
- Engine upgrade mid-project: pin version in `T-FND-01`; upgrades are explicit tasks.
- Large binary assets: LFS from the first commit, otherwise history bloats.

## 11. Acceptance Criteria
- AC-FND-01: `<Game>Editor` and `<Game>` Development targets build from the command line with zero errors.
- AC-FND-02: Fresh clone + LFS pull + build works on a second folder (no missing files).
- AC-FND-03: Tag taxonomy roots from the master plan exist as native tags and appear in the tag picker.
- AC-FND-04: `FCombatHit`, `UHealthComponent`, `UCombatStateComponent` compile, are Blueprint-visible, and a test actor with `UHealthComponent` dies when hit for its max HP (Automation Spec).
- AC-FND-05: Pressing a debug key switches mapping context (`IMC_Combat` ↔ `IMC_Build`) and the log shows the active context.
- AC-FND-06: `UGameTuningSettings` appears in Project Settings; Primary Asset Types are registered; a Data Asset with a missing required field fails `IsDataValid`.
- AC-FND-07: `game.debug.*` CVars toggle debug draw; cheat `SpawnTestDummy` works in PIE.
- AC-FND-08: One Automation Spec and one Functional Test pass from the command line runner.
- AC-FND-09: Packaged Development build launches `L_Boot` on the reference PC; `stat unit`, a `game.debug.*` CVar and an Insights trace capture work.

## 12. Open Questions / Assumptions
- Q-15: project name, module name, UE version, reference PC spec.

## 13. System Contract

| Item | Content |
|---|---|
| Responsibility | Build, test, debug and data infrastructure plus shared gameplay contracts |
| Inputs | Engine version choice, project name |
| Outputs | Buildable project, tag taxonomy, input contexts, combat contract types, settings, debug + test tooling |
| State | None at runtime beyond settings |
| Events | `OnDamaged`, `OnDeath`, `OnStateAdded`, `OnStateRemoved` declared (fired by features) |
| Data model | Primary Asset Types listed in `technical-plan.md` §9 |
| Failure cases | Build break, missing LFS objects, invalid data assets |
| Performance | Profiling workflow defined; no runtime cost |

## 14. Feedback Contract
Not applicable (no player-facing state). Debug feedback: on-screen debug text and Visual Logger entries for every debug CVar.
