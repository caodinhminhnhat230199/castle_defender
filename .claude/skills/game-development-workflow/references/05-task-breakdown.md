# Task Breakdown

## Load When

Use when converting approved plans into executable work.

## Task Design

Each task should be independently understandable and verifiable.

Use:

```text
TASK-ID
Title
Objective
Related Requirements
Dependencies
Implementation Steps
Expected Files/Assets
Acceptance Criteria
Verification
```

## Good Task Size

A task should usually:

- create one meaningful outcome
- be reviewable without unrelated changes
- have clear dependencies
- include its own verification

Avoid vague tasks such as:

- Implement AI
- Finish combat
- Polish everything

## Task Types

Useful prefixes:

```text
DESIGN-
ART-
ANIM-
VFX-
AUDIO-
GAMEPLAY-
AI-
UI-
TOOLS-
PERF-
QA-
BUILD-
```

## Dependency Graph

Use Mermaid when order matters.

Identify:

- sequential tasks
- parallel tasks
- blockers
- external dependencies

## Testing Work

Do not hide testing inside "done".

Create explicit verification steps or QA tasks when risk is meaningful.

## Definition of Ready

A task is ready when:

- objective is clear
- dependencies are known
- acceptance criteria are testable
- required input/assets are available or explicitly mocked
- no major design decision remains hidden inside the task
