# World and Level Architecture

## Load When

Use for maps, streaming, World Partition, sub-levels, persistent world systems, spawn ownership, or controlled open worlds.

## Level Strategy

Choose based on project scale.

### Separate Levels
Simple and explicit for isolated arenas/runs.

### Level Streaming
Useful for authored chunks and controlled transitions.

### World Partition
Useful for large worlds needing spatial streaming and editor collaboration.

Do not choose World Partition only because it is modern.

## Persistent Systems

Do not keep long-lived systems alive through hidden level actors unless their lifetime truly belongs to the world.

Use the correct subsystem/GameInstance ownership when appropriate.

## Runtime Spawned Content

Define ownership and cleanup.

A spawned actor should have an obvious reason to exist and a clear destruction/despawn policy.

## Navigation Changes

If runtime buildings alter paths:

- define whether navigation rebuilds
- define blocked-path policy
- define placement validation
- define fallback behavior

For high-frequency strategic path changes, a custom strategic navigation layer may be preferable to constantly forcing expensive NavMesh rebuilds.

## Rule

World architecture must follow traversal scale, streaming need, and runtime mutation.
