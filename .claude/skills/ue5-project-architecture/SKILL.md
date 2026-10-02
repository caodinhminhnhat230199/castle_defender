---
name: ue5-project-architecture
description: Modular Unreal Engine 5 project architecture skill. Use for UE5 project structure, modules/plugins, Gameplay Framework, C++/Blueprint boundaries, data-driven design, communication, GAS/Gameplay Tags, AI, UI, world architecture, performance, save/config, source control, and architecture anti-patterns. Pair with game-development-workflow for requirements, planning gates, prototyping, playtesting, QA, and release sequencing.
version: 1.0.0
---

# UE5 Project Architecture

## Purpose

This skill answers:

> How should this be built in Unreal Engine 5?

It owns UE5-specific architecture and implementation rules.

It does **not** own the full game-development process. Use `game-development-workflow` for discovery, specification, production sequencing, task planning, prototyping, playtesting, QA, and release gates.

## Core Principles

1. Inspect the existing project before proposing structural changes.
2. Prefer one game module until a real boundary justifies another module.
3. Use feature/domain-oriented organization over technical junk drawers.
4. Prefer composition over deep inheritance.
5. Keep core runtime behavior in C++ when stability, performance, or reuse matters.
6. Use Blueprint for content assembly, tuning, presentation, and designer iteration.
7. Prefer data-driven content with Primary Data Assets, Data Tables, Gameplay Tags, and the Asset Manager where appropriate.
8. Prefer one-way dependencies and explicit ownership.
9. Prefer event-driven communication over unnecessary polling/Tick.
10. Use soft references for large/content-heavy assets when lifecycle allows.
11. Choose Subsystem type by lifetime, not convenience.
12. Avoid premature GAS, Mass, plugin, module, or framework complexity.
13. Measure before optimizing.
14. Do not invent project files or claim project conventions without inspection.

## Routing

Load only the references relevant to the task.

| Topic | Load |
|---|---|
| folders/content layout | `01-project-structure.md` |
| modules/plugins | `02-modules-plugins.md` |
| GameMode/GameState/Pawn/Controller/etc. | `03-gameplay-framework.md` |
| C++ vs Blueprint | `04-cpp-blueprint.md` |
| PrimaryDataAsset/DataTable/AssetManager | `05-data.md` |
| delegates/interfaces/subsystems/events | `06-communication.md` |
| GAS / Gameplay Tags | `07-gas-gameplay-tags.md` |
| AI / StateTree / BT / navigation | `08-ai.md` |
| UMG / CommonUI / HUD | `09-ui.md` |
| levels / world partition / streaming | `10-world.md` |
| profiling / Tick / pooling / scale | `11-performance.md` |
| SaveGame / config / persistence | `12-save-config.md` |
| Git / LFS / binary assets | `13-source-control.md` |
| architecture review / smells | `14-anti-patterns.md` |

## Cross-Skill Rule

If the request is primarily about:

- feature discovery
- gameplay requirements
- GDD/spec
- production sequencing
- task breakdown
- prototype
- vertical slice
- playtesting
- QA
- release workflow

consult `game-development-workflow`.

Do not copy those workflow responsibilities here.

## Decision Order

For any UE5 architecture question:

1. Identify runtime owner and lifetime.
2. Identify authoritative state.
3. Identify data source.
4. Identify communication path.
5. Identify content/presentation boundary.
6. Identify scale/performance constraints.
7. Identify save/network implications.
8. Fit the solution to existing project conventions.

## Default Biases

When the project gives no contradictory evidence:

```text
C++:
core rules, reusable runtime systems, performance-sensitive loops,
interfaces/contracts, stable state ownership

Blueprint:
content assembly, tuning, UI wiring, animation presentation,
VFX/audio triggers, designer-authored variants
```

Use these as defaults, not dogma.

## Project-Specific Architecture

When the game is large/hybrid, separate conceptual layers even if they remain inside one Unreal module:

```text
Presentation
Gameplay
Simulation
Data
Platform / Persistence
```

Do not create a separate C++ module for every conceptual layer unless build/dependency/team boundaries justify it.

## Output Style

Prefer concrete decisions:

```text
Problem
→ UE5 owner/type
→ data ownership
→ communication
→ lifetime
→ trade-off
→ performance risk
```

Avoid abstract architecture theory without an actionable UE5 mapping.
