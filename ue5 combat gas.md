---
name: ue5-combat-gas
description: Use when implementing offense in the UE 5.8 game - combo graphs with chord inputs, unarmed fillers, launcher and aerials, Focus charge and spend heavies, weapon forms with stance heavies, hit detection, damage/poise/Focus/Flux pipeline, executions, input buffering, hit-stop. For dodge/parry use ue5-dodge-perfect-dodge.
---

# UE5 Combat with GAS (offense and flow loop)

Read GAME_DESIGN.md §3 and §5 first. Blend: Crimson Desert combo structure (no grapple), Wukong Focus cash-in, Control-style Flux momentum.

## Architecture
- `AbilitySystemComponent` + `UCombatAttributeSet` (Health, Stamina, Focus, FocusPartial, Flux, Poise, DamageMult) on all combatants.
- Each attack / heavy / counter / execution = `UGameplayAbility` playing a montage via `UAbilityTask_PlayMontageAndWait`.
- `UComboComponent` + `UComboGraphAsset`: node = montage + tags; edge = input (Light, Heavy, Chord.LightHeavy, Chord.LightDodge, ...) + window + conditions (direction, airborne, weapon form, `State.CounterWindow`, Focus >= N).
- **Combo index persists through dodges** for `ComboResumeTime` (owned by ComboComponent, read by DefenseComponent).
- Damage: one `GE_Damage` with an `ExecutionCalculation` computing damage, poise, Focus gain, Flux gain, buffs. No damage math anywhere else.

## Offense content
- Light string ≥5 with unarmed fillers (kick/palm) as normal nodes.
- Heavy: tap = quick heavy; hold = `ChargeLoop` section that grants Focus over time; release spends all Focus (damage curve per pip).
- Varied heavies: heavy at light N → finisher N (1 Focus).
- Chords: detect two inputs within 50 ms in the buffer; launcher, lunge, augmented heavy.
- Aerial: launcher sets `State.Airborne` on both; aerial string; air dash; slam.
- Weapon forms (`UWeaponFormAsset`) swap combo graph and stance heavy; swap only at ChainWindow, keep combo index.
- Elemental augment: tag on form modifies arc/launch values via data.

## Hit detection
`AnimNotifyState_HitWindow` → swept shapes along weapon sockets (prev → current transform) → dedupe `(SwingId, Target)` → `Event.Hit` with weight tag. Debug cvar `Combat.DebugHits 1`.

## Windows (notify states)
`HitWindow`, `ChainWindow`, `DodgeCancel`, `IFrames`, `ParryWindow`, `SuperArmor`, `WarpWindow`, `ChargeLoop`.

## Input buffering
150–200 ms; priority dodge > parry > ability > attack; chords resolved before single inputs; clear on stagger.

## Flow loop
Light hit → FocusPartial + Flux. Perfect dodge/parry → Focus +1, Flux + (from DefenseComponent). Focus heavy / abilities → poise. Poise ≤ 0 → `State.Staggered` + `State.Executable`. Execution → `GE_ExecutionBuff`, Flux refund, Focus +1. All values in `DA_CombatTuning`.

## Executions
Paired animation (see animation skill), warp attacker to victim, invulnerable during, buff on finish, clean exit on interrupt.

## Hit-stop
0.02–0.12 s local `CustomTimeDilation` on attacker and victim, curve by weight; token-based restore; notify DefenseComponent so ImpactTime stays correct.

## Tests
Combo graph validator; scripted combo on dummy (hit count, Focus, Flux, poise); damage unit tests; chord detection test; combo continuity through dodge.

## Pitfalls
Logic in AnimGraph; missed `OnInterrupted`; double hits on re-entered windows; Focus spent before the heavy actually releases; cooldowns not as GameplayEffects.

## Done when
M2 and M4 acceptance pass, tests green, capture attached.
