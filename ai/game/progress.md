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

### 2026-10-05: Claude Code: Commit Hero Combat work, split branches for UXF/SYN
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents, editor closed. No push.
- **Decision (user):** commit the open Hero Combat work, then do T-UXF-01 on `feat/13-hud-feedback` and T-SYN-01 on `feat/04-battlefield-synergy`, both from `main` (SYN merges UXF first). Merge `feat/04-battlefield-synergy` into `feat/01-hero-combat` when T-SYN-01 is Done, to finish T-CMB-06.
- **Tasks:** no status change (T-CMB-05/07/11/21 Review, T-CMB-06 In Progress).
- **Changed:** commits on `feat/01-hero-combat`: placeholder Mannequin content, then hero code/assets (placeholder montages, foot IK, camera facing, Heavy), then docs. `CMB/WIP.md` refreshed (task counts, T-CMB-06 status, branch plan, verification).
- **Left uncommitted on purpose:** `.claude/settings.json` (key reorder only, author unknown), `Config/DefaultEditor.ini` (editor-saved preview scene profile), `Placeholder/Weapons/SM_Placeholder_Blade` (unused; delete waits on user OK).
- **Verified:** `Tools/build.bat` succeeds; `Tools/run_tests.bat` 79/79, editor exit 0 (`Saved/wip-tests.log`).
- **Manual steps for the user:** rendered PIE sign-off for T-CMB-05/07/11 (see "Mannequin placeholder" entry below, manual step 1).
- **Next:** T-UXF-01 on `feat/13-hud-feedback`.

### 2026-10-05: Claude Code: Heavy attack, and fix for Heavy blocking Light
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents, editor closed. No commit or push.
- **Request (user):** create the heavy attack. Pressing Heavy did nothing, and afterwards Light could not be pressed.
- **Cause:** `RequestAction(Heavy)` called `SetActionState(HeavyAttack)` without a montage (T-CMB-06 not started). No montage end ever returned the hero to Idle, so every later action was refused (breaks R-CMB-45).
- **Dependency note:** T-CMB-06 depends on T-SYN-01 (Todo). At the user's request the hero side was built now. T-CMB-06 is **In Progress**: the poise-break / Staggered, Armor Broken hook and AC-CMB-25 checks wait for T-SYN-01.
- **Changed:**
  - C++:
    - `UHeroClassDefinition::ValidateHeavyAttack`: valid montage, one hit window after a startup, later Dodge-only cancel, Heavy out-damages / out-poises / out-costs every Light. Also called from `IsDataValid`.
    - `UHeroCombatComponent`: Heavy validates before stamina is spent, arms the trace with `bIsHeavy` and `AppliedStates`, and plays `Heavy.Montage`. The trace payload code is shared with Light in `ArmMeleeTrace`.
  - Tests: new `HeavyAttack.spec.cpp`. `HeroClassDefinition.spec.cpp` now copies the authored Heavy into its fixture (it failed once without it).
  - Tools / Content: `create_hero_assets.py` authors `AM_Warlord_Heavy` from `MM_ChargedAttack`. Clip from 0.55 s (skips most of the 1 s hold); hit 0.45–0.67 s, matching the measured strike at clip 1.0–1.22 s and the 150 cm lunge; Dodge cancel 0.95–1.25 s. It fills `DA_HeroClass_Warlord.Heavy.Montage`, and `author_montage` gains `clip_start`.
  - Docs: CMB `tasks.md` (T-CMB-06 In Progress, 4 notes + 1 criterion ticked, progress note), this log.
- **Verified:**
  - Editor and game Development builds succeed. Asset script exit 0.
  - `Tools/run_tests.bat`: 79/79, runner exit 0.
  - Rendered PIE on `L_CombatSandbox` with injected `IA_HeavyAttack` then `IA_LightAttack` (`Saved/heavy-pie.json`, `Saved/Logs/heavy-pie-engine.log`, no new warnings):
    - Heavy accepted, stamina 100→75, hit window at about 0.45 s, dummy 100→70, lunge 150 cm, Idle at 1.3 s.
    - Light afterwards accepted, dummy 70→60.
    - Screenshots: `Saved/Screenshots/WindowsEditor/heavy_windup.png`, `heavy_strike.png`.
- **Manual steps for the user:** play `L_CombatSandbox`. Press Heavy (right mouse) near a dummy, then Light. Try Light → Heavy from the Light 1/2 cancel window, and Dodge out of the Heavy recovery.
- **Open questions / blockers:** T-SYN-01 (poise / Staggered) is needed to finish T-CMB-06. Wind-up length (about 0.45 s to the hit) is a placeholder tune for G0.
- **Next:** user sign-off for T-CMB-05/07/11 and the Heavy feel; then T-UXF-01 → T-SYN-01 → finish T-CMB-06 → T-CMB-20.

### 2026-10-05: Claude Code: Camera-facing hero (God of War style), S+Space plays Dodge B
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents. The user closed the editor before the build. No commit or push.
- **Request / decision (user):** the hero must not turn around. It always faces forward like God of War: S only moves backward, and S + Space plays Dodge B exactly. Recorded in spec R-CMB-03 / R-CMB-21 (both still hold: movement and dodge stay camera-relative) and in T-CMB-01 / T-CMB-07 notes.
- **Tasks:** T-CMB-05/07/11 stay Review.
- **Changed:**
  - C++:
    - `FHeroMovementData::bFaceCameraDirection` (default true). `AHeroCharacter::ApplyTuning` sets `bUseControllerDesiredRotation` / `bOrientRotationToMovement` from it, and the constructor default matches. New `IsFacingCameraDirection()`.
    - `FHeroDodgeData::bSideClipsFaceInput` (default false). The `SelectDirection` parameter is renamed to `bFacingFixed`.
    - `UHeroCombatComponent`: a camera-facing hero picks F/B/L/R relative to its facing and does not turn. It turns toward the input only in turn-to-movement mode, or for side dodges while `bSideClipsFaceInput` is set.
    - Engine check: `CharacterMovementComponent.cpp:3044` skips physics rotation during anim root motion, so dodges and attacks travel straight and the hero turns back to the camera afterwards.
  - Tests: `Dodge.spec.cpp` (relabelled selection cases; new facing-mode tuning case).
  - Tools / Content: `create_hero_assets.py` sets `Dodge.bSideClipsFaceInput = true` on `DA_HeroClass_Warlord` (L/R are forward-dash placeholders). The template `ABP_Warlord` already computes `Direction` when orient-to-movement is off and feeds the 8-way walk/jog blendspace. No ABP change.
  - Docs: CMB `spec.md`, `tasks.md`, `WIP.md`, this log.
- **Verified:**
  - Editor and game Development builds succeed. An earlier attempt failed because the editor was open with Live Coding; it was rerun after the user closed it.
  - Asset script exit 0.
  - `Tools/run_tests.bat`: 76/76, runner exit 0.
  - Rendered PIE on `L_CombatSandbox` with injected Enhanced Input (`IA_Move`, `IA_Dodge`) (`Saved/facing-pie.json`, `Saved/Logs/facing-pie-engine.log`, no new warnings):
    - Hold S: yaw stays 0°, velocity -450 cm/s backward, ABP `Direction` 180°.
    - S+Space: `AM_Warlord_Dodge_B` with yaw unchanged, about 365 cm backward.
    - A+Space: `AM_Warlord_Dodge_L`, yaw -90° during the dash, about 330 cm left, then back to 0°.
    - Screenshots: `Saved/Screenshots/WindowsEditor/facing_backpedal.png`, `facing_dodge_b_mid.png`.
- **Manual steps for the user:** play `L_CombatSandbox`. Hold S (backpedal), then S+Space (Dodge B), W/A/D + Space, and turn the camera while moving.
- **Open questions / blockers:** side dodges still turn the hero because the template has no side-step clips. Real side-step clips (and `bSideClipsFaceInput = false`) belong with T-CMB-10 or a Fab animation pack. Unused `SM_Placeholder_Blade` still waits on the user's OK to delete.
- **Next:** user rendered PIE sign-off for T-CMB-05/07/11, then T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20.

### 2026-10-05: Claude Code: Foot IK no longer pins montage legs
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents, editor closed. No commit or push.
- **Request (user):** during the light combo, especially Light 3, the legs stick to the ground and do not follow the montage.
- **Cause:** `ABP_Warlord` is a copy of the template `ABP_Unarmed`. Its AnimGraph is `Slot 'DefaultSlot'` → Control Rig `CR_Mannequin_FootIK` (`ShouldDoIKTrace = NOT IsFalling`, Alpha 1) → Output Pose. Foot IK ran on top of every montage and pulled the raised knee and stepping feet onto the traced ground.
- **Tasks:** T-CMB-05/07/11 stay Review.
- **Changed:**
  - C++: new `Hero/HeroAnimInstance.h/.cpp` (`UHeroAnimInstance`). `FootIKAlpha` targets 0 while `GetCurrentActiveMontage()` is set and 1 otherwise, fading over `FootIKBlendTime` (0.15 s, editable in the ABP defaults). Montages that are blending out do not count as active, so IK returns as the pose blends back to locomotion. `IsAnyMontagePlaying()` was rejected: it also counts stopped instances, and the new spec caught that.
  - Tests: new `HeroAnimInstance.spec.cpp` (`CastleDefender.Combat.Hero.AnimInstance`).
  - Tools: `create_hero_assets.py` reparents `ABP_Warlord` to `UHeroAnimInstance` and connects `Get FootIKAlpha` → Control Rig `Alpha`. This uses the UE 5.8 `BlueprintGraphEditor` / `BlueprintGraphPinLibrary` editor scripting APIs (checked against the engine headers). A rerun leaves the ABP byte-identical.
  - Content: `Hero/ABP_Warlord` (parent class, one new variable-get node and link). Root motion mode stays `RootMotionFromMontagesOnly`.
  - Docs: CMB `tasks.md`, `WIP.md`, `technical-plan.md` (file layout lists `HeroAnimInstance`), this log.
- **Verified:**
  - Editor and game Development builds succeed (only the existing engine deprecation warning).
  - Asset script exit 0, and the rerun is unchanged. A graph dump shows the parent is `HeroAnimInstance` and `Alpha` is linked to `FootIKAlpha`.
  - `Tools/run_tests.bat`: 75/75, runner exit 0. The first run failed the new spec (alpha stayed 0 after `Montage_Stop`); fixed by switching to `GetCurrentActiveMontage()`.
  - Rendered scripted PIE on `L_CombatSandbox`: `FootIKAlpha` fades 1→0 in about 0.15 s at Light 1, stays 0 through Light 1→2→3, and returns to 1 after the chain. Dummy 100→90→80→66. No new log warnings.
  - Light 3 screenshots (`Saved/Screenshots/WindowsEditor/unarmed_light3{,b,c}.png`) show the knee raised, the step-in and the wide landing stance, with no feet pinned. Evidence: `Saved/unarmed-pie.json`, `Saved/Logs/footik-pie-engine.log`, `Saved/footik-*.log`.
- **Manual steps for the user:** in the editor, play `L_CombatSandbox`, press Light ×3 and watch the legs on Light 3. Also check that locomotion on uneven ground still uses foot IK, and dodge/hit react look right.
- **Open questions / blockers:** unchanged (unused `SM_Placeholder_Blade` waits on the user's OK to delete).
- **Next:** user rendered PIE sign-off for T-CMB-05/07/11, then T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20.

### 2026-10-05: Claude Code: Unarmed Warlord demo and SpawnTestDummy placement fix
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents. The user closed the editor before the build. No commit or push.
- **Request (user):** the light combo looked like it broke the mesh. Use Light 1/2/3 as an unarmed combo with no sword for the demo. `SpawnTestDummy` placed the dummy on top of the hero, so the hero could not move.
- **Causes:**
  - The "broken mesh" was the placeholder blade: an engine cube stretched to 75 × 5 × 2 cm along the forearm. The clips themselves play cleanly on Manny.
  - `SpawnTestDummy` measured 400 cm from the camera. The camera boom is 400 cm, so the dummy landed on the hero, and spawn collision handling pushed it up on top of him.
- **Tasks:** T-CMB-05/07/11 stay Review. T-CMB-01's "sword + shield" checkbox now carries a note that the P0 demo is unarmed, at the user's request.
- **Changed:**
  - C++: `UGameCheatManager::SpawnTestDummy` spawns from the pawn location (falls back to the view location when there is no pawn), along the view yaw.
  - Tools: `create_hero_assets.py` clears `WeaponMesh` (no mesh, identity transform) and sets the mesh tick option back to the engine default `AlwaysTickPose`. `create_hero_placeholder_content.py` no longer creates the blade.
  - Content: `BP_Hero_Warlord` resaved without the blade. Hits use `UMeleeTraceComponent`'s existing fallback arc (50–180 cm in front of the hero).
  - Docs: CMB `tasks.md`, `WIP.md`, `Placeholder/LICENSES.md` (blade marked unused), this log.
- **Verified:**
  - Editor and game Development builds succeed, with only the existing engine `GetMovementBase` deprecation warning. Logs: `Saved/unarmed-editor-build.log`, `unarmed-game-build.log`.
  - `Tools/create_hero_assets.bat` exits 0 (`Saved/unarmed-assets.log`).
  - `Tools/run_tests.bat`: 74/74 pass, runner exit 0 (`Saved/unarmed-tests.log`).
  - Rendered scripted PIE on `L_CombatSandbox` (`Saved/unarmed-pie.json`, `Saved/Logs/unarmed-pie-engine5.log`):
    - Saved hero has no weapon mesh.
    - Default `SpawnTestDummy` puts the dummy 400 cm in front at the hero's height (dz 0).
    - The hero then moves 175 cm with movement input.
    - Light chain 0→1→2 through real sweeps: dummy 100→90→80→66, then Idle.
    - Screenshots of each swing are in `Saved/Screenshots/WindowsEditor/unarmed_light{1,2,3}.png`. The poses are normal unarmed punches with no stretched geometry.
  - One `FindTeleportSpot` warning came from the script's second `SpawnTestDummy 150`, which overlapped the first dummy after the hero walked toward it. It is a test-setup artifact; the default spawn logs nothing.
- **Manual steps for the user:** in the editor, play `L_CombatSandbox`, run `SpawnTestDummy`, walk up to it and press Light ×3, then repeat after a pause. Continue the rest of manual step 1 from the "Mannequin placeholder" entry below.
- **Open questions / blockers:** `Placeholder/Weapons/SM_Placeholder_Blade` is untracked and unreferenced. Deleting it is waiting on the user's OK.
- **Next:** user rendered PIE sign-off for T-CMB-05/07/11, then T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20.

### 2026-10-05: Claude Code: Partial rendered PIE check of the Warlord placeholder
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents. Documentation only. No commit or push.
- **Tasks:** T-CMB-05, T-CMB-07 and T-CMB-11 stay Review. The user's PIE run covers only part of manual step 1 in the entry below.
- **Changed:** this log. The other uncommitted files are from the entry below. The `.claude/settings.json` diff only reorders and reformats the plugin keys, and its author is unknown. Left untouched.
- **Verified (from `Saved/Logs/CastleDefender.log`, user's editor session 18:07–18:20, rendered PIE on `L_CombatSandbox` 18:10:52–18:12:24):**
  - Mode stack starts in Combat (`IMC_Combat`).
  - Light chain with real input: 0→1→2, then a pause, then 0→1→2 again. The chain resets after the gap. One Dodge was accepted (action 1, state 2).
  - No `LogGame*` warnings or errors. The only warnings come from the engine or this machine: the audio sample rate differs (48000 vs 44100), and `LogCrowdFollowing` reports no RecastNavMesh during PIE teardown (the sandbox has no navmesh).
  - The log cannot show visual quality, how the swings look or which way the dodge went.
- **Not yet checked in rendered PIE:** `game.debug.Combat` / `CombatTrace` draw, `SpawnTestDummy` hits and damage, dodge with input vs. without input, `DebugHitHero`, `KillHero` and respawn.
- **Manual steps for the user:** finish manual step 1 of the entry below (debug draw, dummy hits, both dodge cases, hit react, death and respawn). Tell the agent what you saw so it can move T-CMB-05/07/11 to Done.
- **Open questions / blockers:** unchanged from the entry below. The Aura entry is no longer in `CastleDefender.uproject` and does not appear in today's log.
- **Next:** finish the rendered PIE sign-off for T-CMB-05/07/11, then T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20.

### 2026-10-05: Claude Code: Mannequin placeholder mesh/animation for the Warlord
- **Agent / branch:** Claude Code, `feat/01-hero-combat`; no other active agents. The user closed the editor before the build and asset writes. No commit or push.
- **Tasks:** T-CMB-05, T-CMB-07 and T-CMB-11 stay Review. Their placeholder content is assembled and verified headless; rendered PIE with real input is still pending. Ticked T-CMB-05 "montage windows" and T-CMB-07 "distance via root motion". No phase or gate change.
- **Source:** the Fab library cache on this PC is empty (`VaultCache/FabLibrary/listings_v1.db` has 0 listings), so the only Epic sample content available is the UE 5.8 Third Person template's Mannequin pack. Recorded in `Placeholder/LICENSES.md`.
- **Changed:**
  - C++: `AHeroCharacter` gains a `WeaponMesh` component (tag `Weapon`, no collision). `OnConstruction` attaches it to `WeaponSocketName` (default `hand_r`) only when the mesh has that socket, which avoids socket warnings on bare native spawns. `BeginPlay` hands it to `MeleeTraceComponent::SetTraceMesh`. New editor helper `UHeroCombatLibrary::SetSingleSegmentMontageSource` (sequence + skeleton, trim, fit duration, optional reverse). The test fixture uses `SKM_Manny_Simple`.
  - Tools: new `create_hero_placeholder_content.py` copies the template subset, moves it to `Placeholder/Mannequins` with references fixed and builds `SM_Placeholder_Blade`. `create_hero_assets.py` retargets legacy Tutorial_Idle montages in place (DA references kept), authors windows from the measured clips, builds `ABP_Warlord` and assembles the hero. Reruns change nothing.
  - Content: `Placeholder/Mannequins/` (68 assets), `Placeholder/Weapons/SM_Placeholder_Blade`, `Hero/ABP_Warlord`, 10 `AM_Warlord_*` montages, `DA_HeroClass_Warlord` (`Dodge.RootMotionScale` 0.4), `BP_Hero_Warlord` (mesh, ABP, blade, bone refresh when unrendered), `BP_SandboxGameMode` (resaved by the script).
  - Docs: CMB `tasks.md` evidence and remaining steps, `WIP.md`, `LICENSES.md`, this log.
- **Design notes:** the template only has unarmed punches and a kick, so a handheld sword would point up during punches and miss. The placeholder is a 75 cm blade along the forearm (`hand_r` -X; `weapon_r` is keyed inconsistently between clips). HitReact clips are rifle-pose. Dodge B is the dash played reversed. L/R reuse the forward dash because there is no side-step clip; only lock-on (T-CMB-10) selects them.
- **Verified:**
  - Editor and game Development builds succeed. Only the existing engine `GetMovementBase` deprecation warnings appear. Logs: `Saved/placeholder-editor-build3.log`, `placeholder-game-build.log`.
  - `Tools/run_tests.bat`: 74/74 pass, 0 warnings, runner exit 0. A later rerun was also 74/74, but two tests caught `LogHttp` warnings from the Aura plugin, so the runner exited 1 (see blockers).
  - Saved asset readback: Manny mesh, ABP class, blade on `hand_r` with tag and sockets, `RootMotionFromMontagesOnly`, montage skeletons and lengths, death auto blend-out off.
  - Scripted headless PIE on `L_CombatSandbox` (saved `BP_Hero_Warlord`, real sweeps, no direct `TryHitTarget`): chain 0→1→2, dummy 100→90→80→66. Zero-input Dodge moves 225 cm backward. `DebugHitHero 20 0 1` gives HitReact_F, HP 180, then Idle. `KillHero` gives Dead with the death montage held; respawn after 3 s at HP 200. Evidence: `Saved/placeholder-pie.json`, `placeholder-pie-engine.log`. The first PIE run showed static sockets because the default tick option does not refresh bones when unrendered; that is now fixed in the BP.
- **Manual steps for the user:**
  1. Open `L_CombatSandbox` and play with keyboard/mouse. Run `game.debug.Combat 1` and `game.debug.CombatTrace 1`, then `SpawnTestDummy`. Light ×3 should show readable swings and blade sweeps; wait, then Light again should restart at hit 1. Dodge with input should go in the input direction and without input go backward. Check `DebugHitHero 20 0 1`, `KillHero` and the respawn. Confirm the Output Log has no new warnings, then move T-CMB-05/07/11 to Done if the acceptance criteria hold.
  2. Optional: replace the unarmed placeholders with sword clips from a free Fab pack (e.g. Paragon Greystone or Kwang) by retargeting them to Manny, and add side-step clips for Dodge L/R before T-CMB-10.
- **Open questions / blockers:** the editor session enabled the **Aura** plugin in `CastleDefender.uproject` (uncommitted, not by this agent). It adds two `GameFeatureData` asset-manager errors, so `create_hero_assets.bat` exits 1 even though the script succeeds. Its HTTP calls to `127.0.0.1:41200` intermittently add warnings to automation tests, which makes the gate flaky. At the user's request, the Aura entry was then removed by restoring `CastleDefender.uproject` to its committed version. Restart the editor for this to take effect. If Aura comes back, add a `GameFeatureData` asset-manager rule.
- **Next:** rendered PIE sign-off for T-CMB-05/07/11, then T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20.

### 2026-10-04: Codex: Saved remaining Hero Combat work as a teammate handoff
- **Agent / branch:** Codex, `feat/01-hero-combat`; preserved the existing empty, untracked `01-hero-combat/WIP.md` by filling the file requested by the user. No gameplay implementation or binary assets changed.
- **Tasks:** no status changes; Hero Combat remains 5 Done, 4 Review and 12 Todo. This documentation-only handoff covers all 16 incomplete tasks.
- **Changed:** `01-hero-combat/WIP.md` saves the requested Vietnamese explanation, task blockers, approved feature-branch workflow, continuation order, provider-document links and the distinction between recorded verification and remaining content/PIE acceptance. This log records the handoff.
- **Verified:** checked all 16 incomplete task IDs and relative document links, task counts against `tasks.md`, and whitespace. No fresh Unreal build/tests/PIE for this documentation-only change; earlier evidence is linked without claiming new verification.
- **Manual steps for the user:** none to save the handoff; the teammate should follow the editor and verification steps in WIP and earlier entries.
- **Open questions / blockers:** existing animation/hero assembly and external SYN/ENM/UXF dependencies remain. P2/VS tasks stay closed until their gates open. No push or PR in this session.
- **Next:** complete content/PIE for T-CMB-05/07/11, then follow T-UXF-01 -> T-SYN-01 -> T-CMB-06 -> T-CMB-20 and the P0A checkpoint.

### 2026-10-04: Codex: Corrected workflow to one Hero Combat feature branch with incremental commits
- **Approval / workflow:** the user clarified that one branch means `feat/01-hero-combat` for every Hero Combat task, with multiple commits, followed by a PR into `main` only after all feature tasks are complete. This supersedes the main-only interpretation below. The user explicitly requested committing all current work to this feature branch. Subsequent task increments reuse it; no per-task branches. Phase/dependency gates and task verification requirements still apply.
- **Agent / branch:** Codex, `feat/01-hero-combat`, original checkout. Verified one worktree and no other active agents before switching. Created the feature branch at existing HEAD `509d51d`, then restored local `main` to its recorded pre-consolidation Foundation merge `0e6e6b0` (also the `origin/main` tracking ref). All six existing Hero Combat commits remain unchanged and reachable on the feature branch; no commits were rewritten or remote refs changed.
- **Tasks:** T-FND-02 stays Done; Hero Combat stays 5 Done, 4 Review and 12 Todo. T-CMB-05/07/11/21 remain Review; neither feature completion nor a phase gate is claimed.
- **Changed / commits:**
  - `b9f9737`: existing shared Hero Combat runtime and automation fixtures (30 Source files). Light/Dodge/hit/death/debugger changes already shared action files, so this checkpoint preserves them together; later task increments should stay scoped to one task where possible.
  - `6960bf6`: existing Editor-authored Blueprint/data edits and ten montage timing fixtures, plus the two asset/respawn tools (15 files). Assets were staged through Git LFS; no binary asset was edited in this session.
  - Documentation commit: `AGENTS.md` sections 8/11, Foundation source-control plan/checklist, Hero Combat task evidence, master-plan section 8a and this log (six files). Rules now specify one shared `feat/<NN-feature>` branch, multiple task-scoped commits and a PR only after all feature tasks are Done and verified.
- **Verified:** initial snapshot captured 51 changed/untracked files in `Saved/Backups/feature-branch/before.json`; branch creation/main restoration preserved every file exactly. After staging, whitespace checks caught an extra blank line at EOF in `HeroHitReaction.spec.cpp` and `create_sandbox_respawn.py`; removed only those blank lines and verified the diffs contained no behavior change. SHA-256 still matches for the other 45 pre-existing files recorded before the original consolidation. All 13 staged asset pointers matched the working asset SHA-256 and size. Staged whitespace checks pass; final Git checks confirm only `main` and `feat/01-hero-combat`, all original branch tips reachable, nine commits ahead of `main` and a clean working tree after the documentation commit. Generated directories remain ignored.
- **Verification limits:** no fresh Unreal build/tests/PIE for this source-control correction. Existing executable code and asset content were preserved; only two EOF blank lines changed. Read the existing automation report: 74 succeeded, zero warnings/failures/NotRun/InProcess. Earlier build/PIE results and pending content acceptance remain recorded below; the montages are still Tutorial_Idle timing fixtures.
- **Manual steps for the user:** none for the branch/commit correction. The gameplay/editor steps below still apply.
- **Open questions / blockers:** feature incomplete (4 Review, 12 Todo); no push, PR or merge in this session. Do not create the PR while tasks remain incomplete. The previous gameplay content/dependency blockers are unchanged.
- **Next:** continue task-scoped commits on `feat/01-hero-combat`; finish playable content/PIE for T-CMB-05/07/11, complete T-SYN-01 before T-CMB-06, and follow the existing phase gates. When all Hero Combat tasks are Done and verified, create its PR into `main` as requested.

### 2026-10-04: Codex: Consolidated local task branches into main
- **Workflow (superseded):** Codex interpreted the request for one branch with multiple commits as development on `main`. The user subsequently clarified that it means one shared feature branch, then a PR into `main` after all feature tasks are complete. The correction is recorded in the newer entry above; the main-only policy is no longer active.
- **Agent / branch:** Codex, `main`; one worktree and no other active agents were found before switching. Preserved all existing staged/unstaged work; the index was empty. No new commit, push, remote branch deletion or history rewrite.
- **Tasks:** T-FND-02 remains Done; updated its source-control checklist wording. Hero Combat task statuses and phase gates are unchanged.
- **Changed:** `AGENTS.md` sections 8/11, `00-foundation/technical-plan.md` section 17, `00-foundation/tasks.md` and this log. Fast-forwarded local `main` from `0e6e6b0` to the existing HEAD `509d51d`, preserving six existing commits. Switched the checkout to `main` and removed six fully merged local branches with `git branch -d`: T-CMB-01/02/03/04/05 and T-FND-foundation. Historical entries below retain the branch names used at the time.
- **Verified:** checked all original branch tips were ancestors of HEAD before consolidation and remained reachable from `main` afterward. HEAD and its committed tree stayed unchanged; SHA-256 matched for all 47 pre-existing changed/untracked files outside this log. Baseline refs/status/hashes: `Saved/Backups/single-branch/before.json`. `git branch -vv` now lists only `main`; `git worktree list` shows the original checkout on `main`. Local tracking comparison reports six commits ahead of `origin/main`. Workflow-document whitespace check passes. No Unreal build/tests/PIE were run for this Git/documentation-only change.
- **Manual steps for the user:** none for the local branch consolidation. Existing gameplay/editor verification steps below still apply; remote refs were left untouched.
- **Open questions / blockers:** none for this workflow change. Pending Hero Combat content and integration gates remain as recorded below. Documentation changes and pre-existing gameplay work remain uncommitted.
- **Next:** continue on `main`; finish playable content/PIE for T-CMB-05/07/11, then T-SYN-01 before T-CMB-06. Keep subsequent commits scoped to their task IDs.

### 2026-10-04: Codex: Dodge, directional hit/death/reset and combat debugger verified; content gates remain
- **Agent / branch:** Codex, `task/T-CMB-05-light-chain`; preserved the existing uncommitted Light work. No concurrent agents, branch switch, commit or push. No CodeGraph index is present; inspected source and pinned engine headers directly. Used the repository game-development-workflow and ue5-project-architecture guides.
- **Tasks:** T-CMB-07 and T-CMB-11 Todo -> In Progress -> Review; T-CMB-21 Todo -> In Progress -> Review. T-CMB-05 stays Review. Task statuses remain 5 Done, 4 Review, 12 Todo out of 21; no phase/gate has been advanced.
- **Changed:**
  - Dodge: F/B/L/R data, free-camera input direction/turning, zero-input Backward, pure locked-direction selection, authored i-frames/recovery cancels, single stamina spend and restoring prior root-motion scale. Runtime locked facing waits for T-CMB-10. Four 0.6-second Idle-based timing fixtures were created through the editor.
  - Input buffer expiry now ticks only while buffered and uses component DeltaTime, which the pinned engine's FActorComponentTickFunction::ExecuteTickHelper already multiplies by owner CustomTimeDilation. No world deadline or double dilation.
  - Hit/death: front/back reactions, authored late Dodge cancel, data-defined resistance disabled by default, no damage/death immunity, no duplicate HeroDamaged feedback, once-only death/Feedback.Hero.Death request after committing Dead for all observers, movement/sprint/action lockout, window/buffer cleanup and Dead preservation when animation cannot play. Restarted instances of one montage ignore stale end callbacks. Block outcome metadata is resolver-owned in damage delegates and resolution records. Shared Staggered stays owned by SYN; its timers/poise behavior were not claimed from the Foundation skeleton.
  - Sandbox Blueprint: OnRestartPlayer binds each possessed hero's OnHeroDeath; SandboxHeroDied sets a world-game-time timer; SandboxRespawn destroys the controlled pawn and RestartPlayer resets it. RespawnDelay is a Blueprint real variable, default 3 seconds. KillHero uses DeliverHit. No C++ respawn owner or new dependency/plugin/module was added.
  - Debugger: live montage position and derived phase/windows, buffer/age/stamina/shared state; explicit inactive lock-on/assist/consumed-parry placeholders; raw timing report with invalid action/montage/window context; sweep/hit point/already-hit set and resolution drawing. Combat/CombatTrace CVars and draw paths plus cheat bodies are excluded in Shipping; disabled CVars skip readout/draw assembly.
  - Authoring script now fills missing montage references/fixtures and preserves existing combat values, resistance settings and authored Light/Dodge/HitReact windows on rerun. Deprecated DodgeStaminaCost remains serialized compatibility only; Dodge.StaminaCost owns the current cost.
- **Files:** Hero/HeroCharacter.*, HeroCombatComponent.*, HeroCombatTypes.h, HeroClassDefinition.*, HeroCombatLibrary.*; Combat/CombatActionTiming.*, CombatLibrary.cpp, CombatTypes.h, MeleeTraceComponent.cpp, TestDummy.cpp; Core/GameCheatManager.*, GameDebug.*, GameTags.*; Tests/HeroCombatFixture.h, CombatTestListener.h, Dodge.spec.cpp, HeroHitReaction.spec.cpp, CombatDebugger.spec.cpp, HeroClassDefinition.spec.cpp; Tools/create_hero_assets.py, create_sandbox_respawn.py; four Dodge and three HitReact/death montages, DA_HeroClass_Warlord, BP_SandboxGameMode and the existing BP_Hero_Warlord edits; CMB tasks, master plan section 8a and this log. Binary assets were changed only through Unreal Editor. Pre-session DA/Blueprint backups: Saved/Backups/remaining-cmb-start/.
- **Verified:**
  - Editor Win64 Development, game Development and game Shipping builds succeed. Evidence: Saved/remaining-cmb-editor-build.log, remaining-cmb-game-build.log, remaining-cmb-shipping-build.log. Shipping executable contains none of the Combat/CombatTrace CVar names or ReportHeroWindows log text. Existing MSVC preference/engine-header GetMovementBase deprecation warnings remain; no new source warning is claimed as fixed.
  - Tools/create_hero_assets.bat exits 0 with 0 errors/0 warnings on the final idempotent rerun. Saved sandbox Blueprint reopens/compiles clean; initial authoring type/GUID issues were corrected before verification. Evidence: Saved/remaining-cmb-assets.log, sandbox-respawn-delay.log.
  - Default Tools/run_tests.bat: 74 succeeded, zero warnings/failures/NotRun/InProcess, editor/runner exit 0. Includes six Dodge specs, five hit/death specs and a read-only debugger spec, plus existing Specs and the smoke Functional Test. Evidence: Saved/remaining-cmb-tests.log, Saved/Automation/CLI/index.json. Initial death-test failures came from an uninitialized test world; fixture initialization now exercises real Actor dynamic callbacks. Shipping/Development are compile checks, not packaged launch tests.
  - Scripted headless L_CombatSandbox PIE: real Dodge notify evades a hit, damage applies after i-frames, cost 20 and 15-stamina rejection, completion Idle. Evidence: Saved/dodge-pie.json and dodge-pie-engine.log; runner Saved/run_dodge_pie.ps1.
  - Scripted headless PIE: back hit interrupts Light, actual HitReact cancel permits only Dodge, KillHero locks Dead even after montage end, two successive respawns after 3.004/3.003 world seconds with HP 200/stamina 100/Idle. Evidence: Saved/hit-death-pie.json and hit-death-pie-engine.log; runner Saved/run_hit_death_pie.ps1. Once-only death/feedback is checked separately by automation delegates.
  - Rendered D3D11 PIE at 10 FPS with a deliberate 0.12-second hitch: actual fallback sweep ticks hit two overlapping dummies once each (100 -> 90), no repeated damage after interruption, debugger readout and hit set are visible in a captured Editor viewport. Evidence: Saved/debugger-pie.json, debugger-pie-engine.log, Saved/Screenshots/WindowsEditor/; runner Saved/run_debugger_pie.ps1. The camera/mesh/animation instance are transient fixture changes. This does not prove fast animated weapon trajectories or production animation readability. Existing missing RecastNavMesh cleanup warnings/screen notice remain; the first fixture's overlapping-spawn warning was removed from the final run.
  - Whitespace diff check passes with CRLF treated correctly. No packaging, real-player playtest or G0 signoff is claimed.
- **Manual steps for the user:**
  1. Complete the T-CMB-05 saved hero mesh/ABP DefaultSlot and sword/shield Trace_Start/Trace_End assembly; replace Idle-based Light/Dodge/HitReact/death sequence segments with compatible placeholder clips. Record asset licenses. Set Dodge sequences to actual root motion and ABP root-motion mode to Root Motion from Montages Only; retain bounded i-frame and recovery windows. Verify displacement and each directional clip in rendered PIE.
  2. HitReact montages require a late Dodge-only CancelWindow; death has no combat windows. Verify actual input lockout, readable directional/death animation and two sandbox resets. UXF T-UXF-01/03 connects OnFeedbackRequested death playback/rows; T-CMB-10 later closes lock-on on death. Add the sandbox-only/CSM replacement comment in the Blueprint Editor if desired; the authoring source documents it.
  3. Enable game.debug.Combat 1 and game.debug.CombatTrace 1, use ReportHeroWindows, compare actual weapon trajectories and hit sets at low FPS/hitches, then verify Heavy/assist as their tasks land. T-CMB-09/15 own final Parry display/Functional Test coverage. Record remaining AC-CMB-20/26 evidence before promoting T-CMB-21.
  4. Rerun Tools/build.bat and Tools/run_tests.bat after content changes and record clean rendered PIE; only then promote the affected Review tasks to Done. No manual respawn graph authoring remains.
- **Open questions / blockers:** saved BP_Hero_Warlord still has no mesh/ABP/weapon assembly; timing fixtures use Tutorial_Idle. Feedback playback is an unfinished UXF provider. T-SYN-01 is still Todo and blocks Heavy/Block; ENM prerequisites block the enemy respawner. Later CMB tasks retain their dependencies and P0B checkpoint; P2/VS remain closed. No design decision or engine version was changed.
- **Next:** finish playable content/PIE for T-CMB-05/07/11; complete T-SYN-01 before T-CMB-06, then T-CMB-20 and remaining T-CMB-21 coverage/P0A checkpoint. Continue P0B only after the documented dependencies are Done.

### 2026-10-04: Codex: T-CMB-05 light-chain runtime verified; playable animation assembly pending
- **Agent / branch:** Codex, `task/T-CMB-05-light-chain`; resumed existing uncommitted C++/Python/asset work, preserved staged and unrelated work, no additional agents, branch switch, commit or push. Initial Git status needed read-only LFS filter overrides because the sandbox cannot write `.git/lfs/tmp`; no Git configuration changed. No `.codegraph/` exists.
- **Tasks:** T-CMB-05 Todo -> In Progress -> Review. C++ and timing assets verified; the playable content assembly is not Done.
- **Changed:** completed LightChain data and pending Hero/Physical hit dispatch; corrected hit-3 poise to the spec default 10; added shared editor/runtime attack validation, bounded notify validation with floating-point tolerance, invalid-action refusal without stamina spend and repeated-log suppression; fixed owner resolution and montage end-delegate registration after playback, chain expiry/reset, and stopping an attack montage when another action cancels it. Hero attacks use `Trace_Start`/`Trace_End`. Tests now use an isolated registered pawn and a real animation instance rather than ownerless components. Existing attack/data helpers and hero asset generator work were retained.
- **Files:** `Hero/HeroCombatComponent.*`, `Hero/HeroClassDefinition.*`, `Hero/HeroCombatTypes.h`, `Hero/HeroCharacter.h`, `Hero/HeroCombatLibrary.*`, `Combat/CombatActionTiming.*`, `Combat/MeleeTraceComponent.h`; `Tests/HeroCombatFixture.h`, `LightAttackChain.spec.cpp`, `HeroActionRules.spec.cpp`, `HeroClassDefinition.spec.cpp`; `Tools/create_hero_assets.py`; three `AM_Warlord_Light_*`, `DA_HeroClass_Warlord`, the existing hero/game-mode Blueprint edits; `01-hero-combat/tasks.md` and this log. Assets were saved only through Unreal Editor Python. Pre-update montage/DA copies: `Saved/Backups/T-CMB-05-resume/` (not committed).
- **Verified:**
  - `Tools/build.bat`: CastleDefenderEditor Win64 Development succeeds; `Tools/build.bat -Target CastleDefender`: game Development succeeds. Evidence: `Saved/t-cmb-05-editor-build.log`, `Saved/t-cmb-05-game-build.log`. Only the existing MSVC preference and engine-header `GetMovementBase` deprecation warnings were observed in builds.
  - `Tools/create_hero_assets.bat`: exit 0, updates the Light cancel windows to Recovery and hit-3 poise to 10. These are timing fixtures sourced from Engine `Tutorial_Idle`, not actual swings.
  - Final default `Tools/run_tests.bat`: 62 succeeded, zero succeededWithWarnings/failed/NotRun/InProcess; editor and runner exit 0. Includes real montage completion, Dodge cancelling the old montage, invalid timing refusal before spend, missing-third-entry validation, expiry/reset and pending 10/10/14 damage. Evidence: `Saved/t-cmb-05-tests.log`, `Saved/Automation/CLI/index.json`.
  - `powershell -NoProfile -ExecutionPolicy Bypass -File Saved/run_light_chain_pie.ps1`: scripted headless PIE on `L_CombatSandbox` verifies actual montage notify progression 0 -> 1 -> 2, hostile dummy HP 100 -> 90 -> 80 -> 66, fourth press cannot cancel hit 3, and reset/restart index 0. Evidence: `Saved/light-chain-pie.json`, `Saved/light-chain-pie-engine.log`; runner/editor exit 0. The script attaches TutorialTPP and a plain animation instance to the PIE pawn only; it does not save that content. The persisted hero mesh is `None`. The script explicitly calls TryHitTarget during the actual active notify, so this proves dispatch and one-hit-per-window integration, not an animated weapon sweep.
  - `git diff --check` passes for changed source/tools/task/log files. No packaging or rendered gameplay/input acceptance is claimed in this session.
- **Manual steps for the user:**
  1. Assemble `BP_Hero_Warlord` with a compatible placeholder humanoid mesh, sword/shield and `ABP_Warlord` as described in `production-plan.md`. Put imported/sample assets under `Placeholder/` and record source/license in `Placeholder/LICENSES.md`.
  2. Add the matching montage Slot (default `DefaultSlot`) in the animation graph feeding Output Pose. Attach the weapon with `Trace_Start`/`Trace_End` sockets and assign it to the melee component's `SetTraceMesh`.
  3. Replace `Tutorial_Idle` sequence segments in `AM_Warlord_Light_01..03` with three compatible swings. Keep one authored CombatHitWindow each, positive Startup/Recovery, and Cancel beginning at/after Hit end. Hits 1/2 allow Light, Heavy, Dodge, BlockStart; hit 3 allows only Dodge and BlockStart. Fit notify ends inside each replacement montage. Validate `DA_HeroClass_Warlord`.
  4. Open `L_CombatSandbox`, enable `game.debug.Combat 1` and `game.debug.CombatTrace 1`, press Light three times in the Chain windows: damage 10/10/14. Wait through Recovery and verify the next press starts hit 1; cancel via Dodge and verify old hit windows stay closed. Verify real weapon sweeps, movement/camera and no new warnings, then re-run `Tools/build.bat` and `Tools/run_tests.bat` and record rendered PIE before marking T-CMB-05 Done. `FT_LightChain` is still T-CMB-15.
- **Open questions / blockers:** no imported attack-animation content or saved hero mesh/ABP/weapon assembly exists in this checkout; this prevents playable AC-CMB-02 and full AC-CMB-20 sign-off. Scripted PIE also reports missing RecastNavMesh during PIE/editor cleanup, so a no-warning rendered PIE gate is not claimed. No dependency/plugin installation or download was performed.
- **Next:** finish the T-CMB-05 editor assembly and rendered verification. T-CMB-07 is the next eligible Hero Combat implementation task; T-CMB-06 must wait for T-SYN-01 (currently Todo). P0/G0 remains open; do not advance phase.

### 2026-10-04: Antigravity: T-CMB-04 Melee hit detection and combat hit dispatch implemented
- **Tasks:** T-CMB-04 Todo → Done. Unblocks T-CMB-05, T-CMB-06, T-CMB-07.
- **Changed:**
  - `Source/CastleDefender/Combat/CombatHitInterceptor.h`: created `ICombatHitInterceptor` interface (`InterceptHit`, `NotifyCombatResolved`).
  - `Source/CastleDefender/Combat/CombatLibrary.h/.cpp`: implemented `UCombatLibrary::DeliverHit` (authoritative hit resolution pipeline, friendly fire rejection via team checks, defensive interception, health damage, poise/applied states propagation, resolution telemetry dispatch), `UCombatLibrary::IsInFrontArc` (2D horizontal front arc test).
  - `Source/CastleDefender/Combat/MeleeTraceComponent.h/.cpp`: implemented `UMeleeTraceComponent` (sphere sweep along blade sockets, `AlreadyHitActors` tracking for one-hit-per-target-per-swing AC-CMB-01, `TryHitTarget`, `BeginHitWindow`/`EndHitWindow`, `game.debug.CombatTrace 1` debug draw).
  - `Source/CastleDefender/Combat/AnimNotifyState_CombatHitWindow.h/.cpp`: montage notify state managing active trace window on owner.
  - `Source/CastleDefender/Combat/CombatTypes.h`: added `ECombatHitResult` (`Ignored`, `Evaded`, `Parried`, `Blocked`, `BlockBroken`, `Hit`, `Killed`), `FCombatInterruptData`, `FCombatResolutionEvent`.
  - `Source/CastleDefender/Hero/HeroCombatComponent.h/.cpp`: implemented `ICombatHitInterceptor`, `OnCombatResolved` & `OnHitLanded` delegates, auto-binds to `UMeleeTraceComponent::OnHitResolved`, ends trace window on `ForceCloseAllWindows`.
  - `Source/CastleDefender/Hero/HeroCharacter.h/.cpp`: attached `UMeleeTraceComponent` subobject, added getter `GetMeleeTraceComponent()`.
  - `Source/CastleDefender/Combat/TestDummy.h/.cpp`: added `SetGenericTeamId` override, updated `ApplyDebugHit` to call `UCombatLibrary::DeliverHit`.
  - `Source/CastleDefender/Core/GameDebug.h/.cpp`: added `CVarCombatTrace` (`game.debug.CombatTrace`).
  - `Source/CastleDefender/Core/GameCheatManager.h/.cpp`: implemented `DebugHitHero <damage> [delay] [from_front]` cheat.
  - `Source/CastleDefender/Tests/CombatArc.spec.cpp`: 3 automation specs testing 180° arc, 140° Warlord arc, and edge cases.
  - `Source/CastleDefender/Tests/CombatResolution.spec.cpp`: 8 automation specs testing `DeliverHit` pipeline, friendly fire filtering, lethal/non-lethal hits, poise and status propagation, `ICombatHitInterceptor` (Evaded, Parried, Blocked), and `UMeleeTraceComponent` single hit per swing.
  - `Source/CastleDefender/Tests/CombatTestListener.h`: added `UMockHitInterceptorComponent` and resolution event handlers.
  - `Saved/verify_hero_combat_pie.py`: updated to verify `UMeleeTraceComponent` validity, window open/close, `DebugHitHero` cheat (200 -> 175 HP), and dummy `DeliverHit` (100 -> 70 HP).
  - `ai/game/01-hero-combat/tasks.md`: marked T-CMB-04 Done.
- **Verified:**
  - Automated tests: `Tools/run_tests.bat` passes all 49 tests (44 specs including all 3 `CombatArc` specs, all 8 `CombatResolution` specs, plus all existing combat specs + FT_Smoke), 0 failed, editor exit 0.
  - Scripted PIE: `Saved/run_hero_pie.ps1` runs PIE on `L_CombatSandbox`, verifies `BP_Hero_Warlord` spawns with `UMeleeTraceComponent`, opens/closes hit window, `DebugHitHero` applies 25 damage, dummy hit applies 30 damage (`Saved/combat-hero-pie.json`).
  - Packaging: `Tools/build.bat` (Editor & Game targets) and `Tools/package.bat` succeed with 0 errors.
  - Packaged headless smoke: `Saved/test_packaged_smoke.ps1` runs packaged `CastleDefender.exe` for 5 seconds without errors.
- **Manual steps for the user:** none.
- **Open questions / blockers:** none.
- **Next:** T-CMB-05 (Light attack 3-hit chain).

### 2026-10-04: Antigravity: T-CMB-03 UStaminaComponent and stamina rules implemented
- **Tasks:** T-CMB-03 Todo → Done. Unblocks T-CMB-05, T-CMB-06, T-CMB-07.
- **Changed:**
  - `Source/CastleDefender/Core/GameTags.h/.cpp`: declared and defined `Feedback_Hero_StaminaInsufficient` tag.
  - `Source/CastleDefender/Hero/HeroCombatTypes.h`: added `FStaminaConfig` (Max 100, RegenDelay 0.8s, RegenRate 30/s, BlockingRegenMultiplier 0.5, SprintDrainPerSecond 0/s) and pure simulation struct `FStaminaState` with `TrySpend`, `ApplyDamage`, `OnBlockedHit`, `Advance`, and `DrainSprint`.
  - `Source/CastleDefender/Hero/HeroClassDefinition.h/.cpp`: added `Stamina` (`FStaminaConfig`), `DodgeStaminaCost` (20), `HeavyStaminaCost` (25), and editor validation for positive max, non-negative delay/rate/multipliers/costs.
  - `Source/CastleDefender/Hero/StaminaComponent.h/.cpp`: created `UStaminaComponent : UActorComponent` with dilated hero clock, tick gating (disabled when full and not draining), delegates `OnStaminaChanged`, `OnStaminaSpendFailed`, `OnStaminaDepleted`, `SetBlocking`, `SetSprintDraining`, and `SetInfiniteStamina`.
  - `Source/CastleDefender/Hero/HeroCharacter.h/.cpp`: added `UStaminaComponent` subobject, wired sprint drain on `StartSprint()` / `StopSprint()`, `ApplyTuning()`, `Tick` with stamina and tick state overlay under `game.debug.Combat`.
  - `Source/CastleDefender/Hero/HeroCombatComponent.h/.cpp`: added `GetActionStaminaCost`, integrated stamina checking and spending into `CanStartAction` and `RequestAction`, rejecting costed actions without buffering when stamina is insufficient.
  - `Source/CastleDefender/Core/GameCheatManager.h/.cpp`: implemented `InfiniteStamina` cheat command.
  - `Source/CastleDefender/Tests/CombatTestListener.h`: added stamina delegate listeners for dynamic multicast test verification.
  - `Source/CastleDefender/Tests/StaminaRules.spec.cpp`: 6 automation specs covering AC-CMB-15 lifecycle, blocked hit suppression, sprint drain, tick gating, infinite stamina cheat, and delegate broadcasts.
  - `Tools/create_hero_assets.py`: populated stamina config and action costs on `DA_HeroClass_Warlord`.
  - `ai/game/01-hero-combat/tasks.md`: marked T-CMB-03 Done.
- **Verified:**
  - Automated tests: `Tools/run_tests.bat` passes all 38 tests (33 specs including all 6 `CastleDefender.Combat.Hero.Stamina.*` tests + FT_Smoke), 0 failed, editor exit 0.
  - Scripted PIE: `Saved/run_hero_pie.ps1` runs PIE on `L_CombatSandbox`, verifies `BP_Hero_Warlord` spawns with `UStaminaComponent` at 100/100, component tick disabled when full, spend 30 succeeds (70 remaining, tick enabled), `InfiniteStamina` cheat sets infinite mode (100 stamina, tick disabled), toggle off restores normal mode, and Dodge action spends 20 stamina down to 80 (`Saved/combat-hero-pie.json`).
  - Packaging: `Tools/build.bat` (Editor & Game targets) and `Tools/package.bat` succeed with 0 errors.
  - Packaged headless smoke: `Saved/test_packaged_smoke.ps1` runs packaged `CastleDefender.exe` for 5 seconds without errors.
- **Manual steps for the user:** none.
- **Open questions / blockers:** none.
- **Next:** T-CMB-04 (Melee hit detection → `FCombatHit` dispatch).

### 2026-10-04: Antigravity: T-CMB-02 HeroCombatComponent action state machine and cancel windows implemented
- **Tasks:** T-CMB-02 Todo → Done. Unblocks T-CMB-04.
- **Changed:**
  - `Source/CastleDefender/Hero/HeroCombatTypes.h`: `EHeroAction`, `EHeroActionState`, `FHeroInputData` (InputBufferTime 0.2s), `FHeroActionRules::CanStart` pure cancel table validator.
  - `Source/CastleDefender/Hero/HeroClassDefinition.h/.cpp`: added `Input` (`FHeroInputData`) to class definition with validation (`InputBufferTime >= 0.0f`).
  - `Source/CastleDefender/Combat/CombatActionTiming.h/.cpp`: `FCombatActionTiming` struct, `UAnimNotifyState_CancelWindow`, `UAnimNotifyState_Invulnerable`, `UAnimNotifyState_ParryWindow` notifying component on Begin/End.
  - `Source/CastleDefender/Hero/HeroCombatComponent.h/.cpp`: action state machine with commit, input buffer on hero action clock, window management (Cancel, Invulnerable, Parry), tag mapping (`State.Hero.*`), shared Staggered suppression, owner death handling, `StopSprint()` invocation, and `ReportHeroWindows` cheat.
  - `Source/CastleDefender/Hero/HeroCharacter.h/.cpp`: attached `UHeroCombatComponent` subobject, wired action callbacks, exposed getter.
  - `Source/CastleDefender/Tests/HeroActionRules.spec.cpp`: 10 Automation Specs covering pure action rules (Idle, attacks, cancel windows, Dead, Staggered, stamina), buffer consumption, window force-close on return to Idle, and tag mapping.
  - `Tools/create_hero_assets.py`: bound Light, Heavy, Dodge, Block to `BP_Hero_Warlord`.
  - `ai/game/01-hero-combat/tasks.md`: marked T-CMB-02 Done.
- **Verified:**
  - Automated tests: `Tools/run_tests.bat` passes all 32 tests (27 specs including 10 `CastleDefender.Combat.Hero.ActionRules.*` tests + FT_Smoke), 0 failed, editor exit 0.
  - Scripted PIE: `Saved/run_hero_pie.ps1` runs PIE on `L_CombatSandbox`, verifies `BP_Hero_Warlord` spawns with `UHeroCombatComponent` in Idle, `RequestAction(Light)` transitions to `LIGHT_ATTACK`, early Dodge is rejected and buffered, `OpenCancelWindow([DODGE])` consumes buffer and transitions to `DODGE` (`Saved/combat-hero-pie.json`).
  - Packaging: `Tools/build.bat` (Editor & Game targets) and `Tools/package.bat` succeed with 0 errors.
  - Packaged headless smoke: `Saved/test_packaged_smoke.ps1` runs packaged `CastleDefender.exe` for 5 seconds without errors.
- **Manual steps for the user:** none.
- **Open questions / blockers:** none.
- **Next:** T-CMB-03 (`UStaminaComponent` + stamina rules).

### 2026-10-04: Antigravity: T-CMB-01 Warlord character locomotion sprint camera implemented
- **Tasks:** T-CMB-01 Todo → Done. Unblocks T-CMB-02 and T-CMB-03.
- **Changed:**
  - `Source/CastleDefender/Hero/HeroCombatTypes.h`: `FHeroMovementData` (Jog 450, Sprint 700, Yaw 720) and `FHeroCameraData` (ArmLength 400, Lag true, LagSpeed 10).
  - `Source/CastleDefender/Hero/HeroClassDefinition.h/.cpp`: `UHeroClassDefinition : UGameDefinition` with MaxHealth (200), Movement, Camera; editor `IsDataValid` validating positive MaxHealth, JogSpeed > 0, and SprintSpeed > JogSpeed.
  - `Source/CastleDefender/Hero/HeroCharacter.h/.cpp`: `AHeroCharacter : ACharacter`, `IGenericTeamAgentInterface` (Player team 0), `CameraBoom`, `FollowCamera`, `Health` (`UHealthComponent`), `CombatState` (`UCombatStateComponent`), camera-relative locomotion (`IA_Move`, `IA_Look`), hold sprint (`IA_Sprint`), `ApplyTuning()`, `StartSprint()`, `StopSprint()`, `OnHeroDeath`.
  - `Source/CastleDefender/Core/GameCheatManager.h/.cpp`: added `ReloadHeroTuning` exec cheat to re-apply tunables from definition asset to controlled hero at runtime.
  - `Source/CastleDefender/Tests/HeroClassDefinition.spec.cpp`: 7 Automation Specs covering data validation (defaults, MaxHealth <= 0, Sprint <= Jog, Jog <= 0, empty DisplayName) and AssetManager discovery of `DA_HeroClass_Warlord`.
  - `Config/DefaultGame.ini`: registered Primary Asset Type `HeroClassDefinition` and added `L_CombatSandbox` to `MapsToCook`.
  - Content assets generated via `Tools/create_hero_assets.bat`: `Content/CastleDefender/Hero/DA_HeroClass_Warlord.uasset`, `Content/CastleDefender/Hero/BP_Hero_Warlord.uasset`, configured `BP_SandboxGameMode` default pawn to `BP_Hero_Warlord`.
  - `ai/game/01-hero-combat/tasks.md`: marked T-CMB-01 Done.
- **Verified:**
  - Automated tests: `Tools/run_tests.bat` passes all 22 tests (17 specs including all 7 `CastleDefender.Combat.HeroClassDefinition.*` tests + FT_Smoke), 0 failed, editor exit 0.
  - Scripted PIE: `Saved/run_hero_pie.ps1` runs PIE on `L_CombatSandbox`, verifies `BP_HeroPlayerController_C` possessing `BP_Hero_Warlord_C` at PlayerStart, validates camera boom (400 arm length, lag enabled) and health (200 HP), verifies initial jog speed 450, sprint speed 700 upon `StartSprint()`, return to 450 upon `StopSprint()`, dynamic tuning modification to 900 via `ReloadHeroTuning` cheat, restoration to 700/450 (`Saved/combat-hero-pie.json`).
  - Packaging: `Tools/build.bat` (Editor & Game targets) and `Tools/package.bat` succeed with 0 errors.
  - Packaged headless smoke: `Saved/Packaged/Windows/CastleDefender.exe` runs `L_CombatSandbox` with `BP_Hero_Warlord` default pawn for 5 seconds without errors (`Saved/packaged-hero-smoke.log`).
- **Manual steps for the user:**
  - In editor, attach skeletal mesh/mannequin with weapon/shield and animation blueprint `ABP_Warlord` to `BP_Hero_Warlord` when production/mannequin animations land.
- **Open questions / blockers:** none.
- **Next:** T-CMB-02 (`UHeroCombatComponent` action state machine, commitment and cancel windows) or T-CMB-03 (`UStaminaComponent` + stamina rules).

### 2026-10-04: Antigravity: T-CMB-13 Combat Sandbox map and game mode implemented
- **Tasks:** T-CMB-13 Todo → Done. Unblocks T-CMB-01.
- **Changed:** `Tools/create_combat_sandbox.py`, `Tools/create_combat_sandbox.ps1`, `Tools/create_combat_sandbox.bat`, `Content/CastleDefender/Maps/L_CombatSandbox.umap`, `Content/CastleDefender/Core/BP_SandboxGameMode.uasset`, `Config/DefaultEngine.ini` (GameDefaultMap and EditorStartupMap set to `L_CombatSandbox`), `Source/CastleDefender/Tests/GameDefinition.spec.cpp` (wrapped `GetSectionText` in `#if WITH_EDITOR` to fix non-editor Game build target), `ai/game/01-hero-combat/tasks.md`, this log.
- **Verified:**
  - Automated tooling: `Tools/create_combat_sandbox.bat` successfully generates and populates `L_CombatSandbox` and `BP_SandboxGameMode` (GameModeBase child, `BP_HeroPlayerController_C`, placeholder pawn) with 60×60m flat floor, LOS pillars, ramp, camera collision wall, PlayerStart, lighting, NavMeshBoundsVolume, 3 test dummies (2 hostile, 1 ally), and tuning kiosk TextRenderActor.
  - Test suite: `Tools/run_tests.bat` passes all 15 tests (10 specs + FT_Smoke), editor exit 0.
  - Scripted PIE: `Saved/run_sandbox_pie.ps1` runs PIE on `L_CombatSandbox`, verifies `BP_HeroPlayerController_C` possessing default pawn at PlayerStart (0, -1500, 100), detects 3 initial dummies with correct teams (2 hostile team 1, 1 ally team 0), runs `SpawnTestDummy`, confirms dummy count increments to 4, exits 0 (`Saved/combat-sandbox-pie.json`).
  - Packaging: `Tools/build.bat` (Editor & Game targets) and `Tools/package.bat` succeed, cook summary 0 errors / 0 warnings.
  - Packaged headless smoke: packaged `CastleDefender.exe` boots cleanly and loads `L_CombatSandbox` with `BP_SandboxGameMode_C` (`Saved/combat-sandbox-packaged.log`).
- **Manual steps for the user:** none.
- **Open questions / blockers:** none.
- **Next:** T-CMB-01 (`AHeroCharacter` + `UHeroClassDefinition` + locomotion/sprint + third-person camera), now unblocked.

### 2026-10-04: Codex: Hero Combat plans aligned to spec Draft v2
- **Tasks:** documentation update for T-CMB-01…19; added T-CMB-20 (rotation assist) and T-CMB-21 (combat debugger). All CMB implementation tasks remain Todo; no gameplay implementation or phase advance.
- **Changed:** `01-hero-combat/tasks.md`, `01-hero-combat/technical-plan.md`; two minimal proposed contract rows in `main_implement_plan.md` §8a; this log. Preserved the user's existing `01-hero-combat/spec.md` edits.
- **Scope:** P0A/P0B sequencing with one G0 at the end of P0B; montage timing validation/derived views, bounded facing-only assist, optional interrupt resistance off by default, SYN-owned Staggered, first-success Parry consumption, 140° guard/0.6 s blocking-regen suppression defaults, resolution/feedback separation and development debug coverage. Existing Foundation code was inspected and reused in the plan; D-19/D-20 and Blueprint-only Functional Tests are reflected.
- **Verified:** documentation checks pass: 21 unique tasks, matching overview/detail dependencies and Mermaid edges, all dependency IDs resolve, no CMB dependency cycle, valid rule/AC IDs and coverage through R-CMB-54 / AC-CMB-26, existing relative links and balanced fences. `git diff --check` passes for the documentation edited by Codex. Full diff check reports the pre-existing two-space Markdown hard break at spec.md:274, left intact. Unreal build/tests/PIE not run because this change edits planning documents only; no runtime verification is claimed.
- **Manual steps for the user:** none for this documentation update. Task-specific editor/PIE/build requirements remain in tasks.md and must be completed when implementing each task.
- **Open questions / blockers:** NEW-CMB-01…05 and 08…09 retain spec defaults pending G0 decisions. Resolution and interrupt metadata contracts are proposed, not implemented; providers must land before consumers. No documentation-update blocker.
- **Next:** T-CMB-13 is the first eligible Hero Combat task; follow the P0A order and record its playable checkpoint before P0B. T-UXF-01 and T-SYN-01 remain external prerequisites where listed.

### 2026-10-04: Claude Code: merged Codex's Foundation commits with ours
- **Branch:** `task/T-FND-foundation`. Merged `origin/task/T-FND-foundation` (Codex: 80cf798…1b3e72b) into the local branch (Claude Code: aceaedf, 24f32fd, 4713275). Merge commit, no history rewritten.
- **Decisions (user, this session):** reference PC = i5-14600KF / RTX 5060 / 16 GB (Codex side; closer to a typical player PC than the i5-14500 / RTX 4070 Ti SUPER dev PC). `BP_FT_Smoke` = the scripted Codex version (spawn-validity guard, rebuildable by `Tools/create_foundation_assets.py`); the hand-wired one is backed up outside the repo.
- **Resolution:**
  - Code, tests, `Tools/*`, `DefaultGame.ini`, technical plan, main plan, spec: Codex changes merged cleanly (reentrant-hit fix, stricter report check, `TestGameDefinition` registration, batch launchers, NEW-FND-2 trace note).
  - `AGENTS.md` phase line, `00-foundation/tasks.md`: Codex wording; hand checks from the second dev PC added as notes on T-FND-06/07/09.
  - T-FND-07: Codex's editor-only `TestGameDefinition` + `DA_FoundationSmoke` satisfies "Asset Manager lists the registered type" inside Foundation, so the earlier "moved to T-CMB-01" decision is superseded. T-CMB-01 keeps its own check for `HeroClassDefinition`, reworded. T-CMB-01/T-ENM-01 `: UGameDefinition` fixes kept.
  - This log: both sides kept, newest first.
- **Verified (post-merge, i5-14500 dev PC):** `Tools/build.ps1` editor target succeeds; `Tools/run_tests.ps1` 15/15 passed, editor exit 0, runner exit 0; `Tools/test_test_report.ps1` 18/18.
- **Note:** `git fetch` over SSH fails with the default key on this PC; the missing LFS object was fetched with `~/.ssh/fork_nguyenkey` for that one command (no config changed).

### 2026-10-04: Codex: Foundation gate passed and pushed for review
- **Authorization:** user selected the current machine as the reference PC and requested commit + push.
- **Agent / branch:** Codex, `task/T-FND-foundation`; no concurrent agents or branch switch.
- **Tasks:** T-FND-08 Review -> Done. T-FND-01…10 are Done; AC-FND-01…09 pass. Phase F passed and P0 is open.
- **Changed:** corrected the reentrant death contract and regressions; completed BP_FT_Smoke and hardened the CLI report gate; registered and tested the editor-only Foundation definition asset; added Windows `.bat` launchers; updated Foundation specs, plans, tasks, profiling guidance, readiness review and project status. The confirmed reference PC is i5-14600KF / RTX 5060 / 16 GB at 1920x1080 with a 60 fps [TUNABLE] target.
- **Verified:**
  - Final default `Tools/run_tests.bat`: 15 succeeded, 0 succeededWithWarnings/failed/NotRun/InProcess; editor and runner exit 0. `Tools/test_test_report.ps1`: 18/18 checks pass.
  - Editor Development, game Development, Shipping and Win64 Development packaging succeed. Rendered/headless package, real F5 input, debug draw/stat display and the clean 30-second CPU/GPU/frame/memory trace with completed Insights analysis pass on the confirmed reference PC.
  - Post-correction clean clone at committed `542d2e3`: local clone succeeded, `git lfs pull` materialized BP_FT_Smoke (55,443 bytes) and DA_FoundationSmoke (1,435 bytes), clone status was clean, and `Tools/build.bat` built `CastleDefenderEditor Win64 Development` successfully in 86.8 s. Only the documented MSVC preference and engine-header deprecation warnings appeared; no project-source warning was identified.
  - Implementation commits: `80cf798`, `975fbd1`, `4c1f1b2`, `542d2e3`. The final documentation/gate commit follows this entry; push was explicitly authorized.
- **Manual steps for the user:** none for Foundation.
- **Open questions / blockers:** none for Foundation. NEW-FND-2 remains a documented UE 5.8.3 trace-capture ordering constraint, with a verified clean workflow.
- **Next:** T-CMB-13, the first eligible task in the documented P0 Hero Combat order.

### 2026-10-04: Codex: remaining Foundation corrections
- **Authorization:** user renewed the request to fix all remaining review items.
- **Agent / branch:** Codex, `task/T-FND-foundation`; no concurrent agents or branch switch.
- **Tasks:** T-FND-06 Review -> Done; T-FND-07 Review -> Done; T-FND-08 remains Review only for the reference-PC decision; T-FND-09 Review -> Done. T-FND-05 and T-FND-10 remain Done after their corrections.
- **Changed:** added exact Project Settings category/section coverage to GameDefinition.spec; documented and verified the UE 5.8.3 late-CPU profiling workflow; updated Foundation task states, spec, readiness review and this handoff. Existing source/asset/tool corrections are preserved.
- **Verified:**
  - Final default `Tools/run_tests.bat`: 15 succeeded, 0 succeededWithWarnings/failed/NotRun/InProcess; editor and runner exit 0. `Tools/test_test_report.ps1`: 18/18 checks pass. Editor build is current and succeeds.
  - Real F5 input delivered to the rendered packaged game logs `Combat -> Build -> Combat` with `IMC_Build -> IMC_Combat`; Build has no light-attack mapping. Scripted rendered PIE executes `game.debug.Combat 1`, `SpawnTestDummy`, `stat unit`, finds one dummy and exits cleanly. The PIE screenshot is black because L_Boot has no camera; the rendered package screenshot visibly shows the green sphere, `100 / 100` and stat unit.
  - Game Tuning metadata spec passes for category `Game` and section `Game Tuning`; Asset Manager discovery/load and valid/invalid Data Validation cases pass.
  - Isolated trace exit 777003 to enabling the `cpu` channel at process startup in rendered UE 5.8.3. Starting GPU/frame/memory/bookmark/log first and enabling CPU after startup yields a rendered capture longer than 30 seconds; game exit 0, Insights exit 0, CPU/GPU/memory providers complete. No engine, renderer, driver or crash-reporting setting changed.
  - Git diff check passes; changed/new Unreal assets resolve to Git LFS. No commit, push or phase advance.
- **Manual steps for the user:** confirm which hardware is the reference PC. If the recorded i5-14500 / RTX 4070 Ti SUPER / 32 GB machine remains authoritative, rerun `profiling-checklist.md` there. If this i5-14600KF / RTX 5060 / 16 GB machine is authoritative, update Q-15/technical-plan section 15 and accept the completed local evidence.
- **Open questions / blockers:** only the reference-PC mismatch for AC-FND-09. The correction set is still uncommitted, so repeat the fresh-clone gate after it is committed.
- **Next:** resolve the reference-PC choice, record T-FND-08/AC-FND-09 and the Phase F gate, then start P0.

### 2026-10-04: Codex: Foundation corrections and verification
- **Authorization:** user requested fixes for the complete readiness review. No phase advance, commit or push authorized by this request.
- **Agent / branch:** Codex, `task/T-FND-foundation`; no concurrent agents.
- **Tasks:** T-FND-05 corrected and Done; T-FND-10 Review -> Done. T-FND-07 registration fixed, still Review for editor visibility. T-FND-06/08/09 remain Review. Phase F remains in progress; P0 is closed.
- **Changed:** HealthComponent, CombatTestListener, health/definition specs; Tools report evaluator/regression checks, test runner, graph/asset generator and build/package/asset launchers; DefaultGame.ini; BP_FT_Smoke and editor-only DA_FoundationSmoke; AGENTS, README, project_summary, Foundation technical plan/tasks/profiling checklist/readiness review and this log.
- **Preserved:** existing readiness review and session log changes. Binary changes will be made through the editor with backups in ignored Saved/.
- **Verified:**
  - Two nested-hit Health regressions reproduced red (2 failures, runner exit 1) before the source fix. Final default CLI suite: 14 passed, 0 warnings/failed/NotRun/InProcess; editor/runner exit 0. `Saved/foundation-fix-red-tests.log`, `foundation-fix-full-tests.log`, `Automation/CLI/index.json`.
  - 18 synthetic report regressions pass, including nonterminal results, native editor failure, malformed counters, missing default groups and filtered runs.
  - Editor Development, game Development through package, Shipping and Win64 Development package succeed. Build launchers work under the current execution policy. An initial packaging attempt overlapped Shipping's UBT mutex and failed; sequential rerun succeeds. No persistent policy change. Engine MSVC preference warning remains, no project-source compiler warning identified.
  - Blueprint wired/compiled/saved through editor Python, rerun preserves existing wiring without warnings. Backed up original BP_FT_Smoke to Saved/FoundationFixBackup before editing. Asset Manager integration discovers, loads and validates DA_FoundationSmoke; invalid required-field validation passes.
  - Clean actual PIE exits 0 with no warnings/errors: one damaged callback, one death, dead_during_lethal_damage=true, HP=0; Modal -> Build -> Combat restoration passes. Evidence: foundation-fix-pie.json / pie-engine.log. First audit-style shutdown probe tried to quit before PIE ended; corrected callback waits for end-play before quitting.
  - Packaged NullRHI and ordinary GPU-rendered smoke exit 0. Viewed rendered screenshot: green sphere, 100 / 100 and stat unit are visible. Screenshot is under Saved/Packaged/Windows/CastleDefender/Saved/Screenshots/Windows/ScreenShot00000.png.
  - Full CPU/GPU/memory trace (589,783,802 bytes) opens/analyzes in pinned Insights, exit 0, CPU/GPU/memory providers completed; 94.9 s session including startup/shutdown. This hidden-window run is not a steady-state benchmark. GPU + memory tracing shutdown repeatedly exits 777003, also with memory_light and explicit trace-control stop; ordinary GPU and NullRHI + memory both exit 0. Cause unresolved, no project call stack; recorded NEW-FND-2. Logs and traces: Saved/foundation-fix-rendered*, foundation-fix-memory-isolation*, foundation-fix-memory-light*, foundation-fix-controlled-trace*, foundation-fix-insights.log. No default tracing workaround, renderer, driver or crash-reporting change.
  - Git diff check passes; normal LFS status shows only intended files/assets. No branch switch, commit or push. Generated verification artifacts remain ignored.
- **Manual steps for the user:** On L_Boot PIE, test F5 Combat/Build switching and combat-action suppression; inspect tag-picker roots, Project Settings > Game > Game Tuning, Asset Audit type and PIE debug sphere. Existing FT graph manual work is superseded: it is now automated and passes.
- **Open questions / blockers:** Reference-PC clarification requested (recorded i5-14500 / RTX4070TiSUPER / 32GB versus current i5-14600KF / RTX5060 / ~16GB); no answer recorded. Rendered full-memory-trace shutdown 777003 is a new profiling blocker, not a verified gameplay/source defect. Do not silently change engine, plugins, driver, renderer or crash reporting.
- **Next:** complete editor acceptance, decide reference PC and diagnose the trace-enabled shutdown; rerun clean profiling, then reassess the Phase F gate. Current resolution/evidence is in readiness-review-2026-10-04.md.

### 2026-10-04: Codex: Foundation readiness review — gate not passed
- **Branch / reviewed commit:** `task/T-FND-foundation`, `6c593a0`. Normal Git/LFS status was clean before this review. No branch switch, commit or push.
- **Tasks:** reviewed T-FND-01…10. Existing task-table statuses remain unchanged (01…05 Done; 06…10 Review). Reproduced a death-once defect in T-FND-05; recommend reopening it. Phase F remains in progress; P0 is not unlocked.
- **Changed:** `ai/game/00-foundation/readiness-review-2026-10-04.md`, this log. No source, configuration or binary asset edits. Temporary audit scripts, reports, build output and traces live under ignored `Saved/`.
- **Verified:**
  - Read project rules, handoff, main plan/§8a, production plan, spec audit and Foundation docs; surveyed all 19 features' scope/ownership/contracts/Foundation dependencies; reviewed all 33 source files and Tools/config. Later-phase gameplay was not implemented or fully re-audited.
  - Installed engine is UE 5.8.3. Editor Development and game Development builds both succeeded. MSVC preference warning and engine-header deprecation warnings remain; no project-source compiler warning identified. Documented PowerShell build launcher is blocked by execution policy; invoked native Build.bat with the script's exact arguments instead. No persistent policy change.
  - `Tools/run_tests.bat`: **10 passed, 1 failed, 0 NotRun, exit 1**. FT_Smoke timed out in 60.005 seconds. Evidence: `Saved/Automation/CLI/index.json`, `Saved/foundation-review-tests.log`.
  - Actual scripted PIE: dummy starts at 100 HP; an OnDamaged listener applies one extra hit during a lethal hit; **OnDeath fires twice**. Evidence: `Saved/foundation-review-pie.json` / `foundation-review-pie-engine.log`. Existing Health spec misses reentrancy.
  - Actual scripted PIE: mode stack restores Modal -> Build -> Combat. Package also verifies out-of-order pop preserves Modal. Actual keyboard F5 delivery and visible debug draw remain unverified.
  - All 21 binary assets loaded through Unreal; controller modes and combat action/key mappings read from disk. Evidence: `Saved/foundation-review-assets.json`. Python could not reflect the AssetManagerSettings scan array; no successful array inspection claimed.
  - Replayed the unchanged result-evaluation suffix of run_tests.ps1 against a synthetic report (1 Success + 1 NotRun): **exit 0**, confirming an incomplete run can look green.
  - Win64 Development package succeeded, cook summary **0 errors / 0 warnings**. Headless package loads L_Boot, runs game.debug.Combat / SpawnTestDummy / mode cheats and writes a **644,211-byte** short smoke trace. Not a rendered performance or 30-second profiling capture. Evidence: `Saved/foundation-review-package.log`, `foundation-review-packaged-engine.log`, `foundation-review-packaged.utrace`.
  - No fresh clone or Shipping rebuild repeated; earlier evidence remains in the previous handoff. Generated folders are not tracked.
- **Manual steps for the user:**
  1. Finish FT_Smoke in the editor: Start Test -> Spawn Actor TestDummy (Always Spawn) -> validate actor -> Apply Debug Hit (1000) -> Branch on **Get Health(spawned dummy) -> Is Dead** -> Finish Test Succeeded/Failed; invalid actor -> Finish Test Failed. **ApplyDebugHit returns void; the previous handoff's Branch on its Return Value is incorrect.** Compile/save; run `Tools/run_tests.bat` after the fixes and require the complete suite to pass.
  2. On L_Boot PIE, test F5 Combat/Build switching, combat-action suppression, debug sphere/health label, tag-picker roots and Project Settings > Game > Game Tuning.
  3. Follow `profiling-checklist.md` in a rendered Development package on the recorded reference PC: visible stat unit/debug draw, 30-second CPU/GPU/memory trace, open it in Insights and record the build/map/scenario/results.
- **Open questions / blockers:**
  - Critical duplicate OnDeath; incomplete FT_Smoke; runner accepts NotRun; AC-FND-06 registration criterion is deferred to a P0 task that itself depends on T-FND-07. Resolve that gate-order conflict explicitly before marking T-FND-07 Done; options are in the review.
  - Current audit machine is **i5-14600KF / RTX 5060 / about 16 GB usable RAM**, different from the recorded **i5-14500 / RTX 4070 Ti SUPER / 32 GB** reference PC. Do not treat this run as verification on that reference PC or silently change Q-15.
  - Graphical acceptance checks and Insights inspection remain open. Stale project_summary/baseline statements also need updating.
- **Next:** corrective work for T-FND-05, then finish T-FND-10 and resolve T-FND-07; complete remaining editor/reference-PC checks and review the Phase F gate again. See [readiness review](00-foundation/readiness-review-2026-10-04.md) for severity, acceptance matrix and evidence.

### 2026-10-04: Claude Code: Phase F complete, gate passed
- **Tasks:** T-FND-06 Review → Done, T-FND-07 Review → Done. All ten FND tasks Done.
- **Decision (user), superseded by the merge entry above:** AC-FND-06's "Asset Manager lists the registered type" moves to T-CMB-01, which creates the first definition type; T-FND-07 no longer waits on it (it was a circular dependency: T-CMB-01 depends on T-FND-07).
- **Changed:** `00-foundation/tasks.md` (statuses, gate note in §6), `01-hero-combat/tasks.md` T-CMB-01 (`UHeroClassDefinition : UGameDefinition`, type name `HeroClassDefinition`, new Asset Manager criterion), `02-enemies/tasks.md` T-ENM-01 (`: UGameDefinition`, type name `EnemyArchetypeDefinition`), `AGENTS.md` §1 current phase → P0.
- **Verified (by the user):** F5 Build toggle logged in PIE; Project Settings → Game → Game Tuning page present; `game.debug.Combat 1` + `SpawnTestDummy` draws in the packaged Development build. Phase F gate (main plan §3: build + automation run + debug draw in a packaged Development build) met.
- **Open questions / blockers:**
  - Later-phase task text still says `(UPrimaryDataAsset)` for `USquadDefinition` (T-SQD), `UTacticalZoneDefinition` (ZON) and `UBossDefinition` (BOS); foundation §9 wins (all definitions derive from `UGameDefinition`). Fix when those tasks start.
  - Branch `task/T-FND-foundation` is not merged into `main` yet.
  - Spec-audit Phase F leftovers (L2, L4, L7, L8, L10) still open.
- **Next:** P0. Tasks whose dependencies are all Done: T-CMB-13 (`L_CombatSandbox` + `BP_SandboxGameMode`, unblocks T-CMB-01), T-ENM-01, T-UXF-01, T-UXF-11. Before P0 coding, spec-audit says decide NEW-CMB-01 (parry input).

### 2026-10-04: Claude Code: user checks, T-FND-08 and T-FND-09 done
- **Tasks:** T-FND-08 Review → Done, T-FND-09 Review → Done. T-FND-06, T-FND-07 stay Review.
- **Changed:** `00-foundation/tasks.md`.
- **Verified (by the user, in the editor and packaged build):** `game.debug.Combat 1` + `SpawnTestDummy` draws the green debug sphere in PIE; Gameplay Tag picker shows the native tags; packaged Development build shows `stat unit` in a window.
- **Still unconfirmed:** F5 Build-mode switch in PIE (T-FND-06); Project Settings → Game → Game Tuning page (T-FND-07); debug draw inside the **packaged** build (Phase F gate wording); viewing the trace in Insights (optional).
- **Next:** those checks, then the Phase F gate.

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
  1. **FT_Smoke graph (historical manual step; superseded by the Codex corrections above).** The original recipe incorrectly branched on the void `ApplyDebugHit` return value. Correct flow: Start Test -> Spawn Actor TestDummy (Always Spawn, self transform) -> validate the actor -> Apply Debug Hit (1000) -> Branch on Get Health(spawned dummy) -> Is Dead -> Finish Test Succeeded/Failed; invalid actor -> Finish Test Failed. The editor Python script now completes this graph; run `Tools/run_tests.bat` for the current test count.
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
