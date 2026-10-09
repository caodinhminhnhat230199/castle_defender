---
name: ue5-animation-motion-matching
description: Use when building or tuning character animation in the UE 5.8 game - Motion Matching locomotion, short directional dodges, attack montages, cancel and combo-resume poses, motion warping, executions as paired animation, hit reactions, ragdoll get-up, IK, and smoothness QA.
---

# UE5 Animation: smooth, readable, responsive

## Strategy
- Locomotion = Motion Matching (Pose Search schema with trajectory + pose channels).
- Attacks, dodges, counters = authored montages with root motion and Motion Warping (exact frame data).
- Reference: Epic's Game Animation Sample (GASP) updated for 5.8 — multi-character motion matching, Pose Search Interaction Assets, Chooser with Pose Match columns, motion-matched get-ups. Verify stability in the installed build.
- One skeleton; IK Retargeter for everything.

## Smoothness rules (Wukong-level fluidity)
- Every attack has a `DodgeCancel` frame; no attack locks the player out of dodge for more than its short commit.
- Use inertialization on all cancels; blend 0.05–0.12 s.
- **Dodge set:** 8 directions (or 4 + blend), ≤0.45 s, exit pose designed to flow into light attack #1 and into the *resumed* combo node.
- Dodge chain: separate slower "tired" clip for diminishing agility.
- Charge heavy: looping section with additive breathing; release from any loop frame without pops.
- Counter moves start from the dodge's exit pose.

## Montage checklist
Sections Startup / Active / Recovery; notify states `HitWindow`, `ChainWindow`, `DodgeCancel`, `IFrames`, `ParryWindow`, `SuperArmor`, `WarpWindow`, `ChargeLoop`; frame-data sheet per move; strong anticipation; follow-through kept with additive settle when cancelled.

## Motion Warping
Set targets on ability start; window from startup to contact; clamp distance; no-warp fallback out of range.

## Executions (paired)
Preferred: Pose Search Interaction Assets with warp. Fallback: paired montages with sync point and per-size-class variants. Always have interrupt exits.

## Hit reactions
Directional additives for light hits; stagger montages on poise break; launch → ragdoll → motion-matched get-up; reaction priority tags prevent stacking.

## Procedural
Foot IK, look-at, two-hand IK, Control Rig secondary motion. Animation on worker threads; no heavy BP in AnimGraph.

## QA before done
- [ ] 10-minute free play, no pops/T-poses
- [ ] Dodge out of every attack at its DodgeCancel frame
- [ ] L, L, dodge, L resumes visually seamless
- [ ] Sprint/stop/pivot/strafe no foot slide
- [ ] Charge-release from any loop frame clean
- [ ] 60 fps with 20 animated enemies (LOD, URO, significance)

## Done when
M1 and animation parts of M2–M4 and M8 pass with footage.
