# <Gate> playtest: <scenario>

## Session

| Field | Value |
|---|---|
| Date / time | |
| Build / version / branch / revision | Include uncommitted changes and relevant data/montage changes |
| Engine / hardware / configuration | |
| Gate / phase | G0-G3 or VS |
| Map / preset / scenario | |
| Tester / gate owner | |
| Session length (game / real seconds) | |
| Telemetry file | `Saved/Playtest/<yyyyMMdd_HHmmss>_<MapName>.jsonl` |
| Evidence files | Logs, test reports, screenshots, video, Insights trace |

## Hypotheses and gate checklist

Source of truth: [master plan section 3](../main_implement_plan.md#3-delivery-phases-and-gates). Copy **every item of the selected gate**, including the common requirements: successful build, no blocker bugs, recorded notes and a decision per hypothesis. Later-phase items are Out of phase, never Pass by omission.

| Gate checklist item / hypothesis | Expected experience / pass bar | Pass / Fail / Not tested / Out of phase | Evidence | KEEP / CHANGE / DELETE | Owner / follow-up |
|---|---|---|---|---|---|
| Build succeeds | | | | | |
| No blocker bugs | | | | | |
| Notes and decisions recorded | | | | | |
| <Copy each selected gate item here> | | | | | |

## Telemetry summary

Schema: [UXF technical plan section 5.4](../13-hud-feedback/technical-plan.md#54-telemetry-schema-json-lines). Report unavailable fields explicitly. Counts establish what occurred; the observations establish the player experience.

| Field | Result / evidence |
|---|---|
| `duration`, `result` | |
| `hero_deaths`, `parries`, `block_breaks` | |
| `actions` by type, `counts` by feedback tag | |
| `focus_seconds`, `focus_pct` | P3; Out of phase before Focus exists |
| `perks_offered`, `perks_picked` | P3 |
| `core_hp_min`, `core_hp_end` | P2+ |
| `structures_destroyed`, `squads_wiped` | P2+ / P1+ |
| RUN `step_end` timings | P3; reference the owning RUN events |

## Readability quiz (GDD section 28.1)

On ten random paused full-run frames, answer all seven questions within two seconds on at least eight frames (AC-UXF-25 / NEW-UXF-9). Before those features exist, mark their columns Out of phase and record a scoped readability observation; do not claim the full-run quiz passed.

| Frame / evidence | Squads' locations | Dangerous lane | Tower near breaking | Elite/siege enemies | Core HP | Current command state | Next forecast | All seven correct within 2 s? / elapsed |
|---|---|---|---|---|---|---|---|---|
| 1 | | | | | | | | |
| 2 | | | | | | | | |
| 3 | | | | | | | | |
| 4 | | | | | | | | |
| 5 | | | | | | | | |
| 6 | | | | | | | | |
| 7 | | | | | | | | |
| 8 | | | | | | | | |
| 9 | | | | | | | | |
| 10 | | | | | | | | |

## Sound-only test

Hide visual cues and randomize the event order. Record expected and identified events without prompting the listener. Current documented defaults: hit types at least 8/10; section 28.3 events at least 9/10 (NEW-UXF-9). Report event sets outside this phase separately.

| Trial / audio evidence | Event played | Event identified | Correct? | Note |
|---|---|---|---|---|
| <One row per trial> | | | | |

Identified / total: __ / __. Tester: __. Result: __.

## Feedback contract audit

Source: [UXF spec section 14](../13-hud-feedback/spec.md#14-feedback-contract-master-table-gdd-345). Copy all FC rows required by the selected phase. G0 scope: FC-02..07, FC-09..14, FC-16, FC-17, FC-65. Capture `game.feedback.Coverage` after the scenario and explain every unplayed phase tag. A row without a feedback tag still needs its montage/widget evidence.

| FC row / tag | Trigger exercised | Visual | Audio | UI | Readability / timing / duplicate count | Result / evidence | Owner / task for gaps |
|---|---|---|---|---|---|---|---|
| <One row per phase contract> | | | | | | | |

Coverage output: __. Unplayed tags and explanations: __. Automated table/contract report: __.

## Observations

Use the [workflow playtesting fields](../../../.claude/skills/game-development-workflow/references/09-playtesting.md). Design means the implementation meets its rules but the experience needs work; Bug means behavior violates the rules.

| Build / Version | Scenario | Tester | Expected Experience | Observed Behavior / evidence | Issue Type (Design / Bug) | Severity | Decision (keep / tune / redesign / remove / test again) | Follow-up |
|---|---|---|---|---|---|---|---|---|
| | | | | | | | | |

## Decisions and follow-ups

| Hypothesis | KEEP / CHANGE / DELETE | Reason / evidence | Owner | Task / changed assumption / open question |
|---|---|---|---|---|
| | | | | |

Gate owner review: __. Gate result: Not reviewed / Remain open / Passed. Required unresolved work: __. Next playtest scenario: __.
