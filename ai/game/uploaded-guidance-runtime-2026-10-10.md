# Uploaded UE5 guidance: runtime integration

Date: 2026-10-10. Agent: Codex. Branch: `feat/01-hero-combat`.

The user requested compatible gameplay implementation from four uploads, followed by a Vietnamese progress report with detailed references. This work applies the existing [design-preserving integration](skill-integration.md), current P1 phase and approved feature rules. The originals remain source material.

## Delivered gameplay and ownership

| Upload | Application in current gameplay | Owning task / references | Limits |
|---|---|---|---|
| `ue5 combat gas.md` | Existing component/DeliverHit pipeline now applies target armor centrally to every incoming layer. Heavy's newly applied state cannot amplify its own hit. Existing combo, poise, hit deduplication and hero-time buffering remain covered by regression. | [T-SYN-02](04-battlefield-synergy/tasks.md); `Combat/HealthComponent.*`, `Combat/CombatStateComponent.cpp`, `Core/GameTuningSettings.*` | D-04 retains components; no GAS, Focus/Flux or combo replacement. |
| `ue5 dodge perfect.md` | Reproduced and repaired defensive teardown: close i-frames/Parry/cancel/trace windows, clear buffer/counter/guard, stop owned montage, restore root-motion scale and detach observers. Late action requests are refused. Existing saved Dodge is checked in rendered PIE at configured frame caps. | T-CMB-07/15 correction; `Hero/HeroCombatComponent.*`, `Tests/Dodge.spec.cpp` | Existing authored i-frames and chain reset remain; no perfect-dodge registry/rewards. |
| `ue5 anim motion matching.md` | Owned montage teardown detaches the actual per-montage callback, prevents queued stale callbacks, and releases combat/root-motion state. Saved `ABP_Warlord`, Enhanced Input and authored Heavy/Dodge/reaction playback verified; FootIK returns to 1. | T-CMB-07/15 correction; `Hero/HeroCombatComponent.cpp`, [PIE script](../../Tools/verify_uploaded_guidance_pie.py) | No Motion Matching/Pose Search/Motion Warping/graph replacement; provisional T-CMB-19 remains Todo. No new production animation clips or 10-minute human animation sign-off claimed. |
| `ue5 ablities telekinesis.md` | Applies its compatible effect/data/lifetime guidance to Warlord Heavy's already specified Armor Broken opening: eligible target, immutable authored data, shared world-time duration and shared feedback producer. | P1 T-SYN-02, R-SYN-11..14 / AC-SYN-07..08 | Hurl/Gravity Well/Surge Slash/Stasis Seal/loadouts remain outside the approved Warlord design. Provisional T-CMB-17/18 and later phase tasks are not started. |

## T-SYN-02 implementation

- `UHealthComponent::ComputeDamageAfterArmor` implements damage × (1 − effective armor), clamping BaseArmor to 0..0.9 and the configured remaining-armor multiplier to 0..1. `ApplyHit` evaluates the already active state before `DeliverHit` applies this hit's new states; returned damage remains actual HP loss.
- Game Tuning exposes `ArmorBrokenArmorMultiplier` (default 0.25) and Armor Broken duration (default 6 s). Every layer reuses this calculation.
- `UCombatStateComponent` rejects Armor Broken on units without positive BaseArmor before state events/timers/feedback.
- Saved `DA_HeroClass_Warlord.Heavy.AppliedStates` includes `State.Combat.ArmorBroken`; the default duration stays data-driven. Existing montage windows and other definition values were preserved.
- Added `Feedback.State.ArmorBroken.Applied`, its `DT_Feedback` output and `DT_CombatStatePresentation` row. Removal is silent per spec; `OnStateRemoved` remains the observation/cleanup contract. State icons/body VFX remain T-SYN-04/UXF work.
- `SFX_ArmorBreak` is a dedicated placeholder duplicated from installed engine `StartSimulate`; license/source recorded in [LICENSES.md](../../Content/CastleDefender/Placeholder/LICENSES.md).
- Added `DA_Enemy_ArmorTest` (armor 0.5, health 1000), duplicating the existing test enemy's body/AI/attack data. It is a fixture, not the T-ENM-06 Armored archetype.
- `DebugHitTarget <Damage> <Poise>` produces a simulated Army hit through DeliverHit. The shared debug resolver prefers lock-on; crosshair fallback queries Pawn capsules bounded by Visibility world occlusion, since Pawn/CharacterMesh collision profiles ignore Visibility.
- [Content authoring script](../../Tools/create_armor_break_content.py) preserves existing rows/tuning/assets and backs up changed packages under `Saved/Backups/armor-break-content` before saving. Binary packages were edited only through Unreal Editor.

## Verification evidence

| Check | Result / evidence |
|---|---|
| Editor build | `Tools/build.bat` passes, exit 0; `Saved/skill-runtime-editor-final-build.log`. |
| Development game build | `Tools/build.bat -Target CastleDefender` passes, exit 0; `Saved/skill-runtime-game-final-build.log`. |
| Armor focused tests | 3/3 pass, editor/runner exit 0; `Saved/armor-tests.log`. Covers armor 0/0.5/0.9, bounds, applying-hit order, Hero/Army/Tower benefit, removal and unarmored state refusal. |
| Teardown regression | Before fix: 0 passed/1 failed, six stale-state assertions; `Saved/dodge-teardown-red-tests.log`. After fix: the same regression passes in the full suite, including late action refusal. |
| Complete automation gate | 244 passed, 4 failed, 0 succeededWithWarnings, 0 NotRun; editor exit 255, runner exit 1. Only the unfinished T-SQD-01 spawn/content cases fail; none disabled. `Saved/skill-runtime-full-tests-final.log`, exported `Saved/skill-runtime-full-report.json`. |
| Rendered saved-content PIE | `L_CombatSandbox`, D3D11, saved Hero/enemy assembly; Enhanced Input Heavy starts and its real melee sweep applies Armor Broken. Heavy damage 15 (base 30 with full 0.5 armor), duration 6 s; simulated Army 20 damage gives 10 -> 17.5 -> 10, with one applied feedback event. `Saved/uploaded-guidance-pie.json`, `Saved/uploaded-guidance-pie-engine.log`; screenshots under `Saved/Screenshots/WindowsEditor`. |
| Actual debug crosshair cheat | The added check first fails (0 target damage, missing crosshair target warning). After the Pawn-query repair, `DebugHitTarget 20 0` deals 10 damage to the aimed armored enemy and the complete PIE script succeeds, editor exit 0. Failure archived as `Saved/uploaded-guidance-pie-crosshair-red.json` / `-engine.log`; final proof is in the PIE JSON/log above. |
| Dodge / animation PIE | At configured caps 30/60/120, hits during authored i-frames return Evaded without HP loss, hits after them remove 20 HP, reaction exits to Idle and FootIK returns to 1 on saved `ABP_Warlord_C`. No transient animation fixture used. Slate deltas are recorded; this is not proof of sustained exact frame rates or a complete millisecond edge sweep. |
| Content idempotency | Re-running the editor authoring script exits 0 and SHA-256 hashes of all five existing packages remain identical; `Saved/armor-content-idempotency.log`. |

Known engine-header C4996 and MSVC preference notices remain. PIE's Recast teardown warning also occurs in earlier recorded PIE runs; editor-layout migration is an environment notice. No new gameplay warning is accepted. Initial sandbox .NET build aborted before compilation; the authorized normal Unreal launcher succeeded. Initial asset sound discovery and crosshair-cheat verification failed and were corrected; failures remain in their saved logs.

## Remaining acceptance and task state

**T-SYN-02 is Review.** Implementation and scoped tests/PIE are present, but owner sound/readability acceptance and the project-wide automation blocker remain open. Existing CMB Done statuses and G0 acceptance are retained; corrected lifecycle integration is recorded as a separate work item. **T-SQD-01 remains In Progress** and its source/content/tests were not modified by this work.

The four full-suite failures are:

1. `CastleDefender.Army.Spawn.discovers the authored SquadDefinition asset and validates its soldier blueprint`: saved SquadDefinition fixture is missing.
2. `CastleDefender.Army.Spawn.rejects fourth squad before it creates any soldiers`: test controller registration does not establish the expected registry/spawn setup.
3. `CastleDefender.Army.Spawn.spawns configured count/stats/team/back-pointers with rotated home-relative grid positions`: same controller fixture setup.
4. `CastleDefender.Army.Spawn.uses edited definition count at next spawn and destroys only owned soldiers on teardown`: same controller fixture setup.

### Manual steps

1. In `L_CombatSandbox` PIE, run `SpawnEnemy DA_Enemy_ArmorTest 1` and `game.debug.CombatStates 1`. Hit it with Heavy; inspect the Armor Broken debug state and expiry. `DebugHitTarget 20 0` on the aimed/locked target should deal 17.5 while broken, then 10 after expiry.
2. Listen to the dedicated armor-break cue and review Heavy/Dodge/reaction transitions at the gameplay camera. The scripted PIE runs use `-nosound`; no owner audio/feel approval is inferred.
3. Complete T-SQD-01's controller fixture/content/PIE acceptance and rerun `Tools/run_tests.bat`; retain the four failing tests until they pass. Finish review before advancing dependent T-ENM-06 or calling G1 passed.

Next: owner review of T-SYN-02 and completion of the existing T-SQD-01 handoff. Other dependency-ready P1 tasks may proceed according to the current task graph. No branch switch, commit, push, PR, engine upgrade, plugin/module/dependency addition or GDD edit occurred.

API reference checked for the authoring helper: [Epic UE 5.8 GameplayTagLibrary Python documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/GameplayTagLibrary), plus pinned local engine headers/collision profiles.
