# Data Architecture

## Load When

Use for tunable gameplay data, PrimaryDataAsset, DataTable, DataRegistry, Asset Manager, soft references, or content definitions.

## Data-Driven Bias

Data that designers tune often should not require code changes.

Examples:

- unit stats
- enemy definitions
- tower definitions
- ability tuning
- wave definitions
- loot tables
- encounter data
- progression costs

## Primary Data Asset

Prefer for rich typed content definitions with asset references and inheritance/authoring needs.

Example conceptual definitions:

```text
EnemyDefinition
UnitDefinition
BuildingDefinition
AbilityDefinition
EncounterDefinition
```

## Data Table

Prefer for flat row-oriented data with stable schemas and bulk authoring/import needs.

## Asset Manager

Use when you need:

- primary asset identity
- controlled loading
- bundles
- soft-reference lifecycle
- large content catalogs

## Soft References

Prefer soft references for large or optional content that should not be transitively loaded.

Do not use soft references everywhere. Hard references are fine when lifecycle matches.

## Rule

Separate:

```text
definition data
runtime state
presentation asset
```

Do not mutate shared definition assets as runtime state.
