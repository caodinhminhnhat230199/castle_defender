# Skill alignment: review and corrections

Date: 2026-10-09. Agent: Codex. Branch: `feat/01-hero-combat`.

The five adapted skills fit the approved game design. This review found and repaired a Development game build failure and documentation that could send subsequent work back to P0, into a closed phase, or toward duplicate input/presentation owners. Components, stamina, combo reset on Dodge, first-success Parry, montage timing, the existing animation assembly and shared feedback pipeline are retained.

## Scope and evidence

Reviewed the original uploads and project adaptations, AGENTS/current handoff, main plan decisions/contracts, CMB/ENM/SYN/UXF task/spec/technical-plan sections, source for combat resolution/traces, action/defense/timing, animation, shared states, feedback/HUD/telemetry, test helpers, authoring scripts and project/build configuration. Followed mode-stack consumers into CSM, TFM and PRK docs. Compared task status/dependencies with G0 and owner-review records. This is a skill-alignment review of those connected systems, not a claim that every later-phase implementation exists or every source file received a full security/performance audit.

CodeGraph CLI/index were unavailable; used direct `rg`/source inspection. Preserved prior uncommitted skill work and original uploads. No branch switch, commit, push, plugin/module/dependency or GDD change.

## Findings fixed

| ID | Severity | Finding and consequence | Repair / evidence |
|---|---|---|---|
| SKA-01 | Blocker | Hero/Enemy test helper namespaces were imported at file scope. In a unity translation unit both `GetOrCreateTestHero` functions became visible: game build failed C2668 despite a passing editor/automation gate. | Scoped each `using namespace` to its scenario function in [Hero helper](../../Source/CastleDefender/Hero/HeroCombatTestLibrary.cpp) and [Enemy helper](../../Source/CastleDefender/Enemy/EnemyCombatTestLibrary.cpp). Real red game build exit 6, then game/editor builds and full suite green. No fixture behavior changed. |
| SKA-02 | Major | Main plan's G0 checklist, demo status and next actions still said P0/Not started and suggested a P2 DEF spike before its phase opened. Project summary also said P0 in progress. | [Main plan](main_implement_plan.md) / [summary](../../project_summary.md) now reflect the recorded G0 pass/P1 opening; later gates remain closed. Next-task examples are checked against actual dependencies. No gate was newly approved by this review. |
| SKA-03 | Major | [CMB WIP](01-hero-combat/WIP.md) said 14 Done / 1 Review / 6 Todo and pending P0 acceptance, contradicting the current table and G0 handoff. | Replaced the active handoff with 17 Done / 4 Todo, no Review tasks. All P0 CMB tasks are Done; P2 Interact and provisional VS traits/polish remain gated. Historical notes remain labeled superseded. |
| SKA-04 | Major | CMB plans described existing resolution/interrupt systems as proposals, suggested nonexistent per-notify headers/separate phase/Chain types, and mixed implemented data with future multiplier/interaction fields. | Updated [CMB technical plan](01-hero-combat/technical-plan.md), [task intro](01-hero-combat/tasks.md) and main section 8a to actual declarations/paths. Phases derive from notify bounds; Chain uses CancelWindow allowing Light. Rejected invalid/dead/no-health/same-team attempts return before resolution creation. P1 multipliers/P2 interaction are explicitly future tasks. |
| SKA-05 | Major | UXF task notes allowed bHeroOnly to define hit-stop eligibility, used a void radial camera helper that cannot retain/scale instances, and permitted real-time input buffering contrary to D-20. Restore text offered an obsolete dilation-scaled timer. | Aligned [UXF tasks](13-hud-feedback/tasks.md) / [plan](13-hud-feedback/technical-plan.md) with the implemented strict large-impact guard, returned-instance camera routing, hero-time input/counter deadlines and monotonic real-time restore. Resolved-row tag is broadcast, and positive Hero HP loss is an independent damage cue. |
| SKA-06 | Major | CSM/TFM/perk instructions bypassed D-19 with direct input-mode/UI writes and forced Combat restoration; one CSM note incorrectly claimed the controller uses a switch rather than a stack. UXF wording encouraged separate feature-owned layer flags. | Corrected CSM/TFM/PRK plans and task notes plus UXF spec/plan to exact `PushMode(EPlayerMode::Spirit/Focus/Modal, Reason)` / `PopMode(Reason)` and presentation observation of `OnPlayerModeChanged`. Nested/out-of-order restoration follows the stack. Future controller-to-layer integration remains work for the mode-owning tasks; it is not falsely labeled implemented P0 code. |
| SKA-07 | Minor | Telemetry descriptions mixed current played-feedback/action-state records with planned raw resolution/Core/run sampling; NEW-UXF-12 and an audio checklist still appeared pending despite the G0 owner record. | Clarified current versus planned telemetry and retained the recorded KEEP decision in [UXF spec](13-hud-feedback/spec.md). Linked the recorded G0 blind-audio resolution without claiming a new listening test. |
| SKA-08 | Minor | Project summary omitted all five new guides; integration's original documentation-only validation could be mistaken for the follow-up's build/test result. | Added guides to the summary and linked this separate follow-up from [skill integration](skill-integration.md). Original integration evidence remains historical. |
| SKA-09 | Minor | Feature/phase/demo totals omitted T-CMB-20/21, still reporting CMB 19 / total 271 and undercounting every cumulative gate by two. The D1 path also omitted those prerequisites of Block/Lock-on. | Task-table enumeration finds 273 unique tasks, CMB 21; phase totals 10/32/33/55/75/68. Main plan/summary now include both tasks in the 31-task playable path and report cumulative gate totals 42/75/130/205/273. |

## Alignment by skill

| Guide | Runtime / content mapping | Result |
|---|---|---|
| `ue5-combat-components` | Existing HeroCombatComponent, HeroClassDefinition, montage windows, CombatLibrary/MeleeTraceComponent and state owners | Source and saved-content regressions pass; no GAS/parallel pipeline needed |
| `ue5-dodge-parry` | Existing stamina refusal, directional montage/i-frames, chain reset, consumed Parry and hero-time counter | Existing action/defense/Parry/Dodge/low-FPS tests pass; perfect-dodge mechanics remain outside the selected design |
| `ue5-animation-combat` | Existing HeroAnimInstance, saved ABP/montages/root-motion assembly and timing inspection | Automated montage/pose/content checks pass; no fresh footage or visual polish acceptance claimed |
| `ue5-abilities-scope` | P1 T-SYN-02; provisional VS T-CMB-17/18 only when eligible | Scope/phase routing matches task dependencies; supernatural ability/loadout systems are not introduced |
| `ue5-vfx-impact` | Existing FeedbackSubsystem, physical-surface sounds, variants, hit-stop/camera lifecycle and feedback-table auditor | Feedback/impact/table coverage and functional suite pass; new spectator effects/pooling budgets are not adopted |

## Verification

- Baseline editor build: passed, exit 0. Baseline full gate: **235/235**, zero failures/warnings/NotRun, editor exit 0. An initial sandboxed bundled .NET invocation exited -532462766 before a build result; the auto-reviewed normal launcher succeeds outside the sandbox.
- Baseline Development game build: **failed, exit 6**, C2668 on `GetOrCreateTestHero`. Evidence preserved in `Saved/skill-recheck-game-build-red.log`.
- After repair: `Tools/build.bat -Target CastleDefender` **passed** (`Saved/skill-recheck-game-build-green.log`); `Tools/build.bat` **passed** (`Saved/skill-recheck-editor-build-green.log`); both exit 0.
- Final `Tools/run_tests.bat`: **235/235**, zero failed/succeededWithWarnings/NotRun, editor exit 0 (`Saved/skill-recheck-full-tests-green.log`, exported report `Saved/Automation/CLI/index.json`). Suite includes saved hero/enemy/feedback content and functional maps.
- Existing engine-header C4996 and MSVC toolchain preference notices remain; no project compiler warning identified in the final build logs. No warnings were suppressed.
- All five adapted skill validators, local links/balanced fences, five-skill catalog coverage, seven setup Bash blocks (syntax only) and `git diff --check` pass. Task enumeration validates 273 unique IDs, phase totals, CMB statuses, the 31-task D1 dependency closure / exact 42-task G0 set and dependency-ready P1 examples. These checks are separate from runtime tests.
- No fresh rendered PIE, owner feel/blind-audio session, packaging, Shipping build or performance capture was performed. The source change scopes helper lookup only; regression scenarios run through the full existing suite. Recorded G0 acceptance is retained and attributed to its original record, not independently reconfirmed here.

## Next eligible work

P1 Todo tasks with all dependencies Done as checked here: **T-SQD-01**, **T-ENM-13**, **T-SYN-02**. Recheck live status before choosing one. T-UXF-04 waits for T-SQD-01/T-ENM-06; T-SYN-07 waits for T-SYN-03. T-CMB-12 is P2; T-CMB-17/18/19 are provisional VS even when their individual prerequisites become Done.

Use the adapted guides for the chosen task, record actual code/content/PIE evidence, and update `progress.md`. This review does not authorize a PR, deployment or work behind a closed phase gate.
