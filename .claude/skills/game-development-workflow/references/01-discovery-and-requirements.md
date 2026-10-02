# Discovery and Requirements

## Load When

Use for new features, unclear ideas, GDD gaps, Jira tickets, or ambiguous gameplay requests.

## Goal

Convert an idea into a bounded, testable problem without inventing missing product decisions.

## Minimum Context

Capture when available:

- feature or mechanic description
- player goal
- target platform
- game mode
- affected systems
- engine
- references
- constraints
- expected scale
- known anti-goals

Do not block progress on optional information. Record assumptions instead.

## Discovery Questions

Answer from supplied materials first.

1. What player problem or gameplay opportunity exists?
2. What does the player do?
3. What changes in game state?
4. What feedback does the player receive?
5. What is the success condition?
6. What is the failure condition?
7. What existing systems does this touch?
8. What scale must it support?
9. What is explicitly out of scope?
10. What is uncertain enough to prototype?

## Requirement IDs

Use stable IDs for non-trivial features:

```text
GAME-01
GAME-02
NFR-01
```

Requirements must describe observable behavior, not implementation.

Good:

> GAME-03: Enemies blocked by a defensive structure stop and attack it before continuing.

Bad:

> GAME-03: Use StateTree Task X with Component Y.

## Assumptions

Mark assumptions clearly:

```text
ASSUMPTION: The feature is single-player only.
```

Do not turn assumptions into facts.

## Output

Discovery is complete when there is enough information to write a concise `spec.md` or create a prototype hypothesis.
