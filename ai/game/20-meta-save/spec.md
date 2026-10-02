# Meta Progression and Save (MET): Specification

> **Provisional — re-validate after G3.** This feature starts only after Gate G3 passes. Re-check every rule, value and task against the G3 playtest notes before work starts. Prototype results will change it.

| | |
|---|---|
| GDD sections covered | §24.1–24.4 Meta Progression, §30 Save / Persistence, §33 (25:00 Resolve), §34.2 (Meta Progression State), §34.6 Save / Migration |
| Cross-references | §4.1 (Hub = permanent progression), §5.5 (campaign progress, blueprint sources), §17.4 / §24.2 (conversion recipe unlocks), §23 (perk families), §25.3 Ascension, §26.1 retention, §32 VS ("save meta cơ bản") |
| Phase | VS only |
| Status | Provisional draft v1 (2026-10-02) |
| Owner types | `UMetaProgressionSubsystem` (GameInstanceSubsystem) + `UMetaSaveGame` (master plan D-07, D-14) |

## 1. Overview

MET keeps everything that survives a run: horizontal unlocks, a small capped set of vertical upgrades, campaign and biome progress (rules owned by WLD), tutorial completion (ONB) and user settings. It turns a run result into meta reward and keeps part of that reward when the run is lost. The save file is versioned, migratable and survives corruption. Run save (suspend/resume mid-run) stays OPEN and is not built here.

## 2. Player Experience

- Every finished run, win or lose, carries something forward and gives a reason to try a different approach (§33 25:00).
- Unlocks widen options (a new squad type, a tower blueprint, a recipe), they do not inflate numbers (§24.1).
- The player can see that progression has an end: "12 / 18 unlocked" (§24.4).
- Progress is never lost silently. If the save is damaged, the game says so and restores the last good copy.

## 3. Core Loop

```text
Run resolves (win / lose / abandon)
→ meta reward shown on the resolve screen
→ return to hub, profile already saved
→ spend meta currency on an unlock or upgrade rank
→ new option available in the next run
→ start next run
```

## 4. Gameplay Rules

### 4.1 Meta progression principles

- **R-MET-01 (§24.1) [LOCKED]:** Meta progression is horizontal first. No meta item may be an uncapped percentage chain of the "+5% HP, +10% damage, +8% income" kind.
- **R-MET-02 (§24.2) [unlabeled]:** Supported unlock types are exactly the nine GDD types: Class, Weapon archetype, Squad type, Tower blueprint, Alternative tower mode, Starting loadout option, New perk family, New conversion recipe, New biome access. A tenth type needs a GDD change.
- **R-MET-03 (§24.2, §5.5) Assumption NEW-MET-02:** An unlock is acquired by one of two sources, set per unlock in data:
  - *Purchase*: bought in the hub with meta currency.
  - *Grant*: given by a campaign event (blueprint source discovered §5.5, Siege Site liberated, boss domain cleared).
- **R-MET-04 (§24.2):** Content that no unlock definition grants is base content and is available from the first run.
- **R-MET-05 (§24.2):** Unlocked content becomes available from the next run start. Nothing unlocks mid-run.
- **R-MET-06 (§24.3) [TUNABLE]:** Vertical upgrades are allowed in small amount. Each upgrade has an explicit max rank, and the sum of all bought ranks has a global cap. GDD gives no numbers. VS placeholder: at most 3 upgrade definitions, max rank 3 each, global cap = sum of max ranks. Values live in `DA_MetaUpgrade_*` and `UGameTuningSettings`.
- **R-MET-07 (§24.3) [TUNABLE]:** Vertical upgrades apply at run start as stat modifiers (`Stat.*` tags) through the shared stat modifier query (PRK). Review rule: a fully upgraded profile must still lose to the same mistakes; if the G3/VS playtest shows otherwise, lower the cap.
- **R-MET-08 (§24.4) [LOCKED]:** Meta progression is finite. The full catalog (unlocks + upgrade ranks) has a completion point and the UI shows completed / total. There is no endless progression bar.
- **R-MET-09 (§24.4) [LOCKED]:** After the catalog is complete, MET adds no new sink. Meta currency may keep counting but has nothing to buy. Replay value comes from mastery, build/class variety, Ascension, challenge modifiers and biome/boss mastery. Ascension is post-launch by default (Q-11) and out of VS.

### 4.2 Run result to meta reward

- **R-MET-10 (§33 25:00) [unlabeled]:** A won run grants the full meta reward: meta currency, unlock grants listed by the Siege Site, and campaign progress (site liberated, applied by WLD rules).
- **R-MET-11 (§33 25:00, §5.5) [unlabeled]:** A lost run keeps part of the meta reward "theo rule progression", then the player returns to the hub with the world intact. Keep fraction is [TUNABLE] with no GDD number. Placeholder 0.5 in `UGameTuningSettings::MetaLoseRewardFraction`.
- **R-MET-12 Assumption NEW-MET-03:** Meta reward is computed from run progress: amount per wave cleared + boss defeated bonus + first-clear bonus. Values come from the Siege Site reward table (WLD `USiegeSiteDefinition`). The lose fraction applies to the progress part only. First-clear bonus and unlock grants need a win.
- **R-MET-13 Assumption NEW-MET-04:** "Abandon run" from the pause menu resolves as a Lose (partial reward). A crash or forced quit mid-run grants nothing and leaves the meta save exactly as it was before the run.
- **R-MET-14 Assumption NEW-MET-01:** VS uses one meta currency. Its identity (the "rare material" of §33, or a separate currency) is open; data uses a neutral id and display name.

### 4.3 Persistence scope (§30)

- **R-MET-15 (§30) [LOCKED]:** Persistent meta data holds: unlocked classes, blueprints and every other unlock, campaign/biome progress, permanent upgrades, settings. MET also stores meta currency, tutorial completion and seen-hint flags (ONB).
- **R-MET-16 (§30 [OPEN], Q-10, A-11):** No run state is written to the meta save. Mid-run save/suspend is not built. If it is added later, it uses its own slot and version.
- **R-MET-17 (D-14):** Saved data references content only by stable IDs (`FPrimaryAssetId`, Gameplay Tags, authored `FName` IDs). Never object pointers, never asset paths.
- **R-MET-18 (§30, §34.6):** Settings persist in user config, separate from the meta save file. Resetting or losing the meta save never resets settings.
- **R-MET-19:** Campaign progress stores facts only (discovered, liberated, cleared). Derived states (site available, boss domain open) are recomputed on load by WLD rules, so a rules change never leaves stale state in the file.

### 4.4 Save version, migration, corruption (§34.6)

- **R-MET-20 (§34.6):** Every meta save carries a `SaveVersion` integer. The build knows its current version.
- **R-MET-21 (§34.6):** An older save migrates step by step (v1→v2→…→current) on load. The migrated profile is written only after every step succeeds. The pre-migration file stays on disk as a backup until the next successful save cycle.
- **R-MET-22 (§34.6):** A save with a newer version than the build is not loaded and never overwritten. The player is told, and the session runs on a default profile with saving disabled.
- **R-MET-23 (§34.6):** Corruption fallback. Two save slots are written alternately, each with a checksum. Load picks the newest valid slot. If the newest slot is invalid, the older valid slot loads and the player is told that the last progress may be missing. If no slot is valid, a new profile starts, the damaged files are kept under a renamed name, and the player is told. The game never crashes on a bad save and never overwrites the last valid copy with an unverified write.
- **R-MET-24 (§34.6):** Unknown content IDs (content removed or renamed between versions) stay in the file and are ignored at runtime. Renamed IDs are remapped through redirects.
- **R-MET-25 (§34.6):** Save only at safe points: run resolve, hub purchase, campaign fact change (discovery, liberation), tutorial completion, explicit quit from a menu. Never mid-combat. Writing must not hitch the frame.
- **R-MET-26 (§34.6):** Settings persistence: settings survive restart. Missing or invalid values fall back to defaults, clamped to valid ranges.
- **R-MET-27:** "Reset progress" exists in the menu, needs a confirmation, keeps settings, and keeps the previous profile as a backup file.

## 5. Player Actions

| Action | Where | Rule |
|---|---|---|
| View unlock catalog and completion | Hub station, main menu | R-MET-08 |
| Purchase an unlock | Hub station | R-MET-03 |
| Purchase an upgrade rank | Hub station | R-MET-06 |
| View meta reward of the last run | Resolve screen, hub | R-MET-10, R-MET-11 |
| Abandon a run | Pause menu in a Siege Site | R-MET-13 |
| Change settings | Main menu, pause menu | R-MET-18, R-MET-26 |
| Reset progress | Main menu | R-MET-27 |

## 6. Success / Failure Conditions

- Success: every run end produces a saved profile that matches the shown reward; restarting the game restores it.
- Partial success: the newest slot is damaged, the older one loads; the player loses at most one save point of progress and is told.
- Failure handled: no valid slot; the game continues on a new profile and keeps the damaged files.
- Not a MET failure: losing a run. A lost run still yields partial reward (R-MET-11).

## 7. Scope

### In Scope (VS)
- `UMetaProgressionSubsystem`, `UMetaSaveGame` schema v1, envelope with checksum, two-slot write, migration framework (with one test migration), redirects, orphan IDs.
- Unlock definitions for all nine types (data type support), VS content of 4–6 unlocks covering at least 3 types, at least one Purchase and one Grant.
- Vertical upgrades: 2–3 definitions with caps.
- Run result to meta reward (win / lose / abandon).
- Settings persistence: audio volumes, mouse sensitivity, invert Y, key rebinds, video (engine user settings).
- Hub unlock/upgrade screen, resolve screen meta section, main menu profile notices, reset progress.

### Out of Scope
- Run save / mid-run suspend (Q-10, A-11).
- Cloud save, multiple profiles, profile import/export.
- Ascension (Q-11), achievements, leaderboards (§26.2 DEFERRED).
- Pre-run loadout selection UI (NEW-MET-05).
- Any meta currency sink other than unlocks and upgrade ranks.

## 8. Anti-Goals

- No "+X% forever" upgrade chains (§24.1).
- No endless progression bar or prestige loop (§24.4).
- No meta power that makes skill or difficulty pointless (§24.3).
- No premium or second currency in VS.
- No single save blob mixing run state, meta and settings (UE save rule: separate data categories).

## 9. Dependencies

| Needs | From | Anchor / task |
|---|---|---|
| Primary Asset Types, `IsDataValid` pattern, `UGameTuningSettings` | FND | T-FND-07 |
| Cheat manager, debug CVars | FND | T-FND-09 |
| Automation test harness | FND | T-FND-10 |
| Enhanced Input base | FND | T-FND-06 |
| Run resolve and its result struct | RUN | T-RUN-06 |
| Run start hook (apply upgrades) | RUN | T-RUN-01 |
| Stat modifier query | PRK | T-PRK-02 |
| Perk offer generation (filter by unlocked families) | PRK | T-PRK-04 |
| Build menu / placement (filter tower blueprints) | DEF | T-DEF-07 |
| Squad roster at run start (filter squad types) | SQD | T-SQD-01 |
| Siege Site reward table, campaign rules | WLD | T-WLD-02 |
| Conversion recipe filter | CNV | T-CNV-02 |
| Tutorial completion flag write | ONB | T-ONB-08 |
| Feedback rows | UXF | T-UXF-01 |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Same unlock granted twice (replayed site, re-discovered source) | Idempotent; no double reward, no error |
| Purchase with exactly enough currency, then a save fails | Purchase stays applied in memory; save retried at next safe point; warning shown |
| Disk full or write error | Game continues; warning; retry at next safe point; the last valid slot is untouched |
| Game closed during a write | The slot being written is invalid on next load; the other slot is used (R-MET-23) |
| Content removed in a patch | ID kept as orphan; unlock count and completion ignore it (R-MET-24) |
| Lose with 0 waves cleared | Reward = 0; the profile still saves any campaign facts gained during the session |
| Currency near int32 max | Clamped; never wraps |
| Global vertical cap reached | Remaining upgrade ranks show "cap reached"; purchase disabled |
| Save from a newer build | R-MET-22 |
| Reset progress while a save is in flight | Reset waits for the write to finish |

## 11. Acceptance Criteria

- **AC-MET-01:** Winning a run, quitting and restarting the game shows the same currency, unlocks and campaign progress as the resolve screen showed.
- **AC-MET-02:** Losing a run grants exactly `floor(progress reward × MetaLoseRewardFraction)`; first-clear bonus and unlock grants are not given.
- **AC-MET-03:** Abandon from the pause menu resolves as Lose. Killing the process mid-run leaves the save byte-identical to the pre-run file.
- **AC-MET-04:** A purchased unlock (e.g. Spearman squad type) is missing in runs before the purchase and present in the next run after it.
- **AC-MET-05:** Upgrade purchase is refused above an upgrade's max rank and above the global cap; the UI shows why.
- **AC-MET-06:** The unlock screen shows completed / total; completing all VS catalog items shows 100% and no further purchasable item.
- **AC-MET-07:** Corrupting the newest slot (cheat or edited bytes) loads the older slot and shows the "recovered" notice. Corrupting both starts a new profile, keeps both files renamed, and shows the notice. No crash in either case.
- **AC-MET-08:** A v1 fixture save loads in a build with version 2 through the test migration; the result matches the expected v2 fixture (Automation Spec).
- **AC-MET-09:** A save with version > current is not loaded, not overwritten, and the notice is shown.
- **AC-MET-10:** A save containing an unknown unlock ID loads; the ID is kept after the next save; it does not count toward completion.
- **AC-MET-11:** Settings changed in the menu (volume, sensitivity, invert Y, one key rebind) persist after restart; resetting progress keeps them.
- **AC-MET-12:** No frame above the frame budget during a save on the reference PC (Insights trace of resolve → save).
- **AC-MET-13:** No catalog item is an uncapped percentage stat (design review of all `DA_MetaUnlock_*` / `DA_MetaUpgrade_*` against R-MET-01).

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| NEW-MET-01 | Is §33's "rare material" the meta currency, or is there a separate meta currency? | One meta currency, neutral id, display name in data | REQUIRED |
| NEW-MET-02 | How are unlocks acquired: hub purchase, campaign grants, or both? | Both, per unlock in data | REQUIRED |
| NEW-MET-03 | What does the meta reward formula count (waves, boss, first clear, rare material)? | Per wave cleared + boss bonus + first-clear bonus | REQUIRED |
| NEW-MET-04 | Does "Abandon run" count as Lose with partial reward? | Yes | IMPROVEMENT |
| NEW-MET-05 | Is there a pre-run loadout selection, and with what limits? | No selection in VS; runs use all unlocked content within run limits | FUTURE |
| Q-10 / A-11 | Mid-run save at launch? | No | (master plan) |
| Q-11 | Ascension at launch? | Post-launch | (master plan) |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Own the persistent profile: unlocks, upgrade ranks, meta currency, campaign facts, tutorial/hint flags. Load, migrate, validate, save. Turn a run result into meta reward. Answer "is this content unlocked?". |
| Inputs | Run result from RUN resolve (outcome, Siege Site ID, waves cleared, boss defeated); purchase requests from hub UI; grant requests from WLD (discoveries, liberation) and ONB (tutorial flag); settings changes from the settings menu. |
| Outputs | Unlocked content ID set for consumers (DEF build menu, SQD roster, PRK pool, CNV recipes, WLD gates); upgrade modifiers at run start; reward breakdown for UI; user notices (recovered, new profile, newer version, save failed). |
| State | `FMetaProfile` inside `UMetaSaveGame` (one in memory); slot sequence counter; dirty flag; save-in-flight flag. Settings in `UUserGameSettings` (config). |
| Events | `OnProfileLoaded(LoadResult)`, `OnMetaChanged`, `OnUnlockAcquired(UnlockId, Source)`, `OnRunRewardApplied(Breakdown)`, `OnSaveFinished(bSuccess)` |
| Data model | `UMetaUnlockDefinition`: unlock type (one of 9), granted content IDs, source (Purchase/Grant), cost, display data. `UMetaUpgradeDefinition`: target `Stat.*` tag, modifier per rank, max rank, cost per rank. `UGameTuningSettings`: lose fraction, global vertical cap, save slot names. Siege Site reward table lives in WLD `USiegeSiteDefinition`. |
| Failure cases | Corrupt slot, both slots corrupt, newer version, migration step failure, write failure / disk full, quit during write, unknown IDs, abandon/crash mid-run (§34.4 "player leaving Siege Site": leaving through Abandon = Lose; leaving by crash = no change). |
| Performance | Profile is small (target < 100 KB). Serialization on the game thread is short; file write on a background thread. No Tick. Saves only at safe points. |

## 14. Feedback Contract (GDD §34.5)

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| Meta reward granted (`Feedback.Meta.RewardGranted`) | Currency count-up on resolve screen | Reward sting | Breakdown: waves, boss, first clear, lose fraction | Lose screen shows what was kept and why |
| Unlock acquired (`Feedback.Meta.UnlockAcquired`) | Card reveal | Unlock sting | Toast "New: Spearman squad" + where it appears | Toast names the next-run effect |
| Purchase refused (`Feedback.Meta.PurchaseRefused`) | Button shake | Soft deny | Reason text: cost / cap / already owned | Always a reason, never a silent fail |
| Saving (`Feedback.Meta.Saving`) | Small save icon | None | Corner icon while writing | Never a blocking dialog |
| Save failed (`Feedback.Meta.SaveFailed`) | Warning icon | Warning tone | "Progress could not be saved, will retry" | Shown once per failure streak |
| Recovered from backup (`Feedback.Meta.SaveRecovered`) | Notice panel | None | "Last progress may be missing" | Shown at boot before the hub |
| New profile after corruption (`Feedback.Meta.SaveReset`) | Notice panel | None | Says damaged files were kept | Shown at boot |
| Newer save version (`Feedback.Meta.SaveTooNew`) | Notice panel | None | "Saving disabled this session" | Shown at boot |
| Settings applied (`Feedback.Meta.SettingsApplied`) | None | UI click | Values update immediately | Invalid values snap to valid range |
