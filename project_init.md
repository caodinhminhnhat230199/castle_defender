# Project Init — Game (UE5)

This file sets up the Claude Code skills a new member needs for this project.

**How to run:** open Claude Code at the project root and say:

```
execute project_init.md
```

Claude reads this file and follows the steps below. You can also run the script in step 2 by hand.

---

## Instructions for the agent

Run every step from the project root. Do not touch global skills in `~/.claude`. Do not overwrite a skill that is already in `.claude/skills/`. Skip it and report it as already installed.

### 1. Requirements

Check that these tools exist: `git`, `unzip`. If one is missing, stop and tell the user what to install.

The 2 internal skills (`game-development-workflow`, `ue5-project-architecture`) normally come with the clone in `.claude/skills/`. Only when one of them is missing, check for its zip at the project root:

- `game-development-workflow-skill.zip`
- `ue5-project-architecture-skill.zip`

If both the skill folder and the zip are missing, stop and ask the user to get the zip from the lead.

### 2. Install skills into `.claude/skills/`

```bash
set -euo pipefail
ROOT="$(pwd)"
DEST="$ROOT/.claude/skills"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$DEST"

install_dir() { # $1 = skill name, $2 = source directory
  if [ -d "$DEST/$1" ]; then echo "SKIP  $1 (already installed)"; else cp -R "$2" "$DEST/$1" && echo "OK    $1"; fi
}

# Internal skills (zip files at the project root)
for z in game-development-workflow ue5-project-architecture; do
  if [ -d "$DEST/$z" ]; then echo "SKIP  $z (already installed)"; else unzip -q "$ROOT/$z-skill.zip" -d "$DEST" && echo "OK    $z"; fi
done

# addyosmani/agent-skills: install only the skills this project needs
git clone -q --depth 1 https://github.com/addyosmani/agent-skills.git "$TMP/agent-skills"
for s in code-review-and-quality debugging-and-error-recovery documentation-and-adrs \
         git-workflow-and-versioning idea-refine source-driven-development; do
  install_dir "$s" "$TMP/agent-skills/skills/$s"
done

# othmanadi/planning-with-files
git clone -q --depth 1 https://github.com/othmanadi/planning-with-files.git "$TMP/pwf"
install_dir planning-with-files "$TMP/pwf/skills/planning-with-files"

# hardikpandya/stop-slop (SKILL.md sits at the repo root)
git clone -q --depth 1 https://github.com/hardikpandya/stop-slop.git "$TMP/stop-slop"
rm -rf "$TMP/stop-slop/.git"
install_dir stop-slop "$TMP/stop-slop"
```

### 3. Plugins (caveman, ponytail)

These are declared in `.claude/settings.json` (`extraKnownMarketplaces` + `enabledPlugins`). Claude Code offers to install them the first time the user trusts the project folder. If they are not installed yet and the `claude` CLI is available, run the commands below. If the CLI is not available (for example in the desktop app), tell the user to open a `claude` terminal and use `/plugin` to install `caveman@caveman` and `ponytail@ponytail`.

```bash
claude plugin marketplace add juliusbrussee/caveman
claude plugin marketplace add dietrichgebert/ponytail
claude plugin install caveman@caveman --scope project
claude plugin install ponytail@ponytail --scope project
```

### 4. CodeGraph (code intelligence)

[CodeGraph](https://github.com/colbymchenry/codegraph) indexes the project's C++ code so the agent can find symbols, callers/callees and blast radius without endless grep/read loops. The CLI and MCP server are installed **globally** (once per machine). The index (`.codegraph/`) is **per project** and stays local. Never commit it.

1. **CLI.** If `codegraph --version` works, skip the install (run `codegraph upgrade --check` to see whether an update exists). Otherwise **ask the user first**, because this downloads and runs the official installer, then run one of:

   ```bash
   curl -fsSL https://raw.githubusercontent.com/colbymchenry/codegraph/main/install.sh | sh
   # or, when Node is installed:
   npm i -g @colbymchenry/codegraph
   ```

   The installer does not update the current shell. Open a new terminal, or call the binary by its full path.

2. **Wire it into Claude Code.** Skip this step if `~/.claude.json` already has `mcpServers.codegraph`. Do not register it twice. Otherwise:

   ```bash
   codegraph install --target claude --location global
   ```

3. **Telemetry.** Keep the user's existing choice. If there is none, ask the user. Default to turning it off:

   ```bash
   codegraph telemetry off
   ```

4. **Index the project.** Run this only once the project has source code (a `Source/` folder with `.h`/`.cpp` files). Skip it if `codegraph status` shows a healthy index. Never re-init a healthy index.

   ```bash
   codegraph init --yes
   ```

   CodeGraph honors `.gitignore`. Make sure `.gitignore` has the UE5 generated folders (`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`) and `.codegraph/`, so the graph only contains the team's code.

### 5. Verify

```bash
codegraph --version && grep -q '"codegraph"' ~/.claude.json && echo "PASS  codegraph (CLI + MCP)" || echo "FAIL  codegraph"
for s in game-development-workflow ue5-project-architecture code-review-and-quality \
         debugging-and-error-recovery documentation-and-adrs git-workflow-and-versioning \
         idea-refine source-driven-development planning-with-files stop-slop; do
  f=".claude/skills/$s/SKILL.md"
  if [ -f "$f" ] && grep -q "^name: $s" "$f"; then echo "PASS  $s"; else echo "FAIL  $s"; fi
done
```

All 11 lines must be `PASS`. If the project already has code, also run `codegraph status`; it must show an initialized index. Report the result to the user and remind them to **start a new Claude Code session** so the new skills load.

---

## Skill list

| Skill | Source | Use for |
|---|---|---|
| `game-development-workflow` | internal zip | Requirements, spec, planning, prototyping, vertical slice, playtesting, QA, release, quality gates |
| `ue5-project-architecture` | internal zip | UE5 architecture: modules/plugins, Gameplay Framework, C++/Blueprint, GAS, AI, UI, world, performance, save |
| `idea-refine` | addyosmani/agent-skills | Brainstorming and refining game ideas and mechanics |
| `source-driven-development` | addyosmani/agent-skills | Checking work against official UE5 docs (engine APIs change often) |
| `debugging-and-error-recovery` | addyosmani/agent-skills | Root-cause debugging for build errors, crashes, bugs |
| `code-review-and-quality` | addyosmani/agent-skills | Code review before merge |
| `documentation-and-adrs` | addyosmani/agent-skills | Recording architecture decisions (ADRs) |
| `git-workflow-and-versioning` | addyosmani/agent-skills | Branches, commits, PRs, releases |
| `planning-with-files` | othmanadi/planning-with-files | Saving plans and progress to files so work continues across sessions |
| `stop-slop` | hardikpandya/stop-slop | Removing AI writing patterns from documents and design docs |

Plugins (`.claude/settings.json`): `caveman` (terse replies), `ponytail` (minimal code, no over-engineering).

Tool (global, installed in step 4): **CodeGraph**, which provides code navigation, callers/callees and impact analysis through the `codegraph_explore` MCP tool or the `codegraph explore "<query>"` CLI. Once `.codegraph/` exists, use it before grep/find.

To add or remove a skill: update the script in step 2, the list in step 5, and the table above.
