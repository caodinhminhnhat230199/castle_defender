# GAS and Gameplay Tags

## Load When

Use for Gameplay Ability System, attributes, effects, abilities, cooldowns, status effects, or semantic tags.

## GAS Fit

GAS is strong for:

- hero abilities
- boss abilities
- complex cooldown/cost rules
- status effects
- attribute modification
- multiplayer-ready combat semantics

GAS can be excessive for hundreds of simple low-level units.

Use lightweight components/data when units do not need the full ability/effect model.

## Gameplay Tags

Use hierarchical semantic tags for:

- state
- ability categories
- damage types
- unit categories
- targeting rules
- effect categories

Examples:

```text
State.Stunned
State.Dead
Damage.Physical
Unit.Melee
Ability.WarCry
Effect.Buff.Attack
```

## Rules

- define tag taxonomy centrally
- avoid free-form string logic
- avoid tag explosion
- tags describe semantics; they are not a replacement for all state variables

## Attributes

Keep the attribute set focused on values needing GAS semantics.

Do not put every gameplay number into an AttributeSet.
