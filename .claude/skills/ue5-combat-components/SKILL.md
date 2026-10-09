---
name: ue5-combat-components
description: Implement and review CastleDefender melee combat, montage windows, input buffering, hit detection, poise and impact routing using the existing component architecture.
---

# CastleDefender combat

Adapted from the user upload `ue5 combat gas.md`. The user confirmed on 2026-10-09 that integration must preserve the current game design. Read [integration decisions](../../../ai/game/skill-integration.md) when comparing with the original upload.

## Load for the task

- [Hero Combat spec](../../../ai/game/01-hero-combat/spec.md): R-CMB-07..18, 43..54 and the task's ACs.
- [Hero Combat technical plan](../../../ai/game/01-hero-combat/technical-plan.md): timing, resolution and clock domains.
- [Master plan](../../../ai/game/main_implement_plan.md): D-04, D-05, D-07, D-10, D-19, D-20 and consumed/provided section 8a contracts.

## Existing owners

Inspect `Source/CastleDefender/Hero/HeroCombatComponent.*`, `HeroClassDefinition.*`, `StaminaComponent.*`, and `Source/CastleDefender/Combat/{CombatLibrary,MeleeTraceComponent,CombatStateComponent,CombatActionTiming}.*` before editing.

- `UHeroCombatComponent` owns action state, Light chain, latest buffered action, defensive windows and counter lifetime.
- `UHeroClassDefinition : UGameDefinition` owns non-timing tuning; montage notifies own exact action windows.
- `UMeleeTraceComponent` owns open-window sweeps and the per-swing target set.
- `UCombatLibrary::DeliverHit` resolves every hit. `UHealthComponent` owns HP; `UCombatStateComponent` owns poise and shared states.
- `UFeedbackSubsystem` presents selected outcomes; gameplay resolution remains observable independently of feedback playback/throttling.

## Apply the useful guidance

1. Keep Startup / Active / Recovery and applicable hit, chain/cancel, invulnerability, parry, rotation-assist and interrupt-resistance windows authored in montages. Reuse existing notify types; do not introduce parallel timers or per-instance state on shared notify assets.
2. Validate data before spending stamina or entering an action. Missing/invalid montage windows must refuse the action with an actionable diagnostic, without leaving partial state.
3. Sweep previous-to-current weapon sockets only during hit windows. Deduplicate each target per swing. Request physical materials and carry the actual surface through the shared hit context.
4. Treat callbacks as reentrant: a resolved hit may interrupt, destroy an actor, or close/reopen the trace window. Stop processing the old swing if its context changed.
5. Keep only the latest buffered press (R-CMB-08). Buffer/counter deadlines use hero dilated time, preserving input through actor hit stop and Tactical Focus.
6. Close windows and clear stale trace/buffer/assist state on interruption, stagger and death. Never duplicate SYN's Staggered lifetime.
7. Preserve the approved three-hit Light chain and stamina Heavy. Starting Dodge or another action resets the chain; no chord detection or dodge/parry priority queue replaces the current buffer.
8. Use existing `game.debug.Combat`, `game.debug.CombatTrace` and `ReportHeroWindows` diagnostics. Keep debug behavior out of Shipping.

## Boundaries

GAS, `GA_*`, `GE_*`, AttributeSets, combo graphs, Focus/Flux resources, charged/varied heavies, aerials, weapon-form swaps and executions are outside the approved design. D-04 allows GAS re-evaluation at G3 only; a skill does not authorize it. Do not substitute the uploaded damage calculation for `DeliverHit`.

## Verification

Use the owning task's tests and PIE criteria. Relevant existing coverage includes hit deduplication/reentrant sweeps, notify validation, combo reset, stamina refusal, consumed Parry, counter payload and clock behavior under hit stop. Use `Tools/build.bat` and `Tools/run_tests.bat`; use a focused filter only after checking the real test names. Saved playable montages and actual input in `L_CombatSandbox` remain required for gameplay changes. Do not treat adapting this guide as runtime verification.
