# Save and Configuration

## Load When

Use for SaveGame, persistent progression, settings, config files, versioning, or migration.

## Separate Data Categories

```text
runtime transient state
run/session state
meta progression
user settings
developer/config defaults
```

Do not store all categories in one giant save blob without boundaries.

## SaveGame

Use typed, versioned save structures.

Include a save version.

Plan migration when shipped saves must survive schema changes.

## IDs

Persist stable semantic IDs, not fragile direct UObject pointers.

Gameplay Tags, names, GUIDs, or Primary Asset IDs may be appropriate depending on the domain.

## Settings

Use Unreal config/settings systems for user/developer configuration rather than SaveGame when the data is truly configuration.

## Robustness

Define:

- write timing
- atomicity strategy
- corruption handling
- missing-data defaults
- migration behavior

## Rule

Persistence is a data-contract problem, not just serialization.
