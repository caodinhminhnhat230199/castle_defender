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

### 2026-10-04: Claude Code: FT_Smoke wired, T-FND-10 done
- **Tasks:** T-FND-10 Review → Done.
- **Changed:** `Content/CastleDefender/Maps/Test/BP_FT_Smoke.uasset` (Start Test graph wired by the user in the editor: spawn `TestDummy`, `Apply Debug Hit` 1000, `Get Health` → `Is Dead` → `Finish Test`), `00-foundation/tasks.md`.
- **Verified:** editor Test Automation: `FT_Smoke_DummyDies` Success ("Dummy died"). `Tools/run_tests.ps1`: 11/11 passed (10 specs + FT_Smoke), exit 0.
- **Manual steps for the user:** steps 2–4 from the entry below (PIE checks, editor checks, windowed packaged build).
- **Open questions / blockers:** unchanged from the entry below.
- **Next:** finish those checks, move T-FND-06…09 to Done, then run the Phase F gate check before P0 (T-CMB-01).

### 2026-10-04: Claude Code: Phase F foundation implemented
- **Branch:** `task/T-FND-foundation` (from `main`), one commit per task, not pushed. Your uncommitted `.claude/settings.json` change was left untouched.
- **Decisions (user, this session):** Q-15 answered: project and module `CastleDefender`, UE 5.8 (5.8.3), reference PC = the dev PC (i5-14500, RTX 4070 Ti SUPER, 32 GB, 1080p) at 60 fps [TUNABLE]. Python Editor Script Plugin enabled for asset creation. Branch + commit per task allowed.
- **Tasks:**
  - Done: T-FND-01, T-FND-02, T-FND-03, T-FND-04, T-FND-05.
  - Review (code done and verified headless; editor/PIE checks below): T-FND-06, T-FND-07, T-FND-08, T-FND-09.
  - Review, blocked on one manual step: T-FND-10 (FT_Smoke graph).
- **Changed:**
  - Project: `CastleDefender.uproject` (plugins: ModelingToolsEditorMode, PythonScriptPlugin, FunctionalTestingEditor), `Source/CastleDefender*.Target.cs`, `Source/CastleDefender/CastleDefender.Build.cs`, `Config/Default{Engine,Game,Input}.ini`, `.gitignore`, `.gitattributes`.
  - C++: `Core/` (GameLog, GameTags, GameDebug, GameCheatManager, GameTuningSettings, GameDefinition), `Combat/` (CombatTypes, HealthComponent, CombatStateComponent, TestDummy), `Player/` (PlayerMode, HeroPlayerController), `Tests/` (Health, ModeStack, GameDefinition, GameTags specs + helpers).
  - Content (LFS, made by `Tools/create_foundation_assets.py`): `Maps/L_Boot`, `Core/Input/IA_*` (9 P0 actions + `IA_DebugToggleBuild`), `IMC_Combat/CommandWheel/TacticalFocus/CommanderSpirit/Build/Debug`, `Core/BP_HeroPlayerController`, `Core/BP_BootGameMode` (global default game mode), `Maps/Test/FT_Smoke` + `BP_FT_Smoke`.
  - Tools: `build.ps1`, `run_tests.ps1/.bat`, `package.ps1`, `create_foundation_assets.ps1/.py`, `UERoot.ps1`.
  - Docs: `AGENTS.md` §1/§7, main plan §2/§8a (mode-stack contract)/Q-15, foundation `technical-plan.md` §1/§15/§16, `spec.md` §12 (NEW-FND-1), `tasks.md`, new `input-keymap.md`, `profiling-checklist.md`.
- **Verified:**
  - `Tools/build.ps1`: `CastleDefenderEditor` Development and `CastleDefender` Development + Shipping build with zero project warnings (UBT warns MSVC 14.51 is newer than its preferred 14.50).
  - `Tools/run_tests.ps1`: 10/10 specs pass (`CastleDefender.Combat.Health`, `.Player.ModeStack`, `.Core.GameDefinition`, `.Core.GameTags`). `FT_Smoke` is discovered and **fails by timeout** (graph not wired); the script exits 1, which also proves a failure makes it exit non-zero.
  - Headless `-game` run on `L_Boot`: `BP_BootGameMode` loads, the mode stack switches contexts and logs (Combat → Build → Modal → Build, out-of-order pop keeps the top, unknown reason warns), `SpawnTestDummy` works, all 5 `game.debug.*` CVars exist.
  - `Tools/package.ps1`: Win64 Development package succeeds with no cook errors; the packaged exe (headless) runs the CVar and cheats and wrote a 670 KB trace.
  - Fresh clone + `git lfs pull` (21 files) + editor build + `L_Boot` load: no missing assets.
  - Assets read back from disk: key mappings, modifiers and per-mode contexts are as in `input-keymap.md`.
- **Manual steps for the user:**
  1. **FT_Smoke graph (finishes T-FND-10).** Open `Content/CastleDefender/Maps/Test/BP_FT_Smoke`. From the existing *Event Start Test*: *Spawn Actor from Class* (Class `TestDummy`, Spawn Transform = *Get Actor Transform* of self, Collision Handling = *Always Spawn*) → *Apply Debug Hit* (Target = Return Value, Damage 1000) → *Branch* on Return Value → *Get Health* → *Is Dead* → True: *Finish Test* (Succeeded, "Dummy died"); False: *Finish Test* (Failed, "Dummy survived"). Compile, save, run `Tools\run_tests.bat` → 11 passed, exit 0.
  2. **PIE on `L_Boot` (T-FND-06, T-FND-09).** Press F5 → Output Log shows `Player mode Combat -> Build` and `Active mapping contexts for Build: [IMC_Build]`; F5 again → back to Combat. Console: `DebugPushMode Modal Menu`, `DebugPopMode Menu` → back to the previous mode. `game.debug.Combat 1` then `SpawnTestDummy` → green sphere and `100 / 100` above the dummy. `God` and `SetTimeDilation 0.5` respond.
  3. **Editor (T-FND-04, T-FND-07).** Project Settings → Game → *Game Tuning* page exists (empty until features add values). Any Gameplay Tag picker shows the roots `State, Unit, Structure, Command, Zone, Damage, Perk, Stat, Feedback, Modifier, Lane, Resource, Tutorial`.
  4. **Packaged build (T-FND-08).** Follow `ai/game/00-foundation/profiling-checklist.md` once in a window: `stat unit` visible, trace opens in Unreal Insights.
- **Open questions / blockers:**
  - AC-FND-06 "Asset Manager lists the registered type" can only pass once the first `UGameDefinition` subclass exists (e.g. `UHeroClassDefinition`); that task must register its type in `DefaultGame.ini` (convention is written there as a comment).
  - NEW-FND-1: functional tests are Blueprint-only under D-01 (a C++ `AFunctionalTest` breaks Shipping for the single module).
  - `ATestDummy::ApplyDebugHit` calls `UHealthComponent::ApplyHit` directly because `DeliverHit` does not exist yet; T-CMB-04 must switch it.
  - Commit `f46b2de` uses type `build(fnd)`, which is not in the AGENTS §8 list (should be `chore`); left as is to avoid rewriting history.
  - Spec-audit items for Phase F not covered by the FND tasks remain open: `decisions.md` (L2), bug/severity policy (L4), localization rule (L10), build versioning/crash logs (L7), performance budget doc (L8).
- **Next:** do manual steps 1–4, then move T-FND-06…10 to Done and run the Phase F gate check (main plan §3) before starting P0 (T-CMB-01).

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
