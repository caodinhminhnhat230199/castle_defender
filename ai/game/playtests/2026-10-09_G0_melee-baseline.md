# G0 melee baseline: T-ENM-11

UE 5.8.3, Development D3D11 editor PIE, `feat/01-hero-combat`, uncommitted Codex work. Map: L_CombatSandbox. **G0 remains open; owner feel/readability/audio evaluation pending.**

DA_Enemy_Melee starts from the valid DA_Enemy_Test fixture so its initial tuning matches regression evidence. No numeric balance adjustment is accepted as final yet. BP_Enemy_Melee inherits the existing Quinn rig/animation/stagger presentation and uses red Paint Tint material instances. Duel/Pair use the playable DA; the original five east-side fixture actors/data remain preserved. One melee behavior, two attacks, no enemy block or later archetypes.

## Saved tuning

Read back from the saved DA in `Saved/enm11-tuning.json`:

| Field | Value | Initial baseline reason |
|---|---|---|
| Max HP / armor | 100 / 0 | Existing damage acceptance; armored gameplay stays P1 |
| Walk speed / local aggro | 300 cm/s / 600 cm | Existing chase/engage baseline |
| Decision interval | 0.2 game s | Existing timer-driven brain |
| Max poise / regen delay / rate | 50 / 2 s / 25 per s | Existing Heavy/Parry poise baseline |
| Stagger duration | 1.5 game s | Existing SYN duration |
| Minimum attack gap / turn rate | 1 s / 360 degrees/s | Existing commitment/tracking baseline |
| Corpse delay | 3 game s | Existing removal/lifespan contract |
| Light | Range 150 cm, damage 10, poise 10, weight 2, cooldown 0, rate 1 | Approved shorter placeholder wind-up, 0.5 s |
| Heavy | Range 150 cm, damage 20, poise 25, weight 1, cooldown 3 s, rate 1 | Distinct charged clip, approved wind-up 0.8 s |

Gameplay values stay in the DA or montage windows; the playable DA can be tuned independently of fixed fixtures. Red tint is placeholder presentation.

## Areas and verification

| Group | Centre (cm) | Hero start for this scenario |
|---|---|---|
| 1 | (-1300, -650) | (-1650, -650) |
| 3 | (-1300, 1200) | (-1650, 1200) |
| 5 | (1300, 1600) | (950, 1600) |

Nine Melee_1/3/5 actors are added without rebuilding/deleting arena content. They start Idle outside aggro, then chase/attack when approached. Default Duel stays enabled at (500, -1500); Pair stays disabled. The planned `SpawnEnemy DA_Enemy_Melee <Count>` dev helper initializes the DA before FinishSpawning, with a P0 per-call cap of five.

First rendered soak: 300.001 game seconds, about 100 seconds per group, real Light/Heavy/Dodge/Block and timed Parry input; both enemy montage types observed. Twenty-four dead enemies replaced. Stamina minima: 69.5 / 7.45 / 27.03; enemy HP minima before replacement: 5 / 5 / 10. Native editor exit 0 (`Saved/enm11-pie.json`, `enm11-pie-exit.json`). Screenshots 00037-00039 show red enemies and actual poses. God protects the fixture Hero, and sound is disabled: this establishes mechanical integration, not HP pressure or human engagement/audio readability.

One saved-content spec validates the DA, telegraph timing, distinct attacks, inherited presentation and red tint. Full gate 180/180 with no test warnings/failures/not-run and editor exit 0; editor/game builds pass. Authoring rerun adds nothing. A repeated console lookup performance notice from QA replacements is addressed by direct Blueprint/dev calls after one console smoke call; a clean repeat is in progress. No packaged performance verdict is inferred.

Clean repeat completed: 300.003 game seconds, 25 replacements, 23/78/105 observed attack starts in the 1/3/5 groups, both attack montages in each group. Stamina minima 69.5/40.07/0; HP minimum 5 in each group before replacement. Native exit 0; only known engine crowd-manager teardown warnings (`Saved/enm11-pie.json`, `enm11-pie-exit.json`, `enm11-pie-engine.log`). The first paragraph's initial-run figures remain as historical evidence; random attack selection and the scripted input mix make these soak metrics observations, not accepted tuning targets.

## Owner hypotheses and follow-ups

Play each group normally, with God off, for five minutes and telemetry on. Record Dodge/Parry readability, time-to-kill, stamina/HP pressure and boredom/engagement. Evaluate every applicable [G0 check](../main_implement_plan.md#3-delivery-phases-and-gates); final impact verdict also needs T-UXF-03.

| Hypothesis | Current evidence / limit | KEEP / CHANGE / DELETE |
|---|---|---|
| One simple melee enemy supports the loop | Moves/attacks, is damaged/staggered/killed/replaced under input | KEEP two attacks as initial set; owner evaluates AC-ENM-09 |
| Red Quinn reads against Warlord | Rendered distinction; no glance/blind test | KEEP placeholder pending owner readability |
| HP/poise/timing provides useful pressure | Stamina varies; God masks HP pressure | CHANGE after normal-play observations, one value at a time |
| Impact/audio strong enough | T-UXF-03 and blind sound checks pending | CHANGE through that task; keep G0 open |

| Build / Version | Scenario | Tester | Expected Experience | Observed Behavior | Issue Type | Severity | Decision |
|---|---|---|---|---|---|---|---|
| UE 5.8.3 uncommitted | Material authoring | Codex | Verified tint assignment | Engine setter always returns false; read-back and render prove assignment | Engine API bug | Tooling | keep read-back validation |
| Same | QA replacements | Codex | Shared spawn helper | Repeated Exec lookups trigger a performance notice | Fixture issue | Verification noise | direct helper after one console call |
| Same | G0 pressure/engagement | Owner pending | Useful 3-5 minute combat | God/no-sound automation cannot establish player experience | Design evaluation | Required G0 evidence | test again normally |

Feedback audit, quiz and blind audio remain their phase/owner tasks. Final tuning and G0 approval are not recorded. Next: normal owner playtest, impact/vitals feedback and remaining P0 Functional Tests.
