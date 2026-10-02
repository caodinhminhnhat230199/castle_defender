# Modules and Plugins

## Load When

Use when deciding whether to split code into Unreal modules or plugins.

## Default

Start with one game runtime module unless there is a real reason to split.

Premature module splitting adds:

- dependency management
- build complexity
- include friction
- initialization/lifetime complexity

## Create a Module When

A boundary is justified by one or more:

- clear dependency direction
- independent build/runtime responsibility
- large team ownership boundary
- editor-only code separation
- reusable platform/service layer
- compile-time isolation with measurable value

## Create a Plugin When

The capability is:

- reusable across projects
- optional
- independently versioned
- editor tooling with independent lifecycle
- third-party-like in ownership

Do not convert ordinary game features into plugins by default.

## Dependency Rule

Prefer one-way dependency graphs.

Avoid circular dependencies between gameplay domains.

Use interfaces/events/data contracts instead of adding reverse module dependencies.

## Common Split

For larger projects:

```text
<Game>          runtime gameplay
<Game>Editor    editor-only tooling
```

Add more only when evidence justifies it.
