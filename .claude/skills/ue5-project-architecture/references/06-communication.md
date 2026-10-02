# Communication and Dependencies

## Load When

Use for delegates, interfaces, events, subsystem access, component communication, or reducing coupling.

## Preferred Tools

### Direct Reference
Use when ownership is clear and lifetime is guaranteed.

### Interface
Use when callers need capability without concrete-type coupling.

### Delegate / Event Dispatcher
Use for one-to-many notifications.

### Component
Use to package reusable actor behavior.

### Subsystem
Use for services tied to a well-defined Unreal lifetime.

### Gameplay Message / Event Layer
Use only when the project needs decoupled cross-system messaging at scale.

Do not introduce a global event bus by reflex.

## Dependency Direction

Prefer:

```text
high-level orchestration
→ stable interface/component
→ implementation
```

Avoid bidirectional feature dependencies.

## Polling

Prefer events for state changes.

Avoid per-frame polling when the underlying state changes rarely.

## Rule

Communication should reveal ownership, not hide it.
