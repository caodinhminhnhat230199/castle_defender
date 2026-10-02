# Gameplay Framework

## Load When

Use for GameMode, GameState, PlayerState, Controller, Pawn/Character, Components, and Subsystems.

## Ownership by Meaning

### GameMode
Server/authority-only rules for a game mode or match.

Do not store client-visible authoritative shared state only here.

### GameState
Replicated/shared match state visible to clients.

### PlayerState
Persistent player state across pawn possession within a match/session.

### PlayerController
Player intent, input interpretation, UI interaction, possession-level orchestration.

Avoid putting all gameplay logic here.

### Pawn / Character
World representation that can be possessed/controlled.

### ActorComponent
Reusable behavior/state owned by an Actor.

Use composition to avoid deep inheritance.

### UObject
Lightweight non-Actor behavior/data objects without world transform/Actor lifecycle needs.

### Subsystems
Choose by lifetime:

- EngineSubsystem
- GameInstanceSubsystem
- WorldSubsystem
- LocalPlayerSubsystem

Do not use Subsystems as global dumping grounds.

## Decision Questions

1. Who owns the state?
2. What lifetime should it have?
3. Does it need a world transform?
4. Does it need replication?
5. Is it reusable across actors?
6. Is it player-local, world-local, or process-wide?

Choose the smallest correct owner.
