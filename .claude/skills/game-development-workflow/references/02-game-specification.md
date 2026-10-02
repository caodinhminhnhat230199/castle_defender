# Game Specification

## Load When

Use when defining what a mechanic/system should do from the player's perspective.

## Purpose

Specify **WHAT** is built and **WHY** it exists.

Do not describe low-level architecture.

## Recommended Structure

### 1. Overview
Short explanation of the system and player value.

### 2. Player Experience
Describe intended feel and decision pressure.

Examples:

- tactical control
- escalating tension
- heavy combat
- risk versus reward
- readable feedback

### 3. Core Loop
Use a short loop.

```text
Observe
→ Decide
→ Act
→ Receive Feedback
→ Adapt
```

### 4. Gameplay Rules
Give major rules stable IDs.

### 5. Player Actions
List meaningful actions, not button mappings unless input itself is part of the design.

### 6. Success / Failure
Define completion, failure, retry, and partial-success behavior when relevant.

### 7. Scope
Separate:
- In Scope
- Out of Scope

### 8. Anti-Goals
Mandatory for large systems.

Anti-goals prevent accidental transformation into a different game.

### 9. Dependencies
List gameplay/system dependencies.

### 10. Edge Cases
Only meaningful cases that can alter behavior.

### 11. Acceptance Criteria
Player-observable, testable outcomes.

### 12. Open Questions
Do not invent missing design.

## Rules

- no implementation code
- no invented technical paths
- no hidden scope expansion
- keep terminology consistent
- every technical plan must trace back to approved gameplay requirements
