# Implementation Loop

## Load When

Use during actual development.

## Loop

```text
Select Task
→ Inspect Existing Project
→ Make Smallest Correct Change
→ Build
→ Test
→ Playtest if Gameplay
→ Profile if Performance-Sensitive
→ Fix
→ Verify Acceptance Criteria
→ Mark Complete
```

## Before Editing

Inspect:

- relevant code
- call sites
- data definitions
- assets
- scenes/levels
- naming conventions
- existing reusable systems

Do not create a parallel architecture because the current one was not inspected.

## Change Discipline

Prefer:

- small diffs
- local changes
- existing abstractions
- data-driven extensions
- explicit ownership

Avoid:

- unrelated refactors
- speculative architecture
- duplicate managers
- silent design changes

## Verification

After a major task:

- build succeeds
- no broken references
- no obvious runtime errors
- expected behavior works
- important edge cases work
- no known regression
- performance remains acceptable

Use the UE5 architecture skill for Unreal-specific implementation choices.
