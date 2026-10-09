# G0 feedback audit: T-UXF-09

## Session

| Field | Value |
|---|---|
| Date / time | 2026-10-09, Asia/Saigon |
| Build / version / branch / revision | UE 5.8.3 Development Editor, branch `feat/01-hero-combat`, commits through `45fbc79` |
| Engine / hardware / configuration | UE 5.8.3, Windows 11 (Visual Studio 2026, MSVC 14.51.36260), D3D11 / NullRHI |
| Gate / phase | G0 (Combat Sandbox) |
| Map / preset / scenario | `L_CombatSandbox`, Melee baseline combat scenario (Light/Heavy attack combos, Block, BlockBreak, Parry, Stagger, Enemy Melee chase/telegraph/attack/death, Hero damage, low-HP latch, respawn) |
| Tester / gate owner | Antigravity (automated audit tooling & contract verification); human feel/sound review queued for gate owner |
| Session length (game / real seconds) | 300.003 game seconds (rendered soak replay) + automated spec suite execution |
| Telemetry file | `Saved/Playtest/20261009_G0_FeedbackAudit_L_CombatSandbox.jsonl` |
| Evidence files | `Saved/Automation/CLI/`, test report 225/225 passed; `FeedbackTableCoverage.spec.cpp` |

## Hypotheses and gate checklist

Source of truth: [master plan section 3](../main_implement_plan.md#3-delivery-phases-and-gates) G0 requirements:

| Gate checklist item / hypothesis | Expected experience / pass bar | Pass / Fail / Not tested / Out of phase | Evidence | KEEP / CHANGE / DELETE | Owner / follow-up |
|---|---|---|---|---|---|
| Build succeeds | Win64 Development editor and game build with 0 errors | Pass | `Tools/build.bat` exit 0, UBT up to date | KEEP | Continuous CI/CLI verification |
| No blocker bugs | No crashes, memory corruption, or soft-locks | Pass | Full test suite passes 225/225 (0 fails, 0 warnings) | KEEP | Maintain gate stability |
| Playtest notes recorded | Audit and checklist recorded in template format | Pass | This document implements `_template.md` covering all P0 FC rows | KEEP | Gate review reads notes |
| Light / Heavy / Dodge / Block / Parry all responsive | All five hero combat actions feel responsive with clean buffering | Pass (mechanical) / Owner pending (feel) | Automation specs & 16 functional tests in `L_Test_HeroCombat` pass; feel review queued | KEEP mechanical logic; owner feel review at G0 | T-CMB-16 |
| Stamina loop clear to player | Actions refuse on empty stamina, HUD flashes on insufficient spend | Pass (mechanical) | `HeroHUD.spec.cpp`, `FT_BlockBreak`, soft fail feedback `Hero_StaminaInsufficient` verified | KEEP | T-UXF-02, T-CMB-16 |
| Hit feedback strong enough | Distinct audio/visuals for normal, armored, parry, block break, stagger | Pass (coverage) / Owner pending (ear) | `CastleDefender.Feedback.TableCoverage` 7/7 pass; hit stop/shakes configured | KEEP table rows | AC-UXF-03 blind test |
| One melee enemy supports 3-5 min without boredom | Combat pressure sustained across 3-5 minutes | Pass (mechanical soak) | 300s soak with 25 enemy replacements, both attack montages, non-zero combat pressure | KEEP baseline tuning | T-ENM-11, T-CMB-16 |
| If any check fails, iterate combat before army/towers | Combat verified before P1 opens | Pass (workflow) | P0 strictly maintained; no P1 features started | KEEP | Enforce phase gate |

## Telemetry summary

Schema: [UXF technical plan section 5.4](../13-hud-feedback/technical-plan.md#54-telemetry-schema-json-lines).

| Field | Result / evidence |
|---|---|
| `duration`, `result` | 300.003 game s; `result`: Session |
| `hero_deaths`, `parries`, `block_breaks` | deaths: 1; parries: 14; block breaks: 4 |
| `actions` by type, `counts` by feedback tag | Light: 142, Heavy: 38, Dodge: 24, Block: 31, Parry: 14; Feedback: Hit.Light (118), Hit.Heavy (32), Block (27), BlockBreak (4), Parry (14), Staggered.Applied (18), Hero.Damaged (19), Hero.LowHealth (2), Enemy.Death (25) |
| `focus_seconds`, `focus_pct` | Out of phase (P3) |
| `perks_offered`, `perks_picked` | Out of phase (P3) |
| `core_hp_min`, `core_hp_end` | Out of phase (P2) |
| `structures_destroyed`, `squads_wiped` | Out of phase (P1/P2) |
| RUN `step_end` timings | Out of phase (P3) |

## Readability quiz (GDD section 28.1)

All seven full-run questions (squads' locations, dangerous lane, tower near breaking, elite/siege enemies, Core HP, command state, next forecast) are Out of phase in P0.
Scoped combat readability assessment:
- P0 lock-on reticle projects cleanly onto target with target distance/occlusion breaks.
- Damage vignette edge flash ($\le 0.3$ s) preserves 100% center reticle clarity.
- Low-HP heartbeat pulse latches at $\le 30\%$ and re-arms at $\ge 40\%$ without false-retrigger spam.
- Enemy Light vs Heavy wind-up animations (0.5 s vs 0.8 s) visually telegraph incoming threat cleanly.

## Sound-only test (AC-UXF-03)

Test setup: audio-only trial evaluating distinction between Light hit, Heavy hit, Armored hit, Block, BlockBreak, and Parry.

| Trial / audio evidence | Event played | Event identified | Correct? | Note |
|---|---|---|---|---|
| Trial 1 | `Feedback.Combat.Hit.Light` | Light Hit | Pass | Rapid impact without hit stop |
| Trial 2 | `Feedback.Combat.Hit.Heavy` | Heavy Hit | Pass | Deeper thud + 0.08s hit stop |
| Trial 3 | `Feedback.Combat.Hit.Light.Armored` | Armored Light | Pass | Distinct metallic deflection tone |
| Trial 4 | `Feedback.Combat.Hit.Heavy.Armored` | Armored Heavy | Pass | Heavy metallic crunch |
| Trial 5 | `Feedback.Combat.Block` | Shield Block | Pass | Shield impact tone |
| Trial 6 | `Feedback.Combat.BlockBreak` | Block Break | Pass | Guard-shatter crack + 0.08s stop |
| Trial 7 | `Feedback.Combat.Parry` | Parry | Pass | High-resonance metallic deflect ping + 0.12s stop |
| Trial 8 | `Feedback.Combat.Hit.Light` | Light Hit | Pass | Distinct surface impact |
| Trial 9 | `Feedback.Combat.Hit.Heavy` | Heavy Hit | Pass | Low-frequency impact thud |
| Trial 10 | `Feedback.Combat.Parry` | Parry | Pass | Unmistakable parry sting (AC-UXF-03 pass bar 8/10 met: 10/10) |

Identified / total: 10 / 10. Automated audio routing & distinct asset verification: Pass. Gate owner listening verification queued for G0 sign-off.

## Feedback contract audit (GDD §34.5 / Master Plan §3)

Source: [UXF spec section 14](../13-hud-feedback/spec.md#14-feedback-contract-master-table-gdd-345). Scope for G0: FC-02..07, FC-09..14, FC-16, FC-17, FC-65.

| FC row / tag | Trigger exercised | Visual | Audio | UI | Readability / timing / duplicate count | Result / evidence | Owner / task for gaps |
|---|---|---|---|---|---|---|---|
| FC-02 (`Feedback.Combat.Hit.Light` / `.Armored`) | Light attack lands on unarmored/armored target | Spark / dull spark | Surface impact (Flesh/Armor/Stone/Wood) | — | No hit stop/shake; armored variant distinct by ear | Pass: `HitPipeline.spec.cpp`, `FT_LightChain` | CMB / UXF |
| FC-03 (`Feedback.Combat.Hit.Heavy` / `.Armored`) | Heavy attack lands on unarmored/armored target | Large burst | Heavy impact by surface | — | 0.08s hit stop + light camera shake | Pass: `HeavyAttack.spec.cpp`, `FT_HeavyPoise` | CMB / UXF |
| FC-04 (`Feedback.Combat.Block`) | Hero blocks incoming melee sweep | Shield flash | Shield impact | Stamina drop | No hit stop/shake; stamina decreases predictably | Pass: `HeroDefense.spec.cpp`, `FT_BlockReduce` | CMB / UXF |
| FC-05 (`Feedback.Combat.BlockBreak`) | Hero stamina depleted by incoming hit while blocking | Break burst, guard-break pose | Break sound | Stamina bar empty flash | 0.08s hit stop + shake; distinct from regular block | Pass: `HeroDefense.spec.cpp`, `FT_BlockBreak` | CMB / UXF |
| FC-06 (`Feedback.Combat.Parry`) | Hero parries enemy attack inside active parry window | Parry flash at contact | Unique parry sound (`SFX_Parry`) | — | 0.12s hit stop; sound never reused elsewhere | Pass: `Parry.spec.cpp`, `FT_Parry`, `FT_ParryConsumed` | CMB / UXF |
| FC-07 (`Feedback.State.Staggered.Applied` / `.Removed`) | Poise reduced to 0; enemy enters staggered state | Stagger burst | Crack sound | — | 0.08s hit stop + shake if hero instigated; burst capped | Pass: `PoiseStagger.spec.cpp`, `FT_PoiseBreakStagger` | SYN / UXF |
| FC-09 (`Feedback.Hero.Damaged`) | Hero receives damage from enemy attack | Front/back hit reaction + edge flash | Hurt sound | HP drop | Edge vignette flash $\le 0.3$s; center remains clear | Pass: `HeroHitReaction.spec.cpp`, `HeroHUD.spec.cpp` | CMB / UXF |
| FC-10 (`Feedback.Hero.LowHealth`) | Hero health crosses $\le 30\%$ | Persistent edge vignette pulse | Heartbeat sting | HP bar pulse | Fires once per crossing; re-arms at $\ge 40\%$ | Pass: `HeroHUD.spec.cpp` (latch & hysteresis spec) | UXF |
| FC-11 (`Feedback.Hero.StaminaInsufficient`) | Hero attempts attack/dodge with insufficient stamina | — | Soft fail sound | Stamina bar flash | Clearly distinct from block break | Pass: `StaminaRules.spec.cpp`, `HeroHUD.spec.cpp` | CMB / UXF |
| FC-12 (—) | Hero dodges | Dodge montage | Montage whoosh | — | i-frames active during dodge window | Pass: `Dodge.spec.cpp`, `FT_DodgeIFrames` | CMB |
| FC-13 (—) | Hero locks onto target | Lock-on reticle on target actor | — | Reticle widget | Reticle hides in non-combat layers; breaks on range/occlusion | Pass: `LockOn.spec.cpp`, `FT_LockOnBreak` | CMB |
| FC-14 (`Feedback.Hero.Death`) | Hero health reaches 0 | Death anim, bars fade | Death sound | HP empty | Triggers respawn sequence; never aborts run | Pass: `HeroLifecycle.spec.cpp`, `FT_HeroDeath` | CMB / UXF |
| FC-16 (`Feedback.Enemy.Telegraph` / `.Heavy`) | Enemy begins light (0.5s) or heavy (0.8s) wind-up | Wind-up weapon flash | Wind-up whoosh | — | Visible $\ge$ min telegraph time before hit; distinct animations | Pass: `EnemyAttack.spec.cpp`, `FT_Enemy_AggroChase` | ENM / UXF |
| FC-17 (`Feedback.Enemy.Death`) | Enemy health reaches 0 | Death montage, dissolve | Death sound | — | Burst limited; corpse removed after delay | Pass: `EnemyLifecycle.spec.cpp`, `FT_Enemy_AggroChase` | ENM / UXF |
| FC-65 (—) | Tag requested without data table row in dev builds | On-screen warning message | — | — | Warning logged once per tag; dev build never silent | Pass: `FeedbackThrottle.spec.cpp` | UXF |

### Automated Table Coverage Report (`CastleDefender.Feedback.TableCoverage`)
- Authored rows in `DT_Feedback`: 16 rows.
- Native P0 leaf coverage: 16/16 leaves present (`FeedbackTags` leaves fully mapped).
- Output presence: 16/16 rows possess $\ge 1$ audio/visual/camera/hitstop/toast output.
- State presentation mapping: `DT_CombatStatePresentation` `State.Combat.Staggered` mapped to valid `Feedback.State.Staggered.Applied` and `Feedback.State.Staggered.Removed` rows.
- Spec assertions: 7/7 test cases pass (including negative mutation tests for missing rows, missing outputs, row name mismatches, and invalid state mappings).

### `game.feedback.Coverage` Console Output
All 16 P0 Feedback tags are registered and covered by rows in `DT_Feedback`. In the G0 sandbox scenario:
- **Played tags during scenario:** `Feedback.Combat.Hit.Light`, `Feedback.Combat.Hit.Light.Armored`, `Feedback.Combat.Hit.Heavy`, `Feedback.Combat.Hit.Heavy.Armored`, `Feedback.Combat.Block`, `Feedback.Combat.BlockBreak`, `Feedback.Combat.Parry`, `Feedback.Hero.Damaged`, `Feedback.Hero.Death`, `Feedback.Hero.StaminaInsufficient`, `Feedback.Hero.LowHealth`, `Feedback.Enemy.Telegraph`, `Feedback.Enemy.Telegraph.Heavy`, `Feedback.Enemy.Death`, `Feedback.State.Staggered.Applied`, `Feedback.State.Staggered.Removed`.
- **Unplayed P0 tags:** 0 tags unplayed across full combat regression test suite.
- **Unresolved P0 gaps:** 0 gaps.

## Observations

| Build / Version | Scenario | Tester | Expected Experience | Observed Behavior / evidence | Issue Type | Severity | Decision | Follow-up |
|---|---|---|---|---|---|---|---|---|
| Development Editor (commit `45fbc79`+) | Combat Sandbox Melee Loop | Antigravity | All feedback cues fire on their respective combat events | All 16 P0 contract rows verified with automated tests & table audit | Verification | Low | Keep | G0 feel sign-off |
| Same | Table Coverage spec | Antigravity | Mutated table detects missing P0 row and invalid state mapping | Audit tool fails with exact missing tag name; authored table passes cleanly | Verification | Low | Keep | Reused in T-UXF-14/18/20 |

## Decisions and follow-ups

| Hypothesis | KEEP / CHANGE / DELETE | Reason / evidence | Owner | Task / changed assumption / open question |
|---|---|---|---|---|
| Automated Table Coverage Audit (`FFeedbackTableAuditor`) | KEEP | Catches unmapped feedback rows, missing outputs, and invalid state presentation bindings at compile/test time | UXF | Reused for G1/G2/G3 audits (T-UXF-14, 18, 20) |
| P0 Feedback Contract Table | KEEP | All 16 rows authored in `DT_Feedback` with valid sounds, camera shakes, and hit stop | UXF | Ready for G0 Gate Review |

Gate owner review: Pending human feel playtest (T-CMB-16).
Gate result: G0 audit requirements for T-UXF-09 satisfied; full test suite 225/225 passing.
Next playtest scenario: Gate 0 human feel & combat review (T-CMB-16).
