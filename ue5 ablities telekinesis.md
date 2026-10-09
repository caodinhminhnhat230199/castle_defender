---
name: ue5-abilities-telekinesis-destruction
description: Use when implementing supernatural abilities in the UE 5.8 game - telekinetic hurl, gravity well, ranged surge slash, stasis seal freeze, Flux cost, weapon forms with stance heavies, loadouts, Chaos destruction and physics-object handling.
---

# UE5 Abilities, Physics and Destruction

Inspired by momentum-driven ability loadouts (melee fuels powers; powers open executions). All designs here are original; do not reuse names or assets from other games.

## Ability pattern
Each ability = `GA_*` + `GE_Cost` + `GE_Cooldown` + `DA_Ability` (tuning, tags, VFX profile). Use `UAbilityTask`s for aim, charge, hold, and montage. Cancel rules via tags (`Block`/`Cancel` tag sets). Cost is Flux; melee hits, perfect dodges and executions refill it per the flow loop.

## Slice abilities
1. **Telekinetic Hurl**: select a physics object (cone/sphere trace + scoring by distance, mass, tag `Throwable`), pull with a physics handle or constraint, throw with impulse and damage on impact; on high speed impact fracture via Chaos Field or geometry collection break.
2. **Gravity Well**: spawn attractor (Chaos Field radial force + AI pull via movement impulse), short duration, ends with shockwave GameplayEffect (poise damage).
3. **Surge Slash**: projectile or swept blade volume tied to weapon form; scale damage with Flux; pierce rules in data.
4. **Stasis Seal**: freezes one enemy (pause montage + movement, `State.Sealed`) for a short duration; bosses get reduced duration or a partial slow; hits on a sealed enemy add bonus poise; release with a crack VFX. Diminishing returns if recast on the same target.

## Weapon forms
`UWeaponFormAsset`: mesh, socket set, combo graph, **stance heavy** (Twin Fangs: charge while moving, dash-through release; Monolith Staff: charge perch that evades ground attacks, crushing spin release), trails, impact weight, ability modifiers. Swap only inside a ChainWindow; combo index is kept.

## Loadout
`UAbilityLoadoutAsset` defines slots (ground/air), allowed abilities and forms; loadout UI is a minimal screen in the slice. Boss rewards unlock abilities (branching choice later).

## Chaos destruction rules
- Use geometry collections with limited fracture levels; pre-fracture props, avoid runtime heavy fracturing.
- Cap active pieces (<= 300), pool debris, auto-sleep and despawn after short lifetime, collision channel `Debris` ignores pawns.
- Kill plane and max velocity clamps to avoid physics explosions.
- Stress test: 50 throwable objects plus 3 gravity wells without hitching.

## Pitfalls
Physics handle fights with character collision; thrown objects hitting the owner; AI pathing broken by debris (use nav modifiers off for debris); network-unfriendly physics (out of scope, keep deterministic-friendly where cheap).

## Done when
M5 acceptance in `IMPLEMENTATION_PLAN.md` passes: each ability data-driven, cancel-safe, VFX/SFX complete, stress test stable.
