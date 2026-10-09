# G0 recording dry run

## Session

| Field | Value |
|---|---|
| Date | 2026-10-08 to 2026-10-09, Asia/Saigon |
| Build / version | Development editor/game, `feat/01-hero-combat`, uncommitted Codex changes |
| Engine / configuration | UE 5.8.3, local Windows PC, D3D11 rendered PIE plus NullRHI Automation |
| Gate / map / scenario | G0 remains open; `L_CombatSandbox`, Block/Parry/lock-on and respawner verification |
| Tester / gate owner | Codex mechanical checks / owner review pending |
| Session length | Separate short rendered scenarios; respawner ten-minute soak not yet accepted |
| Telemetry | T-UXF-08 not implemented; structured fixture results under Saved, not a telemetry summary |
| Evidence | `Saved/cmb09-pie.json`, `cmb10-pie.json`, `enemy-defense-pie.json`, `cmb14-full-tests.log`, `cmb14-pie.json`; progress log records exact per-task artifacts |

## Hypotheses and G0 checklist

Every G0 item from [master plan section 3](../main_implement_plan.md#3-delivery-phases-and-gates), including the common gate requirements:

| Item / hypothesis | Result | Evidence / limits | Decision | Follow-up |
|---|---|---|---|---|
| Build succeeds | Pass at respawner implementation checkpoint | Editor and game builds; full gate 171/171. Later input changes require fresh validation. | KEEP | Final G0 build still required |
| No blocker bugs | Fail / investigation | Respawner live PIE did not restore owned capacity before the next kill; isolated six specs pass. | CHANGE | T-CMB-14 |
| Playtest notes recorded | Pass for recording dry run | This record exercises all checklist/evidence/decision fields. | KEEP | Gate owner reviews T-UXF-11 template |
| Light / Heavy / Dodge / Block / Parry all responsive | Not tested by a human | Mechanical authored-input/sweep/window checks passed; no claim of satisfying combat from automation. | KEEP mechanical behavior; owner decision pending | T-CMB-16 |
| Stamina loop clear to player | Not tested | Block/stamina math verified; HP/stamina HUD still T-UXF-02. | CHANGE | T-UXF-02, T-CMB-16 |
| Hit feedback strong enough | Not tested | Parry sound authored; impact/shake task T-UXF-03 remains Todo. Sound was disabled in scripted PIE. | CHANGE | T-UXF-03, T-UXF-09 |
| One melee enemy supports 3-5 minutes without boredom | Not tested | Lifecycle soak and content tuning differ from an owner feel test. | KEEP scenario; owner decision pending | T-ENM-11, T-CMB-14/16 |
| If any check fails, iterate combat before army/towers | Pass as workflow | P1 remains closed; no later-phase feature work. | KEEP | Repeat G0 after prerequisites |

## Telemetry summary

`duration/result`, deaths/parries/block breaks, action counts and feedback counts: unavailable until T-UXF-08. Focus, perks, Core HP, structures, squads and RUN step timings: Out of phase. Fixture JSON is linked as mechanical evidence and is not substituted for the prescribed session summary.

## Readability quiz

All seven full-run questions (squads' locations, dangerous lane, tower near breaking, elite/siege enemies, Core HP, command state, forecast) are Out of phase in P0. Ten-frame/full-run pass bar is not claimed. P0 lock-on marker and action poses were visually inspected in the individual task evidence; owner readability check pending.

## Sound-only test

Events played/identified: Not tested, 0 trials. Scripted sessions used `-nosound`. Hit type 8/10 and section 28.3 event 9/10 pass bars remain pending T-UXF-09/gate review.

## Feedback contract audit

| FC scope | Trigger / current evidence | Result / gap | Owner |
|---|---|---|---|
| FC-02,03 | Light/Heavy real sweeps and math | Impact outputs still require UXF-03 audit | CMB/UXF |
| FC-04,05 | Front Block and block break verified | HUD empty flash/audio still pending | CMB/UXF |
| FC-06 | Real Parry cancels enemy trace and staggers | Distinct sound asset exists; blind audio/longest hit stop audit pending | CMB/UXF |
| FC-07 | Shared poise break/stagger verified | UXF outputs and SYN Functional Test pending | SYN/UXF |
| FC-09,10,11 | Damage/stamina rules have specs | HUD/vignette/low-health/readability audit pending | CMB/UXF |
| FC-12,13 | Authored Dodge and lock-on rendered evidence | Mechanical/visual checks pass; owner feel review pending | CMB |
| FC-14 | Existing death/respawn evidence | Bar fade/readability audit pending | CMB/UXF |
| FC-16,17 | Real melee attack and enemy lifecycle verified | Blind sound/visual audit pending | ENM/UXF |
| FC-65 | Silent controller mode-stack specs | UXF shell/layer integration pending | FND/UXF |

`game.feedback.Coverage`: Not run as the G0 audit. Unplayed tags require scenario-level explanation under T-UXF-09. This dry run does not close contract rows.

## Observations

| Build / Version | Scenario | Tester | Expected Experience | Observed Behavior | Issue Type | Severity | Decision |
|---|---|---|---|---|---|---|---|
| Uncommitted UE 5.8.3 | Respawner first kill | Codex | Fresh enemy after five game seconds | Owned capacity remained pending at next kill deadline | Bug | Blocks T-CMB-14 acceptance | test again after root-cause repair |
| Same | Counter-parry enemy sweep | Codex | Closed trace safely cancels old sweep | Reentrant close originally accessed cleared samples; fixed and red/green regression passed | Bug | Crash, fixed | keep regression |
| Same | G0 combat feel | Owner pending | Responsive and satisfying combat | No human observation yet | Design | Gate evidence missing | test again |

## Decisions and follow-ups

| Hypothesis | KEEP / CHANGE / DELETE | Evidence / owner |
|---|---|---|
| Independent owned-slot respawn | KEEP approach; CHANGE defect | T-CMB-14 diagnostics pending |
| One Parry success and reserved first counter | KEEP documented default | Mechanical tests pass; NEW-CMB-01 owner review at G0 |
| Record missing evidence explicitly | KEEP | T-UXF-11 template review pending |

Gate owner review: Pending. Gate result: **Remain open**. Questions: NEW-CMB-01..04/08/10 remain with the owner. Next: repaired respawner soak, eligible P0 HUD/feedback/telemetry/test work, then owner G0 playtest. No army/towers before G0.
