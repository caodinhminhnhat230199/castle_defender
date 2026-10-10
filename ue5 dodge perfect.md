---
name: ue5-dodge-perfect-dodge
description: Use when implementing or tuning defense in the UE 5.8 game - dodge, Wukong-style perfect dodge with afterimage and Focus reward, counter window, combo continuity after dodge, diminishing agility, parry, attack-through counter, and enemy attack telegraph tagging.
---

# Dodge and Perfect Dodge (smooth, ju generous, rewarding)

Design intent (GAME_DESIGN.md §5.2): one button, fast, never resets offense; perfect timing pays out Focus, Flux and a counter. Original implementation; reference only the feel.

## Components
- `UDefenseComponent` on every combatant: registry of incoming attacks, judges perfect dodge/parry, tracks dodge chain for agility decay.
- `GA_Dodge`, `GA_Parry`, `GA_Counter*` abilities; `DA_DefenseTuning` holds every number.

## Incoming attack registry
1. When an enemy attack ability starts, compute `ImpactTime` = now + (time to the montage's first `HitWindow` start, scaled by play rate and time dilation).
2. Send `IncomingAttack{Attacker, Target, ImpactTime, DefenseTag, AttackId}` to the target's DefenseComponent (and AoE targets by radius).
3. Remove entries when the HitWindow closes, the attack is interrupted, or the attacker dies.
Recompute `ImpactTime` if the attacker's play rate or dilation changes (hit-stop!).

## Perfect dodge judgment
- On dodge start at time `t`: for each pending attack with `Attack.Defense.Dodgeable` or `Parryable`, perfect if `ImpactTime - Window <= t <= ImpactTime + Grace` (defaults: Window 0.12 s, Grace 0.03 s).
- Use world time in seconds, never frame counts. Test at 30/60/120 fps.
- Second check: if the attacker's sweep overlaps the player during IFrames and a perfect judgment is pending, confirm it; if no overlap ever happens, still award (the dodge avoided it) but log for tuning.
- Only one perfect reward per AttackId.

## Rewards (all data-driven)
- Afterimage GameplayCue (pose snapshot of the skeletal mesh, fresnel/dissolve material, 0.4 s life).
- Focus +1 pip, Flux +X, `GE_PerfectDodgeDR` (short damage reduction).
- `State.CounterWindow` for 0.6 s → combo graph routes the next light/heavy to counter moves with bonus poise.
- Enemy micro hit-stop 0.08 s; sound "ting"; subtle desaturation pulse; FOV punch.
- Talent hooks: afterimage burst (AoE), keep charged Focus while dodging.

## Dodge rules
- Directional (8 or 4 + blend), ≤0.45 s, root motion, i-frames on early frames via `IFrames` notify state.
- Cancelable from any attack after its `DodgeCancel` notify; blend 0.05–0.10 s with inertialization.
- **Combo continuity:** ComboComponent keeps the combo index through a dodge for `ComboResumeTime` (default 0.8 s); exit poses match light-attack starts.
- **Diminishing agility:** 4th dodge within 1.5 s uses a slower clip with fewer i-frames; reset when the player attacks or waits.
- Stamina cost per dodge; zero stamina = slow dodge without i-frames (never a full lockout).

## Parry and attack-through counter
- `GA_Parry`: window 0.10 s on Parryable attacks; success = Focus +1, heavy poise damage to attacker, CounterWindow.
- Attack-through counter: heavy pressed during a light string while an attack is within the window → costs 1 Focus, negates the hit (`SuperArmor` + damage immunity for that AttackId), plays counter strike.
- `Unblockable` attacks: parry fails; dodge still works.

## Enemy telegraph standard
Every enemy attack has a DefenseTag and a cue: Dodgeable = white glint, Parryable = gold glint, Unblockable = red glow + audio sting. Telegraph ≥0.3 s before impact for normal enemies.

## Tests (required)
- Functional: scripted attacker; bot dodges at offsets −0.25…+0.05 s in 10 ms steps at 30/60/120 fps; success band = window ±1 frame.
- Combo continuity: L, L, dodge, L → plays light #3.
- Hit-stop during windup still yields correct ImpactTime.
- No double rewards on multi-hit attacks.

## Pitfalls
Frame-count windows; forgetting dilation; awarding perfect dodges against attacks that were never going to reach the player (require target or range check); dodge clip too long (kills smoothness); i-frames that outlast the visual dodge.

## Done when
M3 acceptance in IMPLEMENTATION_PLAN.md passes with a capture and the functional tests green.
