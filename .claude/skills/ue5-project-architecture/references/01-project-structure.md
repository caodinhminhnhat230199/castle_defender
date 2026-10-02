# Project Structure

## Load When

Use for Source/Content organization, naming boundaries, or feature-folder questions.

## Principle

Organize primarily by game/domain feature, not by Unreal class type.

Prefer:

```text
Source/<Game>/
  Hero/
  Units/
  Buildings/
  Combat/
  AI/
  Encounter/
  World/
  UI/
  Save/
```

over:

```text
Actors/
Components/
Managers/
Widgets/
Misc/
```

when the project is large enough to benefit from domain grouping.

## Content

Prefer feature/domain grouping with consistent prefixes inside each feature.

Example:

```text
Content/Game/
  Hero/
  Units/
  Buildings/
  Encounters/
  World/
  UI/
```

Do not mirror the entire C++ tree blindly.

## Public / Private

Inside modules:

```text
Source/<Module>/
  Public/
  Private/
```

Use public headers only when other modules need them.

## Rule

Structure should make ownership obvious.

A developer should be able to answer:

- who owns this system?
- what feature does this asset belong to?
- what can depend on it?

Avoid catch-all folders such as `Helpers`, `Utils`, `Managers`, and `Misc` unless narrowly scoped.
