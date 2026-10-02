# Build and Release

## Load When

Use when preparing a playable build, milestone, demo, QA package, or release candidate.

## Build Gate

Verify:

- correct configuration
- required maps/scenes included
- assets resolve
- no development-only dependency blocks packaging
- save path works
- startup flow works
- restart/quit works
- target input devices work
- target hardware is tested
- crash logging is available when required

## Release Candidate Rule

Do not call a local editor run a release-ready build.

Test the packaged build.

## Milestone Notes

Record:

- build identifier
- source revision
- known issues
- supported platforms
- test scenario
- save compatibility
- major performance notes

## Rollback

For production projects, know how to:

- identify last good build
- reproduce build inputs
- revert a bad change
- protect save compatibility

Engine-specific packaging rules belong in the engine architecture/release tooling skill.
