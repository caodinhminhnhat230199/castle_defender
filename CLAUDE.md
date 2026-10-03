# CLAUDE.md

@AGENTS.md

## Claude Code Only

All rules above apply. These notes cover what only Claude Code has.

- **Skills** in `.claude/skills/`:
  - `game-development-workflow`: planning, prototypes, playtests, QA, gates.
  - `ue5-project-architecture`: any UE5 design choice.
  - `source-driven-development`: check Unreal APIs against the official docs.
  - `debugging-and-error-recovery`: build errors, crashes, bugs.
  - `code-review-and-quality`: run before moving a task to `Review`.
  - `documentation-and-adrs`: `CHANGE REQUEST: D-xx` write-ups.
  - `planning-with-files`: work that spans sessions.
  - `git-workflow-and-versioning`: branches and commits.
  - `idea-refine`: new ideas, before they enter scope.
  - `stop-slop`: writing docs.
- **Plugins:** `caveman` (terse replies) and `ponytail` (minimal code) are enabled in `.claude/settings.json`. They change style only and never skip the verification, docs and handoff rules.
- **CodeGraph:** when `.codegraph/` exists, use the `codegraph_explore` MCP tool (or `codegraph explore "<query>"`) before grep or reading files.
- **Subagents:** give them the task ID and tell them to follow `AGENTS.md`. Check their output against the task's acceptance criteria before marking the task done.
