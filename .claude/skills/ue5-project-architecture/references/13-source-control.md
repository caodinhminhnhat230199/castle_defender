# Source Control

## Load When

Use for Git, LFS, binary asset strategy, repository hygiene, or UE project collaboration.

## Git

Track source and project configuration from the beginning.

Use Git LFS for large binary assets when Git is the chosen VCS.

Typical LFS candidates:

- `.uasset`
- `.umap`
- large source art
- large audio
- cinematic media

Project policy may differ.

## Ignore Generated Content

Do not commit generated folders such as build/cache artifacts unless the project has a specific reason.

Common examples include:

- Binaries
- DerivedDataCache
- Intermediate
- Saved

## Binary Asset Collaboration

Because many UE assets are binary:

- establish ownership/locking conventions
- avoid unnecessary concurrent edits
- keep maps modular when team size requires it
- use source art naming conventions

## Rule

Repository strategy must reflect Unreal's binary-heavy workflow.
