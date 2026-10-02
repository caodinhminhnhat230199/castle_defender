# Prototyping

## Load When

Use for uncertain, expensive, novel, or high-risk gameplay ideas.

## Purpose

Answer one important question as cheaply as possible.

A prototype is an experiment, not a mini-production system.

## Prototype Hypothesis

Write:

```text
Hypothesis:
If we build X, players/system will demonstrate Y.

Evidence needed:
Z.
```

## Good Prototype Scope

Prefer:

- one map
- one enemy
- one ability
- one tower
- one encounter
- temporary art
- debug UI

Avoid building supporting systems that are not needed to test the hypothesis.

## Exit Criteria

Before starting, define:

- KEEP
- CHANGE
- DELETE

based on observable evidence.

## Technical Debt Rule

Prototype shortcuts are allowed when explicitly labeled.

Before productionizing:

- remove hacks that affect maintainability
- normalize data ownership
- add error handling
- add tests
- validate performance
- align with project architecture

Do not silently promote prototype code into production architecture.
