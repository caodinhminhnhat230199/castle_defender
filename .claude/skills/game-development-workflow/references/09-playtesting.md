# Playtesting

## Load When

Use when the feature technically works but gameplay quality, readability, pacing, or balance needs validation.

## Separate Two Problem Types

### Design Problem
The implementation works as specified, but the experience is weak.

### Technical Bug
The implementation does not behave as intended.

Do not mix them.

## What to Observe

Depending on the feature:

- player understanding
- input responsiveness
- readability
- feedback timing
- decision quality
- difficulty
- pacing
- frustration
- boredom
- encounter duration
- resource pressure
- visual noise
- audio clarity

## Test Structure

Record:

```text
Build / Version
Scenario
Tester
Expected Experience
Observed Behavior
Issue Type
Severity
Decision
```

## Avoid

- changing five balance variables at once
- treating one anecdote as proof
- polishing visuals to hide a weak mechanic
- using telemetry without a clear question

## Outcome

Every playtest should end with a decision:

- keep
- tune
- redesign
- remove
- test again
