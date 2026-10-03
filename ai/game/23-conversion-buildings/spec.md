# Conversion Buildings (CNV): Specification

> **Provisional — re-validate after G3.** This feature starts only after Gate G3 passes and after ECO's ledger exists. Re-check every rule, value and task against the G3 playtest notes before work starts.

| | |
|---|---|
| GDD sections covered | §17.1–17.4 Conversion Buildings, §14.6 Utility Building, §16.1 Monster Material |
| Cross-references | §6.3 (Collect / Reconfigure), §10.2 (Elemental DEFERRED), §10.3 synergy rule, §21.3 tower rule (for tower-channel effects), §23.2 (Ballista piercing perk), §24.2 ("New conversion recipe" unlock), §33 10:00 Conversion Decision, §34.1 (Conversion Recipe Data), §38 |
| Phase | VS only |
| Status | Provisional draft v1 (2026-10-02) |
| Data type | `UConversionRecipeDefinition` (master plan D-06) |

## 1. Overview

A Conversion Building is a Utility Building where the Hero spends Monster Material (MM) on one short-lived boost for one layer: Hero, Army or Tower. MM is scarce, so the player can afford one or two options, not all. Each building offers only a few clear options; there is no recipe book and no chain. VS ships one conversion building flow end to end.

## 2. Player Experience

- The §33 10:00 moment: "I have MM. Buff myself, reinforce Infantry, or give the Ballistas piercing ammo? I can't do all three."
- The choice reads in seconds: three cards, one per layer, cost and effect in one line each.
- The building is a real object on the battlefield: placing it is a decision, and losing it to a raid hurts.

## 3. Core Loop

```text
Collect MM from kills
→ walk the Hero to the Conversion Building (Prep, Intermission, or mid-wave at a risk)
→ Interact → pick one option (channel: Hero / Army / Tower)
→ MM spent, effect applied to its target for its duration
→ see the effect in combat (feedback on the boosted units)
→ effect expires → next decision
```

## 4. Gameplay Rules

### 4.1 Core conversion rules

- **R-CNV-01 (§17.1) [LOCKED]:** MM is valuable because the player must decide which layer to convert it into.
- **R-CNV-02 (§17.1) [LOCKED]:** No long crafting chain. A conversion is one step: MM → effect. No intermediate items, no recipe inputs other than resources.
- **R-CNV-03 (§16.1) [LOCKED, current direction]:** MM is mostly not spent directly; it goes through Conversion Buildings. Spending uses the ECO ledger.
- **R-CNV-04 (§17.2) [unlabeled]:** Three channels. Each recipe belongs to exactly one:
  - Hero: temporary potion, weapon coating, skill charge, temporary run buff.
  - Army: squad weapon enchant, armor reinforce, temporary formation buff, replenish/reinforce.
  - Tower: special ammo, overcharge, elemental shell, temporary range/fire-rate mode.
- **R-CNV-05 (§10.2) [DEFERRED elemental]:** No recipe applies Burning / Frozen / Shocked or any elemental state in VS. The §17.3 "Flaming Weapon" and §17.2 "elemental shell" examples are built without elemental states (visual flavor only) or left out.
- **R-CNV-06 (§17.3) [unlabeled example]:** The decision must create a Hero / Army / Tower trade-off. Tuning rule: with the MM a player typically holds at the conversion moment (GDD example: 20), the player can afford one or two of the offered options, never all of them. Costs are [TUNABLE] in recipe data; example range 8–12 MM per option for a 20 MM budget.
- **R-CNV-07 (§17.4) [LOCKED]:** Each Conversion Building starts with few clear options. VS: at most 3 options per building. No recipe book in launch scope.
- **R-CNV-08 (§10.3) [LOCKED]:** Each recipe must answer at least one: which state does it create, which state does it exploit, which other layer does it help. A "+20% DPS, same thing" recipe is low priority and needs a review note.
- **R-CNV-09 (§34.1):** Recipes are data. A new recipe that uses existing effect types needs no core code change.

### 4.2 Target, duration, stacking

- **R-CNV-10 (§3, §17.2):** The recipe data sets the target: the Hero, all squads of a squad type (e.g. `Unit.Squad.Infantry`), or all structures of a structure type (e.g. Ballista). The player never picks individual units.
- **R-CNV-11 (§17.2 "temporary") [TUNABLE]:** Effects are temporary. Duration per recipe in waves (until the end of wave N after purchase) or seconds; 0 = rest of run. GDD gives no values.
- **R-CNV-12:** Effects follow their target type, not specific actors: a squad-type effect also covers reinforced soldiers; a structure-type effect also covers structures of that type built later while the effect is active; a Hero effect survives Hero death and respawn.
- **R-CNV-13 Assumption NEW-CNV-02:** One active instance per recipe. Buying the same recipe while it is active is refused (no stacking, no refresh). Different recipes can be active at once.
- **R-CNV-14 Assumption NEW-CNV-01:** Hero-channel effects apply at once as timed buffs. VS has no consumable inventory, no quick-slot and no "drink" input (§3: no big inventory; §9.1 verb list).
- **R-CNV-15 (§24.2):** A recipe can require a meta unlock ("New conversion recipe"). Locked recipes are not shown in the run.

### 4.3 Building rules (§14.6)

- **R-CNV-16 (§14.6) [LOCKED]:** Conversion Buildings are Utility Buildings (`Structure.Role.Utility`): physical footprint, built through the normal build system, have HP.
- **R-CNV-17 (§14.6) [LOCKED] + Assumption NEW-CNV-04:** Utility Buildings should not sit in the middle of a default lane. VS rule: placement is refused when the footprint overlaps a lane corridor cell (validation message shown).
- **R-CNV-18 (§14.6) [LOCKED]:** If enemies reach a Utility Building, they can destroy it (Local Aggro "defensive structure in interaction rule" §14.5).
- **R-CNV-19 (§16.1):** Building and repairing a Conversion Building costs Gold (ECO).
- **R-CNV-20:** A destroyed Conversion Building offers no conversions until rebuilt. Active effects stay. Spent MM is never refunded.
- **R-CNV-21 Assumption NEW-CNV-03:** Converting needs the Hero alive, within interact range of a living Conversion Building (Interact verb). Not possible from Commander Spirit Mode or Tactical Focus.
- **R-CNV-22:** Conversion works in Prep, Intermission and during waves. The menu does not pause time.

### 4.4 VS content

- **R-CNV-23 (§17.3, §32 VS):** VS ships one Conversion Building with three options, one per channel, taken from the §17.3 example:
  - Hero: Combat Potion (timed Hero buff, e.g. stamina regen / damage taken reduction).
  - Army: Infantry Weapon Enchant (Infantry damage + poise damage, flame visuals, no Burning state).
  - Tower: Ballista Piercing Ammo (Ballista bolts pierce N targets).
  - Exact effect stats are [TUNABLE] and chosen at re-validation. Splitting into Forge / Alchemy / Workshop (one channel each, as in §17.3) is launch content.

## 5. Player Actions

| Action | Where | Rule |
|---|---|---|
| Build / repair a Conversion Building | Build mode, Utility-allowed cells | R-CNV-16, R-CNV-17, R-CNV-19 |
| Open the conversion menu | Interact at the building | R-CNV-21 |
| Choose one option | Conversion menu | R-CNV-06, R-CNV-07 |
| See active conversions and time left | HUD active-effect strip | R-CNV-11 |

## 6. Success / Failure Conditions

- Success for the player: the boost lands on the right layer at the right moment.
- Failure states: not enough MM (refused with reason), recipe already active (refused), building destroyed (no menu), Hero dead (no interaction).
- The feature succeeds as design if playtesters make different conversion choices across runs and can name why.

## 7. Scope

### In Scope (VS)
- `UConversionRecipeDefinition`, `UConversionBuildingDefinition`.
- `AConversionBuilding` (Utility) with placement rule, HP, destruction/rebuild.
- Conversion validation, MM spend, effect apply/expire for all three channels.
- Conversion menu, active-effect HUD strip, `Feedback.Conversion.*`.
- VS content: one building, three recipes.

### Out of Scope
- More buildings / recipes beyond VS content; Forge / Alchemy / Workshop split (launch content).
- Elemental states (§10.2 DEFERRED).
- Consumable inventory, item slots (NEW-CNV-01).
- Recipe upgrades, recipe chains, recipe books (§17.4).
- Remote conversion from Tactical Focus or Commander Spirit (NEW-CNV-03).

## 8. Anti-Goals

- Not a crafting system (§3 no 8–10 tier crafting; §17.1).
- No recipe book (§17.4).
- No item inventory (§3).
- No "buy everything" economy: if playtests show players affording all options routinely, costs go up (R-CNV-06).
- Not a stat shop: options must interact with layers and states (R-CNV-08).

## 9. Dependencies

| Needs | From | Anchor / task |
|---|---|---|
| Primary Asset Types, validation | FND | T-FND-07 |
| Structure base, placement, lane corridor data | DEF | T-DEF-02, T-DEF-04, T-DEF-07 |
| Ballista weapon (piercing hook) | DEF | T-DEF-09, T-DEF-10 |
| Interact verb | CMB | T-CMB-12 |
| Perk effects + stat modifier query | PRK | T-PRK-01, T-PRK-02 |
| Squad engagement / damage use of modifiers | SQD | T-SQD-07 |
| Wave lifecycle (duration in waves) | DIR | T-DIR-02 |
| MM ledger, Gold costs | ECO | T-ECO-01, T-ECO-02 |
| Recipe unlock query | MET | T-MET-05 |
| Feedback, HUD shell | UXF | T-UXF-01, T-UXF-02 |

## 10. Edge Cases

| Case | Behavior |
|---|---|
| Building destroyed while the menu is open | Menu closes; no purchase |
| Two spends in the same frame (double click) | Second refused (recipe active or funds gone) |
| Army recipe bought with zero squads of that type alive | Allowed only if the type exists in the run roster; applies when soldiers reinforce. If the type is not in the roster, option is hidden |
| Tower recipe bought with zero Ballistas built | Allowed; applies to Ballistas built while active; card warns "no Ballista built" |
| Hero dies with a Hero effect active | Effect stays; timer keeps running |
| Effect expires mid-swing / mid-shot | Removed at expiry; in-flight projectiles keep the property they were fired with |
| Run resolves with effects active | All effects removed at resolve; no carry-over |
| Second Conversion Building built | Allowed if build limits allow; same options; one active instance rule is per recipe, run-wide |

## 11. Acceptance Criteria

- **AC-CNV-01:** Interacting with the VS Conversion Building shows exactly its unlocked options (≤3), each with channel icon, effect line, target, duration and MM cost.
- **AC-CNV-02:** Buying an option spends its MM and applies its effect to the target type within the same frame; the HUD strip shows it with time left.
- **AC-CNV-03:** With 20 MM and VS default costs, the player can buy at most two of the three options.
- **AC-CNV-04:** Buying an active recipe again is refused with a reason; buying without enough MM is refused with the missing amount.
- **AC-CNV-05:** Ballista Piercing Ammo: a bolt hits N targets in a line while active, one target after expiry.
- **AC-CNV-06:** Infantry Weapon Enchant: Infantry hits show higher damage/poise damage in the combat debug view while active; no elemental state is ever applied.
- **AC-CNV-07:** Placing the building on a lane corridor cell is refused with a message; placing it in a valid Utility cell succeeds and costs Gold.
- **AC-CNV-08:** Enemies that reach the building can destroy it; afterwards Interact shows "destroyed"; active effects continue; rebuilding restores the menu.
- **AC-CNV-09:** Conversion is impossible while the Hero is dead or in Tactical Focus.
- **AC-CNV-10:** All effects end at run resolve.
- **AC-CNV-11:** Playtest: across 3+ runs, testers choose different options at least once and can explain the trade-off (§17.3).

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| NEW-CNV-01 | Do Hero potions need a consumable slot and a use input? | No; instant timed buff | REQUIRED |
| NEW-CNV-02 | Can a recipe stack or refresh while active? | No | IMPROVEMENT |
| NEW-CNV-03 | Must the Hero be at the building, or can conversion happen from build mode / Tactical Focus / Commander Spirit? | Hero at building only | REQUIRED |
| NEW-CNV-04 | Lane placement: hard refusal or warning? | Hard refusal on lane corridor cells | IMPROVEMENT |
| NEW-CNV-05 | One multi-channel building in VS, or three single-channel buildings as in §17.3? | One building, three options (art budget: one building) | REQUIRED |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Offer and execute MM → effect conversions at Utility Buildings; own active conversion effects and their expiry; enforce building placement and availability rules. |
| Inputs | Interact from the Hero; recipe choice from the menu; MM balance (ECO); unlock state (MET); wave lifecycle events (expiry); building health/destruction (DEF). |
| Outputs | MM spend; effect sources added/removed in the PRK effect pipeline (stat modifiers, behavior flags such as pierce count); active-effect list for HUD; refusal reasons. |
| State | `UConversionManagerComponent` on `ARunPlayerState`: active effects (recipe ID, target, expiry). Building: HP (`UHealthComponent`), alive/destroyed. |
| Events | `OnConversionApplied(RecipeId)`, `OnConversionExpired(RecipeId)`, `OnConversionRefused(RecipeId, Reason)`, building `OnDeath` |
| Data model | `UConversionRecipeDefinition`: channel (`ECombatLayer` Hero/Army/Tower), cost (`FResourceAmount` list, MM), target (Hero / squad type tag / structure definition), effects (instanced `UPerkEffect` list or stat modifiers), duration (waves or seconds), required unlock (optional), display (name, one-line effect, icon), synergy note (R-CNV-08). `UConversionBuildingDefinition : UStructureDefinition`: recipe list (≤3). |
| Failure cases | §34.4: tower destroyed (structure-type effects persist and cover rebuilt towers), squad wipe (squad-type effects persist for reinforcements), Hero death (Hero effects persist; no conversion while dead). CNV-specific: building destroyed, insufficient MM, recipe active, recipe locked. |
| Performance | Event-driven only: apply on purchase, remove on expiry/resolve. No Tick. Effect lookups go through the existing stat modifier query. |

## 14. Feedback Contract (GDD §34.5)

| State | Visual | Audio | UI | Readability rule |
|---|---|---|---|---|
| In range of building (`Feedback.Conversion.InRange`) | Building highlight | — | Interact prompt "Convert" + MM held | Prompt shows MM count |
| Conversion applied (`Feedback.Conversion.Applied`) | Burst on building + aura/glow on every affected unit (Hero, Infantry weapons, Ballista bolts) | Channel-specific sting (3 distinct) | HUD strip icon + time left | Boosted units readable at a glance; channel color fixed per layer |
| Conversion refused (`Feedback.Conversion.Refused`) | Card shake | Deny cue | Reason: "Need 4 MM" / "Already active" / "Locked" | Always a reason |
| Effect expiring (`Feedback.Conversion.Expiring`) | Strip icon blinks | Soft tick | Last wave / last 10 s | Warned before it ends |
| Effect expired (`Feedback.Conversion.Expired`) | Aura fades | Fade cue | Icon removed | — |
| Building under attack (`Feedback.Conversion.BuildingAttacked`) | HP bar visible, damage flash | Alarm (throttled) | Edge marker | Uses the structure HP rules from DEF/UXF |
| Building destroyed (`Feedback.Conversion.BuildingDestroyed`) | Collapse VFX | Collapse SFX | Toast "Conversion Building lost" | Interact prompt says "destroyed" |
