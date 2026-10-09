---
name: ue5-vfx-impact
description: Author, wire and profile CastleDefender Niagara impacts, physical-surface sounds, state cues and camera feedback through the existing FeedbackSubsystem and data tables.
---

# CastleDefender impact VFX

Adapted from `ue5 vfx impact.md`; [integration decisions](../../../ai/game/skill-integration.md) retain the current design and performance policy.

Read [HUD/Feedback spec](../../../ai/game/13-hud-feedback/spec.md) R-UXF-01..12, 18 and the task's feedback-contract rows; [technical plan](../../../ai/game/13-hud-feedback/technical-plan.md); [tasks](../../../ai/game/13-hud-feedback/tasks.md); and the current phase in [production-plan.md](../../../ai/game/production-plan.md).

## Reuse presentation owners

- `UFeedbackSubsystem` is the existing world presentation owner (D-10). Extend its approved path rather than adding `UImpactFeedbackSubsystem` or a parallel impact-profile asset.
- `DT_Feedback` / `FFeedbackRow` owns VFX, surface sound overrides, camera shake, cooldown/burst limits and hit-stop tuning. `FFeedbackEventContext` carries participants, surface and variant/armor context.
- Preserve explicit variant selection, `.Armored` selection and base-row fallback. Unknown physical surfaces use the row's default sound. A missing tag/row follows R-UXF-06 diagnostics/no-op behavior, not an invented generic effect.
- Shared state icons/loop VFX come from `DT_CombatStatePresentation`; all unit types use the same state identity. Presentation observes gameplay state.
- Use existing gameplay/notify producers; avoid duplicate cues from AnimGraph, OnDamaged and hit callbacks for the same outcome. Evaded/ignored attempts can remain audiovisually silent.

## Author readable effects

Treat anticipation, trail, contact, particles/decal and residue as optional layers selected for the approved event. A Light hit does not need every layer. Preserve normal / armored / block / block-break / parry / stagger distinctions at gameplay distance.

Expose reusable asset parameters such as scale, intensity and color where they help authoring. Keep trails confined to authored windows and clean them up on interruption. Match impact position, normal, direction and sound to the actual resolved context.

Pair meaningful color cues with shape/audio. Avoid bloom, translucency and decals obscuring enemy telegraphs, the screen center or the lock-on target. Use Niagara effect-type scalability and sound concurrency required by R-UXF-04.

## Hit stop and camera

Only existing large-impact tags with local Hero involvement may stop the hit actors (R-UXF-07). `UFeedbackSubsystem` owns their `CustomTimeDilation`; restore saved original values using the current monotonic real-time deadline/overlap cap, safely handling destroyed actors and teardown. Never change global dilation; Tactical Focus owns it.

Preserve hero-time input/counter windows during the stop. Camera uses one owned active shake per resolved tag, row/global scale and existing radial attenuation; scale zero disables shake. Do not add automatic FOV punches, impact-frame flashes or desaturation.

## Performance and acceptance

Profile the owning phase's scenario with existing tools and retain measurements/capture. Diagnose excessive overdraw, lights, effect lifetime and bounds before increasing density. D-15 requires evidence before pooling; upload limits of 1.5/3 ms, 40 systems and 300 fragments are not adopted budgets.

Reuse feedback-table coverage auditing and relevant focused tests, then run the task's required gate. For visual changes, verify the saved assets in rendered PIE, sound-off hit identification and the prescribed blind-audio/readability checks. Mechanical tests do not establish feel or accessibility acceptance.

Perfect-dodge afterimages, Focus glow, gravity/freeze powers, Chaos fracture and new rendering features are outside this integration. Do not enable plugins/dependencies or rely on the upload's MegaLights/version claims without official pinned-engine verification and separate authorization.
