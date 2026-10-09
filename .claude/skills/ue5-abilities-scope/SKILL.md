---
name: ue5-abilities-scope
description: Evaluate CastleDefender hero ability and physics proposals against approved Warlord traits, phase gates, shared combat contracts and scope before planning implementation.
---

# CastleDefender ability scope

Adapted from `ue5 ablities telekinesis.md`. The user requested guidance integration while preserving the current game design, not new supernatural abilities. See [integration decisions](../../../ai/game/skill-integration.md).

## Route to approved work

Read [Hero Combat spec](../../../ai/game/01-hero-combat/spec.md) R-CMB-01, 16, 38..41 and NEW-CMB-06; [tasks](../../../ai/game/01-hero-combat/tasks.md) T-CMB-17/18; and [master plan](../../../ai/game/main_implement_plan.md) sections 3, 7, 8a and 10.

- P1 Heavy's Armor Broken integration belongs to SYN's T-SYN-02; it is an existing shared-state integration, not a new ability system.
- Warlord proximity buff (T-CMB-17) and rally / charge / hold-line support design spike (T-CMB-18) remain provisional VS work after the required gates. T-CMB-18 must define rules before implementation.
- Use [Battlefield Synergy](../../../ai/game/04-battlefield-synergy/technical-plan.md) for shared state effects and [Perks](../../../ai/game/11-perks/technical-plan.md) for their approved stat-modifier contracts when those tasks become eligible. Do not add interim competing owners.

## Reusable guidance for an approved future task

Identify the effect's state owner, data source, cancel/interruption behavior, target validation, feedback producer and lifetime before choosing implementation. Reuse components and delegates (D-04/D-07/D-10); every damaging hit uses `DeliverHit` (D-05).

Expose tuning through the task's existing definition/settings pattern. Definitions derive from `UGameDefinition` and are immutable at runtime. Add/change a cross-feature API only with the matching section 8a update.

Any authored action must clean up on cancellation, owner/target death and teardown, and obey the appropriate D-20 clock. Presentation uses `UFeedbackSubsystem`; it cannot confer damage, invulnerability or resource rewards by itself.

For a separately approved physics interaction, first inspect collision ownership, self-hit filtering, target destruction, navigation impact and cleanup. Benchmark a documented gameplay scenario before adopting pooling, destruction limits or a physics stress target. These checks do not authorize adding Chaos assets or dependencies.

## Outside this integration

Telekinetic Hurl, Gravity Well, Surge Slash, Stasis Seal, Flux, weapon forms, ground/air loadouts, boss ability unlocks and Chaos fracture/debris systems are not specified for CastleDefender. Do not create their classes, tags, UI, GAS effects or content. Do not translate them into component equivalents and call that preserving the design.

If the user later explicitly requests one, classify it under master-plan section 10 and record it in the relevant spec section 12. A changed D-xx decision needs a `CHANGE REQUEST` and user decision; an unplanned mechanic needs approved rules/tasks before implementation. `GAME_DESIGN.md`, `IMPLEMENTATION_PLAN.md` and M5 in the upload are not this project's sources of truth.

Completion means the actual approved task is implemented, integrated and verified, with task status and handoff updated. Reviewing this skill does not complete any ability or phase task.
