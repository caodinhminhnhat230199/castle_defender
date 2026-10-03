# Battlefield Synergy (Shared Combat States): Specification

| | |
|---|---|
| Feature | SYN (`04-battlefield-synergy`) |
| GDD sections covered | §9.7, §10.1–10.3, §19.5, §21.1 (Ballista uses Armor Broken), §28.2 (state icons/VFX), §28.3 (armor break audio) |
| Cross-references | §13.4 (target priority), §19.1 (Warlord armor break), §20.1 (Armored), §23.2 (perk examples, future consumers), §33 (example run), §34.4, §34.5, §38 |
| Phases | **P0** (poise → Staggered, T-SYN-01) · **P1** (Armor Broken, Marked, presentation, Army + Hero consumers) · **P2** (Tower consumers) |
| Assumptions used | A-03 (Armored in P1), A-06 (no prototype Marked source except debug/perk) |
| Deferred | Elemental / status states (§10.2) |
| Status | Draft v1 |

Related docs: [technical-plan.md](technical-plan.md), [tasks.md](tasks.md); combat contract in [00-foundation/technical-plan.md §7](../00-foundation/technical-plan.md#7-shared-combat-contract-d-05); hit dispatch in [01-hero-combat/technical-plan.md §4.2](../01-hero-combat/technical-plan.md#42-hit-resolution-deliverhit).

## 1. Overview

Battlefield Synergy is the system that makes Hero, Army and Tower create openings for each other instead of stacking damage in parallel (§10.1). It has three shared combat states, **Staggered**, **Armor Broken** and **Marked**, plus the poise model that leads to Staggered (§9.7).

SYN owns the state rules and the poise math inside `UCombatStateComponent` (D-05), the armor reduction applied while Armor Broken is active, the presentation contract for the states, and the consumer rules in each layer (Hero follow-up, Army target preference, Tower priority and bonus). Producers send states through the shared hit pipeline; consumers only ask "does this target have state X?".

## 2. Player Experience

- **Openings, not numbers** (§10.1): the player sees a Heavy crack an Armored enemy's armor and then sees Archers, Infantry or the Ballista exploit it (§33 06:00).
- **One visual language** (§28.2): a state looks and sounds the same whether the Hero, a squad or a tower caused it.
- **Readable cause and effect** (§28.1): the player can tell in 1–2 seconds which enemy is Staggered, Armor Broken or Marked.

## 3. Core Loop

```text
Produce a state (Heavy, Parry, tower impact, perk)
→ State shows on the target (icon, VFX, sound)
→ Other layers react (follow-up damage, target preference, tower priority)
→ State expires or target dies
→ Player creates the next opening
```

## 4. Gameplay Rules

[TUNABLE] values live where the table in §4.5 says. Starting values there are placeholders, not GDD numbers.

### 4.1 P0 scope: poise and Staggered

- R-SYN-01 (§9.7) [LOCKED]: Important enemies have Poise. A unit has poise when its `MaxPoise > 0`; `MaxPoise = 0` means the unit is never poise-staggered. Which archetypes have poise is ENM data.
- R-SYN-02 (§9.7) [LOCKED]: Poise damage comes only from `FCombatHit.PoiseDamage` through `DeliverHit`. Sources: Heavy Attack (P0), Parry (P0), certain tower impact (P2), certain squad ability (VS, with squad abilities §11.3). Light may carry low poise damage (CMB data).
- R-SYN-03 (§9.7, §10.2) [LOCKED]: Poise break → Staggered. When poise reaches 0, the unit gets `State.Combat.Staggered` for its `StaggerDuration` [TUNABLE].
- R-SYN-04 (Assumption, NEW-SYN-06): While Staggered, further poise damage is ignored. Poise refills to max when Staggered ends.
- R-SYN-05 (§9.7) [TUNABLE]: Poise regenerates at `PoiseRegenRate` once `PoiseRegenDelay` has passed since the last poise damage.
- R-SYN-06 (§10.2): Staggered effect: the unit loses its actions for the duration. Its current attack is interrupted and it starts no new attack. Enemies: ENM (T-ENM-04). Hero: CMB (T-CMB-08). Soldiers: SQD when soldiers get poise (none in prototype, NEW-SYN-08).
- R-SYN-07 (§10.2): Staggered sources: poise break from any layer, parry (through parry poise damage), shield bash (VS squad ability), heavy tower impact (P2, through poise), hero block break (CMB, applied directly). Any source may also apply Staggered directly with a duration.

### 4.2 Shared state model (all phases)

- R-SYN-08 (§10.2, D-05): Each state is a Gameplay Tag under `State.Combat` with an expiry in game time, stored on the target's `UCombatStateComponent`. Re-applying an active state refreshes it to the longer of remaining and new duration. States never stack. The latest instigator is recorded.
- R-SYN-09 (§10.1) [LOCKED]: One pipeline for every layer. Hero, Army, Tower and Enemy sources apply states the same way (`FCombatHit.AppliedStates` or a direct apply call). Consumers only query the target. No layer keeps its own copy of a state.
- R-SYN-10 (§34.4): All states are removed and poise reset when the unit dies or is reset (e.g., boss reset by BOS). A unit without `UCombatStateComponent` (structures) ignores states.

### 4.3 P1 scope

**Armor Broken**
- R-SYN-11 (§10.2, §19.1) [LOCKED source]: Warlord Heavy hits apply Armor Broken. This is baseline Warlord behavior (NEW-SYN-02 records the §33 perk wording).
- R-SYN-12 (Assumption, NEW-SYN-01): Armor Broken only applies to targets with `BaseArmor > 0`. Units without armor never show it.
- R-SYN-13 (§10.2): Armor Broken effect: the target's armor drops to `BaseArmor × ArmorBrokenArmorMultiplier` [TUNABLE] for every incoming hit from every layer, for `ArmorBrokenDuration` [TUNABLE]. This makes Hero, Army and Tower damage better at once and creates the focus-fire window.
- R-SYN-14 (Assumption, NEW-SYN-01): Armor model: `BaseArmor` is the fraction of hit damage removed (0 to 0.9), set per unit definition. Damage taken = `Damage × (1 − EffectiveArmor)`.

**Marked**
- R-SYN-15 (§10.2, A-06): Marked is a timed state with `MarkedDuration` [TUNABLE]. In the prototype only the debug cheat and the perk effect API apply it. The Attack/Focus Target command does not apply Marked (A-06, Q-14). Ranger, scout and tower sources are deferred with their features.

**Army consumers**
- R-SYN-16 (§10.2, §13.4): Archer: Marked targets share the top priority tier with the player Focus Target ("player Marked/Focus Target", §13.4).
- R-SYN-17 (§10.2, Assumption NEW-SYN-07): Infantry and Archer: among candidates in the same priority tier and inside leash, Staggered and Armor Broken targets score higher ("squad AI prioritizes the target if the rule allows"). A state preference never beats a higher tier (Guard Zone threat, Focus Target) and never pulls a squad past its leash.
- R-SYN-18 (§10.2): Spearman "more effective vs Armor Broken" is a per-attack damage multiplier, added when Spearman arrives at VS.

**Hero consumers**
- R-SYN-19 (§10.2): Hero attacks have per-attack state damage multipliers. Hits on a Staggered target get the follow-up multiplier (§10.2 "Hero can execute/follow up"). Some hero attacks (default: Heavy) get a Marked bonus (§10.2 "some Hero attacks get a bonus"). No new input or execution move in the prototype (NEW-SYN-03).

**Presentation**
- R-SYN-20 (§28.2): Each state has one icon, one body VFX and one colour, identical for every unit type and every source. State colours are not reused for anything else.
- R-SYN-21 (§28.3): Armor break has its own audio event. Staggered and Marked application each have a one-shot cue.
- R-SYN-22 (§28.1): A unit shows all its active states (max 3) as icons ordered by display priority Staggered > Armor Broken > Marked.

**Design rules**
- R-SYN-23 (§10.3) [LOCKED]: Every new class, tower or squad answers: which state does it create, which state does it use, which other layer does it support. The answer goes into the synergy matrix (§4.6). "Same thing +20% DPS" content gets low priority.
- R-SYN-24 (§19.5) [LOCKED]: A new class changes at least two of: combat rhythm, command behavior, tower/defense interaction. Checked at class intake (first case: Ranger, after the prototype).

### 4.4 P2 scope: Tower consumers

- R-SYN-25 (§21.1, §10.2): Ballista prefers Armor Broken and Marked targets in range over other targets, and deals bonus damage to Armor Broken targets on top of the armor reduction. The "accuracy" part of the Marked effect applies only if Ballista shots can miss (NEW-SYN-04).
- R-SYN-26 (§9.7, §10.2): Heavy tower impact (Bombard by default) deals poise damage, which can Stagger through the normal poise rules.
- R-SYN-27 (§10.2, NEW-SYN-05): Tower weapon data can list states applied on impact ("certain siege/tower impact" → Armor Broken). Default: no tower applies Armor Broken in P2.
- R-SYN-28 (§10.2) [DEFERRED]: Elemental / status states (Burning, Frozen, Shocked) are not built: no tags, data or code. Adding one later must only need a new tag, presentation row and consumer data.

### 4.5 Tunable starting values

| Value | Start (placeholder) | Lives in |
|---|---|---|
| `StaggerDuration` | 1.5 s | `FCombatStateConfig` on each unit definition (ENM, SQD, BOS) |
| `PoiseRegenDelay` / `PoiseRegenRate` | 2.0 s / 25 per s | `FCombatStateConfig` |
| `MaxPoise` examples | P0 melee 50, Armored 120, Swarm 0 | ENM archetype data |
| `BaseArmor` examples | Armored 0.5, others 0 | ENM archetype data (`UHealthComponent` init) |
| `ArmorBrokenDuration` | 6 s | `UGameTuningSettings.StateDefaultDurations` |
| `ArmorBrokenArmorMultiplier` | 0.25 | `UGameTuningSettings` |
| `MarkedDuration` | 8 s | `UGameTuningSettings.StateDefaultDurations` |
| Hero follow-up multiplier vs Staggered | Light 1.25, Heavy 1.5 | `UHeroClassDefinition` attack `StateDamageMultipliers` |
| Hero bonus vs Marked | Heavy 1.2, Light 1.0 | `UHeroClassDefinition` attack `StateDamageMultipliers` |
| Army state preference weights | Staggered 1.0, Armor Broken 1.0 | `USquadDefinition` target rules |
| Ballista vs Armor Broken | ×1.5 damage; priority Armor Broken, Marked | `UStructureDefinition` weapon data |
| Bombard impact poise damage | 30 | `UStructureDefinition` weapon data |

### 4.6 Synergy matrix (§10.3)

| State | Produced by | Consumed by | Phase |
|---|---|---|---|
| Staggered | Hero Heavy/Parry poise (P0); hero block break (on hero, P0); Bombard impact poise (P2); shield bash (VS) | Enemy loses actions (ENM, P0); Hero follow-up multiplier (P1); Infantry/Archer preference (P1) | P0–P2 |
| Armor Broken | Warlord Heavy (P1); tower impact data hook, off by default (P2) | Every layer via armor reduction (P1); Infantry/Archer preference (P1); Ballista priority + bonus (P2); Spearman bonus (VS); perk "Heavy restores stamina on Armor Break" (P3, PRK) | P1–P3 |
| Marked | Debug cheat, perk API (A-06); Ranger/scout/tower later | Archer top tier (P1); Hero Heavy bonus (P1); Ballista priority (P2) | P1–P2 |

Future consumers (§23.2 hybrid perk examples, built by PRK, not SYN): "Heavy Attack restores stamina on Armor Break" listens to `OnStateAdded(ArmorBroken)` with the Hero as instigator; "Successful Parry → nearby Infantry attack speed" listens to CMB `OnParrySucceeded`.

## 5. Player Actions

SYN adds no input. The player creates states through existing actions: Heavy Attack and Parry (CMB), positioning squads (SQD) and towers (DEF). Debug only: cheats apply or clear states.

## 6. Success / Failure Conditions

- Success: an opening created by one layer is visibly used by another (G1 for Hero → Army, G2 for Hero → Tower, G3 "at least one meaningful Hero ↔ Army ↔ Tower synergy").
- Failure: states exist but no player notices them, or one layer's consumer ignores them (playtest finding), or states are so strong that every fight is the same loop (tuning).

## 7. Scope

### In Scope
- P0: poise damage, regen, break → Staggered, timed state storage, debug draw, Automation Spec, Functional Test with the ENM enemy.
- P1: Armor Broken (source, eligibility, armor math), Marked (state, debug/perk API), state presentation contract with UXF, Army and Hero consumers, QA, G1 synergy check.
- P2: Ballista and Bombard consumers, tower state data hook, QA, G2 regression.

### Out of Scope
- Enemy Staggered behavior animation/AI (ENM), hero block break and parry (CMB), icon widgets and VFX assets (UXF T-UXF-05, production-plan), squad target selection core (SQD T-SQD-07), tower targeting core (DEF T-DEF-09), perk effects (PRK), Ranger Mark, squad abilities (VS), Spearman (VS), elemental states ([DEFERRED]).

## 8. Anti-Goals

- No state explosion: three states in the prototype; no new state without a GDD source (§3, §10.2).
- No status-effect RPG layer: no Burning/Frozen/Shocked, no stacking debuffs, no DoT (§10.2 [DEFERRED]).
- No layer-private states: a state means the same thing to every layer (§10.1).
- No "+20% DPS" synergy (§10.3).
- No GAS effects for states (D-04).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| `UCombatStateComponent`, `UHealthComponent`, `FCombatHit` skeletons; tag roots | FND T-FND-04, T-FND-05 | State storage, armor hook |
| Tuning settings, cheats, CVars, test harness | FND T-FND-07, T-FND-09, T-FND-10 | Global durations, debug, tests |
| `DeliverHit` pipeline | CMB T-CMB-04 | Applies poise and `AppliedStates` for every hit |
| Heavy `AppliedStates` hook, parry poise | CMB T-CMB-06, T-CMB-09 | P0/P1 sources |
| Enemy with poise + Staggered behavior; Armored archetype | ENM T-ENM-01, T-ENM-04, T-ENM-06 | P0 test target, P1 armor |
| Squad target selection with tiers and leash; Infantry/Archer content | SQD T-SQD-07, T-SQD-10 | P1 Army consumers |
| Feedback subsystem, combat state icons/VFX, audio set | UXF T-UXF-01, T-UXF-05, T-UXF-06 | Presentation |
| Tower weapon targeting; Ballista; Bombard | DEF T-DEF-09, T-DEF-10, T-DEF-11 | P2 Tower consumers |

Provided to others: `FCombatStateConfig` (embedded in ENM/SQD/BOS definitions), `HasState` / `ApplyState` / `ClearAllStates`, `OnStateAdded` / `OnStateRemoved` (UXF, PRK, ENM), armor math in `UHealthComponent`, `UCombatLibrary::GetStateDamageMultiplier` (CMB, DEF, VS Spearman).

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Heavy breaks poise and applies Armor Broken in the same hit | Both states are added; one feedback per state |
| Poise damage on a Staggered unit | Ignored (R-SYN-04) |
| Same state applied by Hero and Ballista within 1 s | One state, duration refreshed to the longer remaining, instigator = latest |
| Instigator (tower, hero) destroyed while its state is active | State runs to expiry; instigator reads as null safely |
| Unit dies while states are active | All states removed, icons gone, `OnStateRemoved` fires per state |
| Armor Broken applied to a unit with `BaseArmor = 0` | Ignored, no icon, no sound |
| Unit with no `UCombatStateComponent` (structure) gets `AppliedStates` | Ignored silently |
| Bombard AoE staggers 20 Swarm-like units at once | All 20 get the state; presentation cost stays inside UXF budget (risk in technical plan) |
| Time dilation (Tactical Focus, P3) | Durations and regen use game time and slow down with it |
| Boss reset (§34.4) | BOS calls `ClearAllStates` and poise resets to max |
| Squad wipe (§34.4) | Soldiers' states cleared on death; nothing dangling on the squad |
| Marked target leaves Archer range | Archer re-targets by its normal rules; Marked stays on the target |

## 11. Acceptance Criteria

### P0
- AC-SYN-01: A unit with `MaxPoise 50` reaches Staggered after 50 poise damage (e.g., Heavy 40 + two Lights 5); `HasState(Staggered)` is true for `StaggerDuration`, then false; poise is back at max.
- AC-SYN-02: Poise damage during Staggered changes nothing; poise regenerates only after `PoiseRegenDelay` and at `PoiseRegenRate`.
- AC-SYN-03: A unit with `MaxPoise 0` never becomes Staggered from poise damage but can receive Staggered from a direct apply.
- AC-SYN-04: Re-applying an active state refreshes it to the longer duration and does not fire `OnStateAdded` again; death clears every state and fires `OnStateRemoved` per state.
- AC-SYN-05: The CombatState Automation Spec passes (poise math, expiry, refresh, clear).
- AC-SYN-06: In `L_Test_CombatStates` the P0 melee enemy hit by Heavy + Lights stops its attack and stays passive for the stagger duration (with ENM T-ENM-04), and the stagger feedback plays once.

### P1
- AC-SYN-07: Warlord Heavy on an Armored enemy (`BaseArmor 0.5`) applies Armor Broken; the next 20-damage hit from any layer deals 17.5 instead of 10 (armor 0.5 → 0.125); after `ArmorBrokenDuration` it deals 10 again.
- AC-SYN-08: Warlord Heavy on a unit with `BaseArmor 0` applies no Armor Broken, no icon, no sound.
- AC-SYN-09: `MarkTarget` cheat marks the unit under the crosshair for `MarkedDuration`; no gameplay action other than the cheat or perk API applies Marked.
- AC-SYN-10: An Archer squad with two enemies at equal range shoots the Marked one; with a Focus Target set and a Marked enemy, both are in the top tier and the closer one is chosen.
- AC-SYN-11: An Infantry squad on Guard with two equally close enemies in the same tier attacks the Staggered or Armor Broken one; it never leaves leash to reach it and never ignores a Guard Zone threat for it.
- AC-SYN-12: Hero Light on a Staggered enemy deals ×1.25, Heavy ×1.5; Heavy on a Marked enemy deals ×1.2 (data values).
- AC-SYN-13: Each state shows the same icon and colour on an enemy whether the Hero, a cheat or (P2) a tower caused it; armor break plays its own sound; icons order Staggered > Armor Broken > Marked.
- AC-SYN-14: G1 synergy scenario recorded: a playtester can name which layer created and which layer used an Armor Broken opening.

### P2
- AC-SYN-15: A Ballista with two Armored targets in range, one Armor Broken, shoots the Armor Broken one first and deals the bonus damage; a Marked target is preferred over an unmarked one at equal state.
- AC-SYN-16: A Bombard impact deals its poise damage to every unit hit; units whose poise reaches 0 become Staggered.
- AC-SYN-17: With tower `AppliedStates` empty (default), no tower applies Armor Broken; adding `State.Combat.ArmorBroken` to a tower's data makes its hits apply it with no code change.

## 12. Open Questions / Assumptions

| ID | Question | Default until answered | Class |
|---|---|---|---|
| A-06 | Focus Target does not apply Marked | No prototype Marked source except debug/perk | Assumption (Q-14) |
| NEW-SYN-01 | Armor model, Armor Broken strength, eligibility (GDD gives none) | Armor = damage fraction removed; Armor Broken ×0.25; only units with armor > 0 | REQUIRED, tune at G1 |
| NEW-SYN-02 | §10.2 and §19.1 make Armor Broken a Warlord Heavy trait; §33 05:00 shows it as a perk choice. Baseline or perk? | Baseline; a perk may extend duration or add effects | REQUIRED, confirm at G1 |
| NEW-SYN-03 | Hero "execution/follow-up" (§10.2): damage bonus only, or a dedicated execution move? | Damage multiplier only; no new input | IMPROVEMENT, revisit at VS |
| NEW-SYN-04 | Ballista "accuracy" vs Marked: do Ballista shots miss at all? | Priority only, no accuracy model | REQUIRED for P2 (DEF answers) |
| NEW-SYN-05 | Which "siege/tower impact" applies Armor Broken (§10.2)? | None in P2; data hook ready | FUTURE |
| NEW-SYN-06 | Poise during and after Staggered; stunlock protection | Ignore poise damage while Staggered; refill at end; no extra immunity | REQUIRED, tune at G0/G1 |
| NEW-SYN-07 | What "squad AI prioritizes the target if the rule allows" means | Tie-break inside the current priority tier and leash | REQUIRED for P1 |
| NEW-SYN-08 | Do enemy hits poise-stagger soldiers or the Hero? | No: soldiers and Hero have `MaxPoise 0`; Hero is Staggered only by block break | REQUIRED for P1 |

## 13. System Contract (GDD §38)

| Item | Content |
|---|---|
| Responsibility | Poise and timed combat states for every unit; armor reduction under Armor Broken; state presentation events; state consumer rules for Hero, Army and Tower |
| Inputs | `FCombatHit` (PoiseDamage, AppliedStates, StateDuration, Instigator) via `DeliverHit`; direct `ApplyState` calls (block break, cheats, perks); `FCombatStateConfig` and `BaseArmor` from unit definitions; global durations/multipliers from `UGameTuningSettings` |
| Outputs | `HasState` answers; damage reduced by armor; `OnStateAdded` / `OnStateRemoved`; feedback events `Feedback.State.*`; state damage multiplier for attackers; target preference terms for squads and towers |
| State | Per unit: current poise (lazy), last poise damage time, active states (tag, expiry, instigator), one timer handle. No global state |
| Events | `OnStateAdded(Tag, Instigator)` (new state only), `OnStateRemoved(Tag)`; feedback `Feedback.State.<State>.Applied` / `.Removed` |
| Data model | `FCombatStateConfig`: MaxPoise, PoiseRegenDelay, PoiseRegenRate, StaggerDuration. Unit definition: `BaseArmor`. `UGameTuningSettings`: `StateDefaultDurations` (tag → seconds), `ArmorBrokenArmorMultiplier`. `DT_CombatStatePresentation` row: StateTag, AppliedFeedback, RemovedFeedback, Icon, Colour, DisplayPriority, LoopVFX. Attacker data: `StateDamageMultipliers` (tag → multiplier) on hero attacks, tower weapons, later Spearman. Squad target rules: state preference weights; Archer tier list includes Marked. Tower weapon: PoiseDamage, AppliedStates, preferred states |
| Failure cases (§34.4) | Hero death: hero states cleared. Squad wipe: soldier states cleared. Tower destroyed: states it applied run to expiry, null instigator safe. Boss reset: `ClearAllStates`. Blocked path opened, Core critical, leaving Siege Site: not affected. Missing presentation row: state still works, UXF logs a warning |
| Performance | Component on every unit, no Tick; one timer per unit only while a state is active; poise computed lazily on read; `HasState` is a small-array scan; consumers query cached candidate sets (SQD/DEF), never all units. Presentation bursts (AoE stagger) are the main risk |

## 14. Feedback Contract (GDD §34.5)

Rows in `DT_Feedback` and `DT_CombatStatePresentation`; tag names proposed, final names owned by UXF T-UXF-01.

| State / event | Visual | Audio | UI | Readability rule | Tag |
|---|---|---|---|---|---|
| Staggered applied | Stagger body VFX (fixed state colour), stagger pose (ENM) | Stagger one-shot | State icon on world marker | Visible at gameplay camera distance; clearly "can't act" | `Feedback.State.Staggered.Applied` |
| Staggered removed | VFX ends, unit recovers | — | Icon removed | No lingering icon | `Feedback.State.Staggered.Removed` |
| Armor Broken applied | Armor plates flash/drop (production-plan), body VFX in state colour | Dedicated armor-break sound (§28.3) | State icon | Must read from far away; never confused with a normal armored hit | `Feedback.State.ArmorBroken.Applied` |
| Armor Broken removed | VFX ends | — | Icon removed | — | `Feedback.State.ArmorBroken.Removed` |
| Marked applied | Mark VFX above the unit, state colour | Mark one-shot | State icon | Visible through crowds; one colour only | `Feedback.State.Marked.Applied` |
| Marked removed | VFX ends | — | Icon removed | — | `Feedback.State.Marked.Removed` |
| Several states active | Icons side by side | — | Max 3 icons, order Staggered > Armor Broken > Marked | Never hides HP or lock-on marker | (presentation row priority) |
| Poise level | — | — | Debug only (`game.debug.CombatStates`) | Not shown to the player in the prototype | — |
