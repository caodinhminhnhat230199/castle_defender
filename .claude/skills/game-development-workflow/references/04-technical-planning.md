# Technical Planning

## Load When

Use after gameplay requirements are understood and before major implementation.

## Purpose

Translate approved behavior into a practical technical approach without turning the plan into line-by-line coding instructions.

## Required Topics

### Technical Overview
Explain the approach in plain language.

### Existing System Impact
Identify only relevant systems.

Examples:

- combat
- AI
- navigation
- input
- UI
- save
- spawning
- progression
- audio/VFX
- world
- networking

### Runtime Flow
Use a flowchart or sequence diagram when useful.

### Data
Identify tunable/configurable information.

### State
Describe important state transitions.

### Error / Failure Behavior
Include meaningful runtime failures.

### Testing
Define the types of verification needed.

### Performance Risks
Identify likely hot paths before implementation.

### Requirement Coverage
Map every approved requirement to a technical area.

## Unreal Projects

If the project uses UE5 and the plan requires decisions such as:

- Actor vs Component
- Subsystem lifetime
- Blueprint vs C++
- GAS
- StateTree
- Asset Manager
- Gameplay Tags
- World Partition
- UE navigation
- module/plugin boundaries

consult `ue5-project-architecture`.

This reference owns the planning structure, not UE5-specific answers.

## Avoid

- speculative frameworks
- new packages without need
- future-proofing unsupported by current scope
- file paths not verified in the repository
- pseudocode that hardens an unapproved design too early
