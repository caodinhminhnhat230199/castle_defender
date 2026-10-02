# Performance and Profiling

## Load When

Use for FPS drops, spikes, large unit counts, heavy VFX, streaming issues, memory pressure, or performance-sensitive features.

## Rule

Profile before optimizing.

Do not optimize based only on intuition.

## Test a Representative Worst Case

Examples:

- maximum expected enemies
- dense combat
- maximum projectiles
- heavy VFX
- crowded UI
- large navigation updates
- streaming transition
- large save/load
- long session memory behavior

## Classify the Problem

- CPU
- GPU
- memory
- I/O
- streaming
- network
- frame pacing
- loading
- garbage/allocation pressure

## Workflow

```text
Reproduce
→ Capture Baseline
→ Identify Hotspot
→ Form Hypothesis
→ Change One Meaningful Variable
→ Measure Again
→ Compare
→ Keep or Revert
```

## Record

- hardware
- build configuration
- scene/scenario
- baseline
- change
- result
- regression risk

For UE5-specific profiling tools and runtime architecture, consult `ue5-project-architecture/references/11-performance.md`.
