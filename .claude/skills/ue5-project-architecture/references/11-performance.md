# Performance Architecture

## Load When

Use for frame time, large unit counts, Tick cost, spawning, pooling, memory, asset load, navigation, or profiling.

## Principle

Measure first.

Use:

- Unreal Insights
- stat commands
- CPU/GPU profiler
- Memory Insights
- RenderDoc when appropriate
- platform tooling

## Avoid Unnecessary Tick

Ask:

- does this need every frame?
- can an event trigger it?
- can it update at lower frequency?
- can it be centrally scheduled?

## High Unit Counts

Watch:

- Actor count
- component count
- animation cost
- skeletal mesh cost
- perception queries
- navigation
- overlap events
- replication if multiplayer
- per-frame allocations

## Pooling

Use pooling when spawn/despawn churn is a measured problem.

Pooling adds lifecycle complexity. Do not make it the default for every actor type.

## Assets

Use soft references and async loading when large optional content should not load eagerly.

## Performance Budget

Record budget by real target hardware and scenario.

Do not claim a system is performant because the editor seems smooth.
