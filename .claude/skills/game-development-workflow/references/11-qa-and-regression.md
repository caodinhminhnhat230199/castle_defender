# QA and Regression

## Load When

Use before integration, milestone completion, or release.

## Test Layers

Choose what fits:

- unit tests
- component/system tests
- gameplay automation
- integration tests
- manual gameplay tests
- platform checks
- save compatibility
- performance regression
- soak tests

## Regression Matrix

Focus on affected neighboring systems.

Example columns:

| Area | Expected | Result | Notes |
|---|---|---|---|
| new feature | works | | |
| old flow | unchanged | | |
| save/load | compatible | | |
| restart | clean | | |
| input | stable | | |

## Severity

Keep a simple model:

- Blocker
- Critical
- Major
- Minor

## Release Blocking

A feature should not pass QA with:

- crashes in normal flow
- progression blockers
- corrupted saves
- deterministic soft locks
- severe input loss
- known major performance regressions

Project-specific policy may be stricter.
