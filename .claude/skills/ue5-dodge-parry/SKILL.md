---
name: ue5-dodge-parry
description: Implement, tune or verify CastleDefender directional dodge, authored i-frames, block, single-use parry and counter windows without adding perfect-dodge mechanics.
---

# CastleDefender dodge and parry

Adapted from `ue5 dodge perfect.md` under the user's instruction to preserve the current design. See [integration decisions](../../../ai/game/skill-integration.md).

Read [Hero Combat spec](../../../ai/game/01-hero-combat/spec.md) R-CMB-07..13, 19..28, 43..45, 50..52, [technical plan](../../../ai/game/01-hero-combat/technical-plan.md), and the relevant [task](../../../ai/game/01-hero-combat/tasks.md). T-CMB-07, 08, 09, 10 and 15 own the existing integration; T-CMB-19 owns provisional production animation polish.

## Approved behavior

- Dodge starts only when stamina and the current action's allowed cancel window permit it. Insufficient stamina refuses the action with the existing feedback; there is no free slow dodge.
- Dodge consumes stamina and has authored invulnerability and recovery. Direction follows camera-relative movement, or backward relative to facing with no input; preserve F/B/L/R selection and lock-on integration.
- Dodge resets the Light chain. `Light, Light, Dodge, Light` returns to Light 1 (R-CMB-13).
- Attack cancellation is allowed only by the authored action list/window, damage interruption or death. Do not introduce unconditional DodgeCancel frames on every attack.
- Block obeys the front arc, HP reduction and stamina damage. Block break ends guard and applies SYN's shared Staggered.
- Parry is its own input. The first valid hostile hit inside its window/front arc consumes it; further hits resolve normally. A whiff's committed recovery cannot be bypassed by Block or Dodge.
- Successful Parry applies attacker poise damage through the existing shared state path, broadcasts `OnParrySucceeded`, and opens the hero-time Counter Window. Preserve existing first-counter-payload consumption and optional data-authored counter montage.

## Owners and lifecycle

Reuse `UHeroCombatComponent` / `ICombatHitInterceptor`, `FHeroDodgeData`, `FHeroParryData`, `UStaminaComponent`, `UCombatStateComponent`, existing montage notifies and `ULockOnComponent`. No new `UDefenseComponent`, incoming-attack registry or GAS abilities are needed.

Timing follows D-20: hero windows/buffer/counter use hero dilated time, world states use game time, and UXF restores hit stop in real time. Judge defense at actual `DeliverHit` interception against authored windows, rather than predicting an ImpactTime on world time.

On interruption, death, montage completion and pawn replacement, verify window closure, root-motion scale restoration, state cleanup and observer rebinding. Notify assets contain configuration; transient state belongs to the pawn's components.

## Useful QA from the upload

- Exercise window edges at different frame rates, including a low-FPS/hitch case, using existing timing/debug tools. Log observed results rather than changing windows to mask a failure.
- Check real directional input while free and locked, zero input, stamina refusal, recovery and interruption during the dodge.
- During hit stop and global Tactical Focus dilation, confirm the buffer/counter survives and invulnerability follows the montage.
- Test multiple attackers/reentrant callbacks: one Parry success, no duplicated counter benefit, and later hits resolve normally.
- Check transition poses and foot slide at the gameplay camera; animation polish must preserve approved windows.

Perfect-dodge scoring/rewards, afterimages, Focus/Flux, damage reduction, attack-through counters, diminishing agility and color-coded defense categories are outside current scope. Do not import their numeric defaults or tests requiring Light 3 after Dodge.

Follow task acceptance with build, automation and rendered PIE; record evidence in `progress.md`. Human responsiveness/readability approval remains separate from scripted timing tests.
