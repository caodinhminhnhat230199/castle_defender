# Uploaded UE5 skills: CastleDefender integration

Date: 2026-10-09. Owner: Codex. The user explicitly chose **integrate guidance and preserve current design**. This is a skill/documentation integration; it changes no gameplay, task status, phase gate, engine setting or dependency.

Follow-up: [alignment review and fixes](skill-alignment-review-2026-10-09.md) records the fresh build/test check, corrected planning mismatches and a minimal test-helper unity-build repair. The original integration's documentation-only validation below remains historical; the follow-up contains current verification and limits.

Runtime follow-up (2026-10-10): the user requested compatible gameplay implementation from the four combat/dodge/animation/ability uploads. [Implementation and verification record](uploaded-guidance-runtime-2026-10-10.md) documents T-SYN-02's armor-break integration, reproduced/fixed defensive/montage teardown, saved-content rendered PIE and remaining acceptance limits. The original documentation work below does not imply every uploaded mechanic was implemented.

## Installed project guides

The five original uploads remain intact at the project root as source material. The adapted skills live in `.claude/skills/`, following the existing project convention; Codex and Antigravity can read them directly through `AGENTS.md`, without assuming Claude-specific invocation is available.

| Original upload | Adapted guide | Use and task routing |
|---|---|---|
| [ue5 combat gas.md](../../ue5%20combat%20gas.md) | [ue5-combat-components](../../.claude/skills/ue5-combat-components/SKILL.md) | Existing action/timing/damage pipeline; T-CMB-02..06, 15, 20, 21 and later approved hit producers |
| [ue5 dodge perfect.md](../../ue5%20dodge%20perfect.md) | [ue5-dodge-parry](../../.claude/skills/ue5-dodge-parry/SKILL.md) | Stamina dodge, Block, first-hit Parry and counter; T-CMB-07..10, 15, provisional T-CMB-19 |
| [ue5 anim motion matching.md](../../ue5%20anim%20motion%20matching.md) | [ue5-animation-combat](../../.claude/skills/ue5-animation-combat/SKILL.md) | Existing animation assembly, montage timing and transition QA; content work and provisional T-CMB-19 |
| [ue5 ablities telekinesis.md](../../ue5%20ablities%20telekinesis.md) | [ue5-abilities-scope](../../.claude/skills/ue5-abilities-scope/SKILL.md) | Scope review; P1 T-SYN-02 integration and provisional VS T-CMB-17/18 only when eligible |
| [ue5 vfx impact.md](../../ue5%20vfx%20impact.md) | [ue5-vfx-impact](../../.claude/skills/ue5-vfx-impact/SKILL.md) | Existing feedback/data-table pipeline and Niagara QA; T-UXF-01/03/05/09 and later approved presentation tasks |

Task references identify ownership, not permission to reopen Done work or start a later phase. Read current task status/dependencies and `progress.md` at each session.

## Conflict resolution

These are applications of existing project decisions under the user's clarification, not new D-xx decisions.

| Uploaded proposal | CastleDefender rule retained | Integration outcome |
|---|---|---|
| GAS abilities, effects, AttributeSets and ExecutionCalculation | D-04 components; D-05 `UCombatLibrary::DeliverHit` | Reuse current combat owners and damage path; no GAS migration |
| Separate ComboComponent / DefenseComponent / impact subsystem | D-07 single state owner; D-10 one feedback subsystem | Route to `UHeroCombatComponent`, existing interceptors and `UFeedbackSubsystem` |
| Five-plus Light attacks, chords, priority input queue | R-CMB-08/09/13 latest buffered press, simple inputs, three-hit chain | Keep existing inputs/chain; retain buffering and lifecycle QA |
| Combo survives Dodge; zero stamina gets a slow dodge | R-CMB-11/13/19/20 refusal below cost, chain reset, stamina/recovery | Preserve behavior; Light-Light-Dodge-Light starts Light 1 |
| Predicted world-time ImpactTime and perfect-dodge scoring | R-CMB-19/43/44 and D-20 authored windows/hero clock | Retain actual hit interception, frame-rate/hit-stop edge testing; no registry/rewards |
| Universal attack DodgeCancel, diminishing agility, attack-through counter | R-CMB-07/25/27/50 commitment, whiff recovery, Parry counter | Use authored allowed-action windows and current first-success Parry |
| Focus, Flux, charge heavies, weapon forms, aerials and executions | R-CMB-01/15/38 approved Warlord verbs/identity | OUT OF SCOPE for this integration; no equivalent component implementation |
| Hurl, Gravity Well, Surge Slash, Stasis Seal, loadouts and boss ability rewards | R-CMB-38..41 / NEW-CMB-06 later Warlord traits | OUT OF SCOPE; ability guide routes only to approved SYN/VS work |
| Motion Matching, Pose Search, Chooser, Motion Warping, paired kills and ragdoll get-ups | Existing animation assembly; T-CMB-19 provisional timing-preserving polish | Possible future evaluation only; no graph replacement/plugin additions |
| New defense-color taxonomy, afterimages, Focus glow, FOV/impact flashes | CMB/ENM/UXF feedback contracts and accessibility | Keep defined event/state identities; reuse layered-effect/readability methods only |
| New impact profiles, mandatory global fallback effect | `DT_Feedback`, surface/variant defaults; R-UXF-06 missing-row behavior | Keep row fallback and missing-row diagnostics; no parallel assets/owner |
| Hit stop on every impact | R-UXF-07/08/09 large Hero impacts, actor-only, buffer-safe | Retain current tag/participant guards and real-time restore |
| Mandatory pooling, Chaos debris and fixed upload GPU/crowd budgets | D-15 measure first; approved phase budgets/scenarios | Retain profiling/scalability guidance; no adopted thresholds or new stress gates |
| `GAME_DESIGN.md`, `IMPLEMENTATION_PLAN.md`, M1..M8 | GDD, main plan D-xx/section 8a, feature spec/tasks | Replace foreign references with real project paths and task acceptance |

## Application and verification

The skills add implementation and content-review guidance, not new requirements. Existing spec rules/ACs and task acceptance remain authoritative. Runtime changes still need builds, tests, rendered PIE, data exposure and evidence under `AGENTS.md`.

This integration was grounded in current source: HeroCombatComponent, HeroClassDefinition, CombatLibrary, MeleeTraceComponent, CombatActionTiming, FeedbackTypes/FeedbackSubsystem, HeroAnimInstance, the hero authoring script and current module/project configuration. `.codegraph/` and its CLI were unavailable; source inspection used `rg` directly. No tool/index was installed.

Validation for this work checks skill frontmatter/names, local Markdown links, setup/catalog coverage and whitespace. Unreal build/automation/PIE are not applicable because no source, assets or runtime configuration change. Skill selection in a fresh Claude/Antigravity session remains untested; the portable route is the explicit skill links in `AGENTS.md`.


### User-approved exception: custom perfect dodge (2026-10-10)

The user explicitly requested original custom dodge animations and chose afterimage/sound plus a counter opportunity. NEW-CMB-13 / T-CMB-22 narrowly supersede the imported-guidance exclusion of perfect dodge. Reuse current components, counter and central feedback; preserve D-04/D-05/D-20. No GAS, global dilation or Motion Matching migration is included. See CMB spec R-CMB-55..58 and task acceptance before treating this as complete.
