# Progress Log

Newest entry first. Every agent session adds one entry (rules: `AGENTS.md` §9).

## Entry Template

```markdown
### YYYY-MM-DD: <agent>: <short title>
- **Tasks:** T-XXX-NN (Todo → In Progress / Review / Done / Blocked)
- **Changed:** files and assets
- **Verified:** commands and tests run, with results; PIE checks
- **Manual steps for the user:** editor work the agent could not do
- **Open questions / blockers:**
- **Next:** next task ID
```

---

### 2026-10-03: Codex: shared instructions for three tools
- **Tasks:** documentation only; none started (Phase F not started).
- **Changed:** `AGENTS.md` sections 0 and 11; this log.
- **Verified:** checked the existing `CLAUDE.md` import and Antigravity's official rule-loading documentation; reviewed the additions and checked Markdown references and whitespace. Unreal tests and PIE are not applicable to this documentation change. Instruction loading in Antigravity and Claude Code has not been exercised in this session.
- **Manual steps for the user:** if the installed Antigravity version does not load `AGENTS.md`, explicitly attach or request a read of it at session start.
- **Open questions / blockers:** none for this change; Q-15 remains open for T-FND-01.
- **Next:** T-FND-01, after Q-15 is answered.

### 2026-10-03: Claude Code: planning complete, agent rules added
- **Tasks:** none started (Phase F not started).
- **Changed:**
  - `ai/game/` planning docs: 19 features, 271 tasks.
  - `ai/game/spec-audit.md`.
  - `AGENTS.md`, `CLAUDE.md`, this log.
- **Verified:** the doc cross-reference script reports no missing IDs and no dependency cycles, and every rule and acceptance criterion maps to a task.
- **Manual steps for the user:** answer Q-15 (project name, UE version, reference PC).
- **Open questions / blockers:** the decisions in `ai/game/spec-audit.md` §8.
- **Next:** T-FND-01.
