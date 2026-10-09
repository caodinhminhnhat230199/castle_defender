---
name: ue5-animation-combat
description: Author and verify CastleDefender locomotion, attack, dodge, parry and hit-reaction animation while preserving montage timing, root motion and existing Blueprint assembly.
---

# CastleDefender combat animation

Adapted from `ue5 anim motion matching.md`. [Integration decisions](../../../ai/game/skill-integration.md) explain which proposals remain outside the current design.

Read the owning [Hero Combat task](../../../ai/game/01-hero-combat/tasks.md), [technical plan](../../../ai/game/01-hero-combat/technical-plan.md) timing contract, and the relevant phase of [production-plan.md](../../../ai/game/production-plan.md). T-CMB-19 is a provisional VS task, not permission to begin a locomotion replacement in P1.

## Preserve the current assembly

Inspect the saved `ABP_Warlord`, class definition's montage references, `UHeroAnimInstance`, existing notify types and `Tools/create_hero_assets.py` before editing. Preserve custom meshes, skeletons, weapons, sockets, graphs and authored windows. Use the Unreal Editor or enabled editor Python for binary assets.

Keep C++ authoritative for actions, stamina, defense and shared states. Animation presents those states and supplies montage timing callbacks; the AnimGraph must not own damage, combo state or defense timers.

## Content workflow

1. Before replacing clips, capture `ReportHeroWindows` from the tuned saved montages. Retain that report as the timing reference.
2. Retarget to the actual selected skeleton when needed; inspect root motion, orientation, sockets and mesh offsets. Do not enforce a new common skeleton or replace user-authored assembly.
3. Rebuild montages and reuse the existing hit, cancel, invulnerability, parry, assist and resistance notifies. Startup/Active/Recovery are readable phases, not a second schedule.
4. Preserve F/B/L/R dodge behavior and no-input backward dodge. End poses should transition naturally into the next allowed action; after Dodge that action starts a fresh Light chain.
5. Check interruption and blend-out exits, returning to locomotion without T-poses, frozen roots or stale windows. Adjust blend/inertialization only when supported by the current graph and observed artifact; upload blend durations are suggestions, not project tunables.
6. Check foot IK ordering against montage root motion and airborne feet. Existing authoring already handles FootIK around the montage slot; inspect it before adding another layer.
7. Re-run the window report and the owning task's regression suite. T-CMB-19 specifies each window stays within 0.03 s of its reference.

## QA at the gameplay camera

- Start/stop, camera-facing strafe/backpedal, pivots and lock-on movement show stable feet and facing.
- Light chain, Heavy, Dodge, Block, Parry, hit reaction, stagger and death have readable anticipation and exits.
- Cancel only where the montage permits; try input immediately before, inside and after each window.
- Check all dodge directions, chained allowed actions, counter starts and damage interruption for pose pops or sliding.
- Record gameplay footage and the owning task's rendered PIE result. A montage that passes timing tests still needs visual review.
- Profile the phase's real crowd scenario before changing animation LOD/update rates; do not inherit the upload's 20-enemy target as a new gate.

Motion Matching/Pose Search, Chooser, Motion Warping, paired executions, charge loops, tired-dodge clips and ragdoll get-ups are not part of this adaptation. Keep the existing locomotion pipeline; evaluate any future replacement through scope intake and plugin approval. Never assume the uploaded engine-version claims establish API availability.
