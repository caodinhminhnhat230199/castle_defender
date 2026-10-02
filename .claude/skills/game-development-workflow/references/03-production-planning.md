# Production Planning

## Load When

Use when a feature needs art, animation, VFX, audio, UI, level content, or external asset work.

## Purpose

Answer:

> What content must exist before this feature can feel complete?

## Asset Inventory

Include only relevant categories:

- characters
- enemies
- props
- environment
- weapons
- buildings
- UI
- icons
- VFX
- audio
- animation
- cinematics
- level content

## Dependency Order

Typical 3D path:

```text
Reference
→ Blockout
→ Gameplay Validation
→ Model
→ UV / Bake / Texture
→ Rig
→ Animation
→ Engine Import
→ Material / Collision / LOD
→ Gameplay Integration
→ Polish
```

The exact art-production technique belongs in a dedicated art skill or project pipeline.

## Production Rules

- gameplay-critical blockouts come before polish
- placeholder assets are acceptable during prototype stages
- avoid final art before core dimensions and interaction are stable
- identify reusable/shared assets
- identify outsourceable work
- identify critical-path assets
- identify assets that can be produced in parallel

## Budget Awareness

When applicable, record:

- expected on-screen count
- texture class/resolution
- LOD needs
- skeleton sharing
- material reuse
- VFX density
- audio concurrency
- memory sensitivity

Do not invent engine-specific budgets without project/platform evidence.

## Output

`production-plan.md` should contain:

- asset inventory
- production order
- dependencies
- reusable assets
- integration checkpoints
- acceptance criteria
- risks
