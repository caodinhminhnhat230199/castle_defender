---
name: game-development-workflow
description: End-to-end game development workflow router. Use for requirement discovery, gameplay specification, production planning, technical planning, task breakdown, prototyping, vertical slices, implementation sequencing, playtesting, profiling, QA, release, scope control, and quality gates. For Unreal Engine-specific architecture decisions, consult the ue5-project-architecture skill when available.
version: 1.0.0
---

# Game Development Workflow

## Purpose

This skill answers:

> What should we do, in what order, and how do we know it is ready?

It is an orchestration skill for game development. It does **not** own engine-specific architecture rules.

Use progressive disclosure: load only the reference files required by the current task.

## Core Principles

1. Gameplay value before architecture purity.
2. Small playable slices before content scale.
3. Validate uncertain ideas through prototypes.
4. Use data-driven tuning when designers are expected to iterate.
5. Keep scope explicit and protect anti-goals.
6. Prefer small, testable implementation steps.
7. Inspect the existing project before proposing project-specific paths or systems.
8. Do not invent files, classes, APIs, assets, or project conventions.
9. Profile measurable performance problems before optimizing.
10. A feature is not complete until it works in a playable build.

## Responsibilities

This skill owns:

- requirement discovery
- gameplay specification
- production sequencing
- technical planning workflow
- implementation task breakdown
- prototype decisions
- vertical-slice gates
- implementation sequencing
- playtesting
- profiling workflow
- QA and regression
- build and release gates
- scope control
- definition of done

This skill does **not** own:

- Unreal class selection
- UE5 module architecture
- Actor vs Component vs Subsystem decisions
- Blueprint vs C++ implementation rules
- GAS architecture
- StateTree / Behavior Tree architecture
- World Partition architecture
- UE-specific Asset Manager rules
- UE-specific performance architecture

For those decisions, use `ue5-project-architecture` when available.

## Routing

Load the minimum references required.

| Request | Load |
|---|---|
| unclear idea / new feature | `01-discovery-and-requirements.md`, `02-game-specification.md` |
| gameplay spec / GDD section | `02-game-specification.md`, `13-scope-control.md` |
| art/content requirements | `03-production-planning.md` |
| technical plan | `04-technical-planning.md`, `14-quality-gates.md` |
| implementation tasks | `05-task-breakdown.md` |
| risky/unproven mechanic | `06-prototyping.md` |
| first complete playable slice | `07-vertical-slice.md` |
| coding/implementation execution | `08-implementation-loop.md`, `14-quality-gates.md` |
| gameplay feel / balance | `09-playtesting.md` |
| FPS / spikes / scale | `10-performance-and-profiling.md` |
| test / regression | `11-qa-and-regression.md` |
| packaging / release | `12-build-and-release.md` |
| scope creep / new ideas | `13-scope-control.md` |
| readiness review | `14-quality-gates.md` |

## Recommended Documentation Chain

For substantial features, use:

```text
ai/game/<feature-or-system>/
├── spec.md
├── production-plan.md
├── technical-plan.md
└── tasks.md
```

Use templates from `templates/`.

Do not force all four documents for trivial work.

## Workflow

```text
Discover
→ Specify
→ Plan Production
→ Plan Technical Approach
→ Break Down Tasks
→ Prototype if Risky
→ Build Vertical Slice
→ Implement
→ Playtest
→ Profile
→ QA
→ Release
```

A phase can loop back when evidence shows the design or implementation is wrong.

## Cross-Skill Rule

When the project uses Unreal Engine and the current work requires engine-specific architecture:

1. Stay in this workflow skill for process and gates.
2. Consult `ue5-project-architecture`.
3. Load only the relevant UE5 references.
4. Return to this workflow for task sequencing, verification, and release readiness.

Do not duplicate UE5 architecture rules inside this skill.

## Project Awareness

When repository access exists:

- inspect current code and assets before planning
- reuse existing architecture and naming
- identify engine version and target platforms
- identify current gameplay/data patterns
- identify build constraints
- mark unverified assumptions explicitly

When repository access does not exist:

- separate facts from proposals
- do not claim paths/components exist
- keep project-specific details generic

## Output Style

Use clear, practical English.

Prefer:

- short tables
- checklists
- state diagrams
- flowcharts
- dependency graphs
- acceptance criteria

Avoid:

- theory dumps
- speculative abstraction
- repeated explanations
- documentation that does not change a decision

## Main Rule

Optimize in this order:

**Gameplay Quality → Technical Feasibility → Performance → Development Speed → Maintainability → Scalability**

The goal is a game that is playable, stable, performant, maintainable, and realistically shippable.
