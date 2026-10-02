# C++ and Blueprint Boundary

## Load When

Use when deciding where logic should live.

## Recommended Split

### Prefer C++ For

- core gameplay rules
- stable contracts/interfaces
- performance-sensitive loops
- reusable components
- low-level systems
- authoritative state
- complex testable logic
- foundational data types

### Prefer Blueprint For

- content composition
- tuning
- designer iteration
- UI composition
- animation state wiring
- VFX/audio presentation
- content variants
- simple orchestration

## Pattern

```text
C++ base capability
→ Blueprint child/content asset
→ designer tuning
```

Avoid large Blueprint inheritance chains that duplicate logic.

## Blueprint Safety

Keep Blueprint APIs:

- narrow
- named clearly
- data-oriented
- hard to misuse

Use `BlueprintReadOnly` unless mutation is intentionally part of the contract.

## Rule

Do not move code to C++ solely because "C++ is better".

Do not keep critical high-scale runtime loops in Blueprint solely because it is faster to author.

Use project needs and measured cost.
