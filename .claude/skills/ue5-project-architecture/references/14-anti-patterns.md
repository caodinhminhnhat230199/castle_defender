# UE5 Architecture Anti-Patterns

## Load When

Use during design review, refactoring, or architecture evaluation.

## Common Smells

### God Manager
One global manager owns unrelated systems.

Prefer correct Unreal ownership/lifetimes.

### Subsystem as Service Locator
Everything asks a global subsystem for everything.

Use explicit ownership and narrower services.

### Deep Blueprint Inheritance
Many Blueprint children duplicate fragile event graphs.

Move stable shared behavior to C++/components/data.

### Tick Everywhere
Every Actor polls state every frame.

Use events, timers, batching, significance, or scheduling.

### Hard-Reference Web
A persistent asset indirectly loads huge content trees.

Use Asset Manager/soft references where lifecycle requires.

### GAS Everywhere
Simple mass units pay complexity for features they do not use.

Use lightweight runtime models where appropriate.

### Module Per Feature
Build graph becomes more complex than the game.

Split only at real dependency/team/runtime boundaries.

### Actor for Every Piece of Data
Pure data/state becomes expensive world objects.

Use UObjects/structs/data assets where world identity is unnecessary.

### Widget Owns Gameplay
UI becomes authoritative state.

Keep UI in presentation role.

### Blueprint-Only Core
Critical high-scale runtime logic becomes hard to test/profile/maintain.

Move stable core logic to C++ when justified.

## Review Rule

Every abstraction must answer:

> What concrete problem does this solve in the current project?
