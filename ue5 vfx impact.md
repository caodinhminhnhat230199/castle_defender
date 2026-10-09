---
name: ue5-vfx-impact-niagara
description: Use when creating or wiring combat VFX in the UE 5.8 game - weapon slash trails, hit impacts, perfect-dodge afterimage, Focus charge glow, shockwaves, debris, decals, hit-stop, camera shake, impact frames, ability effects, and their performance budgets with Niagara.
---

# UE5 VFX: impact, weight, spectacle

## Layered anatomy of every effect
anticipation -> trail -> contact flash -> sparks/debris -> shock/distortion + decal -> lingering residue. Build each as a layer or sub-emitter with exposed parameters.

## Slash trails
- Mesh-based slash + one master material (scrolling/erosion masks, fresnel, emissive), instances per element/weapon form. Three time-offset layers give depth.
- Niagara drives lifetime/scale/intensity; spawn from animation notify via GameplayCue, not directly in the AnimGraph.

## Impact system (data-driven)
`UImpactProfileAsset` key = `Surface.* x Damage.* x Weight.*` -> { Niagara system, SFX, decal, camera shake, FOV punch, hit-stop curve, optional impact-frame post-process }.
`UImpactFeedbackSubsystem` resolves the profile, pools systems, applies caps. Fallback profile always exists.

## Hit feel stack
1. Hit-stop (0.02-0.12 s, attacker/victim local) - biggest contributor to weight.
2. Contact flash 1-3 frames, bright.
3. Sparks/debris biased along hit normal and attack direction.
4. Camera: short shake + 1-3% FOV punch; optional chromatic/impact frame on finishers.
5. Audio synced to contact frame.

## Perfect dodge and Focus VFX
- Afterimage: pose snapshot (poseable mesh copy or Niagara skeletal-mesh sampling) with fresnel + dissolve material, 0.3-0.5 s; optional burst variant for the talent.
- Perfect dodge accent: chromatic ripple at the dodge origin, 1-frame flash, short desaturation pulse, sharp "ting" SFX.
- Focus: weapon glow intensity driven by charged Focus pips; release burst scaled by pips spent; HUD pip ignite.
- Telegraphs: Dodgeable white glint, Parryable gold glint, Unblockable red glow; never rely on color alone (add shape/audio).

## Ability VFX
GPU Niagara; distortion/refraction for gravity effects; Stasis Seal = crystalline lock shell around the frozen enemy; emissive over dynamic lights (MegaLights is production-ready in 5.8 but still budget dynamic lights). Chaos geometry collections for destruction with pooled, short-lived debris.

## Budgets (see plan section 6)
Niagara GPU <= 1.5 ms typical / 3 ms peak; <= 40 active systems with pooling; <= 300 active fractured pieces. Use scalability tiers, fixed bounds, GPU sim only where particle counts justify it, Niagara debugger and `stat gpu` to profile.

## Checklist per effect
- [ ] Reads clearly in 1 second at gameplay camera distance
- [ ] Exposed params: scale, intensity, color, weight
- [ ] Pooled, bounded, with kill-switch cvar
- [ ] Works on all surface profiles or falls back
- [ ] Matching SFX and camera response
- [ ] Capture + profile numbers attached

## Pitfalls
Over-bloom hiding gameplay; translucent overdraw stacking; unbounded debris; effects tied to montage blend-out; color-only telegraphs (add shape/audio cues).

## Done when
M7 acceptance in `IMPLEMENTATION_PLAN.md` passes and budgets hold in a 20-enemy stress map.
