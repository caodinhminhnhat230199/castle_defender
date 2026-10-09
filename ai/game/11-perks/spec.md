# Perks (PRK): Specification

| | |
|---|---|
| GDD sections covered | §23 (all), §10.3, §15.4 (shared stat modifiers), §24.1–24.2 (meta is not perks), §32 P3, §33 (05:00 Perk), §34.1 (Perk Data), §34.2 (Player State), §34.4, §34.5, §36 Full Run DoD |
| Phases | P2: stat modifier query used by zone bonuses (`T-PRK-02`). P3: perk definitions, effects, 1-of-3 offers, initial pool |
| Owner folder | `ai/game/11-perks/` |
| Status | Draft v1. Every class/asset name is a proposal (no UE project exists yet) |

## 1. Overview

Two pieces, one feature:

1. **Stat modifier query (P2).** One place where gameplay numbers that other systems may change (zone bonuses now, perks later) are read: `final value = query(target, Stat tag, base value)`. Tactical Zones (ZON) are its first user.
2. **Roguelite perks (P3).** After some waves the player picks 1 of 3 perks. Each perk belongs to one or more categories (Hero, Army, Defense). Perks are either stat modifiers or event-triggered effects ("Successful Parry → nearby Infantry attack faster"). Hybrid perks are favored because they tie Hero, Army and Tower together (§23.3, §1.3).

Player value: a run-scoped build that changes *how* the player fights and commands, not only how big the numbers are (§36 "at least one distinct build via perks").

## 2. Player Experience

- Choosing a perk is a short, readable decision between waves: three cards, clear category badges, one sentence each.
- A good pick creates a plan: "I took Breaker's Breath, so I want Armored enemies in the lane where my Ballista is."
- Effects are visible when they fire, so the player connects the payoff to the choice (§34.5).
- Zone bonuses (P2) are light. They reward using a zone but never make it mandatory (§15.4).

## 3. Core Loop

```text
Wave cleared → Intermission "Choose" step (§6.3)
→ See 3 perk cards (categories, hybrid label, effect)
→ Pick 1 → perk active for the rest of the run
→ Next wave: perk fires on its trigger, feedback shows it
→ Adapt positioning / commands / builds around owned perks
```

## 4. Gameplay Rules

### 4.1 P2 scope: stat modifier query

- **R-PRK-01** (§15.4, §34.1; D-04) Gameplay numbers that zones or perks may change are read through one stat modifier query. The consumer keeps the base value in its own definition data (squad, structure, hero class). No GAS attributes (D-04).
- **R-PRK-02** (§15.4) A modifier has: a `Stat.<Domain>.<Name>` tag, an operation (Add or Multiply), a magnitude, a scope, and a source. P2 scope = one target actor (e.g., the squad holding a zone).
- **R-PRK-03** Assumption (NEW-PRK-1): final value = `(Base + ΣAdd) × (1 + ΣMultiply)`, clamped to ≥ 0. Multiply magnitudes are fractions (+0.15 = +15%). Result does not depend on the order modifiers were added.
- **R-PRK-04** (§15.4) [TUNABLE] Zone bonuses use this query: High Ground → ranged range/visibility, Chokepoint → Infantry block efficiency, Rally Point → reform/reinforce speed. Magnitudes live in `UTacticalZoneDefinition` (ZON), not here. "Must not be strong enough to force zone use every time" is enforced by ZON tuning.
- **R-PRK-05** Removing a source (zone released, perk removed, run ended) removes all its modifiers at once. Modifiers on a destroyed target are dropped.
- **R-PRK-06** Consumers read the stat at use time (attack, fire, regen step, reform). A consumer that caches a value refreshes it when it receives the "modifiers changed" event for that stat.

### 4.2 P3 scope: perks

- **R-PRK-07** (§23.1) [LOCKED] Every perk belongs to one or more categories: Hero, Army, Defense (`Perk.Category.*`). A perk with two or more categories is **hybrid**.
- **R-PRK-08** (§23.3) [LOCKED] Hybrid perks are prioritized: (a) in pool content, (b) in offer weights through `HybridWeightMultiplier` [TUNABLE] (starting value 1.5, in `UPerkPoolDefinition`).
- **R-PRK-09** (§23.3) [LOCKED] Generic stat perks (plain "+X% stat", no state or cross-layer link) may exist but must not be the majority of the pool. Pool validation fails when the generic share is above `MaxGenericPoolShare` [TUNABLE] (starting value 0.4, must stay < 0.5).
- **R-PRK-10** Assumption (NEW-PRK-2): one offer shows at most `MaxGenericPerOffer` generic perks (starting value 1), so a single offer is never "three stat bumps".
- **R-PRK-11** (§23.4) [LOCKED/TUNABLE] After selected waves the player chooses 1 of 3. Offer size (3) is data. Which waves = A-07 (after waves 1, 3, 4), stored in `URunDefinition` (RUN).
- **R-PRK-12** (§23.4) [TUNABLE] Weight logic: each perk has a `Weight` (default 1.0). Offers are weighted random picks without replacement from a seeded per-run random stream. Owned perks are not offered again: perks are unique per run (Assumption NEW-PRK-3).
- **R-PRK-13** (§23.4) Reroll is a later option: add it only if playtests show meaningless choices "too often". Design: `RerollsPerRun` in data, default 0 = feature off. Conditional task `T-PRK-14`.
- **R-PRK-14** (§10.3) [LOCKED] Every non-generic perk answers at least one: which state does it create, which state does it use, which other layer does it support. The answer is written in the perk's `SynergyNote` and checked at content review. "Same thing but +X%" perks are low priority and count as generic.
- **R-PRK-15** (§23.2; D-04) A perk effect is one of: (a) stat modifier, (b) event-triggered effect = trigger + response (e.g., Heavy Attack applies Armor Broken → restore stamina). Effects are instanced data inside the perk definition. No GAS.
- **R-PRK-16** (§24.1, §24.2) [LOCKED] Perks are run-scoped. They are granted during a run, cleared at run end and never written to the meta save. Meta progression must not become a chain of "+5% HP" perks. Meta can later unlock new perk families into the pool (§24.2, VS/MET, out of scope here).
- **R-PRK-17** (§18.1, §34.2) Owned perks belong to the player (`ARunPlayerState`), not the Hero pawn. They survive Hero death. Respawn creates a fresh Hero pawn (CSM), so every pawn-side effect re-binds/re-applies when the possessed pawn changes. Hero-triggered effects are inactive while the Hero is dead. Army and Defense effects keep working during Commander Spirit Mode.
- **R-PRK-18** (§6.3) The offer happens in the intermission "Choose" step. Assumption (NEW-PRK-4): that RUN step waits until the player picks; the world is not paused. The choice screen cannot be closed without a pick.
- **R-PRK-19** (§36 Full Run DoD) The initial pool must make at least one distinct build possible: a set of 2–3 perks that changes positioning, command or build decisions (Section 4.3 lists three candidate builds).
- **R-PRK-20** (§34.5) Owned perks are visible during play (perk tray). When an effect fires, feedback links it to its perk (Section 14).
- **R-PRK-21** (§23.2, §11.3) The GDD example "Shield Wall duration +X%" depends on squad abilities, which arrive at VS. It is provisional and not in the P3 pool.

### 4.3 Initial pool (P3 content proposal)

Values are **starting data**, not GDD numbers ([TUNABLE], stored in each `DA_Perk_*`). Rows marked "§23.2" are GDD examples; all others are content proposals that must pass §10.3 review in `T-PRK-06` (NEW-PRK-7).

| Asset | Name | Categories | Kind | Effect (starting values) | §10.3 link | Source |
|---|---|---|---|---|---|---|
| `DA_Perk_BreakersBreath` | Breaker's Breath | Hero | Event | Heavy Attack that newly applies Armor Broken restores 20 stamina | uses Armor Broken | §23.2 Hero example |
| `DA_Perk_RallyingParry` | Rallying Parry | Hero + Army | Event | Successful Parry: Infantry squads within 12 m get +25% attack speed for 6 s | Parry supports Army | §23.2 Hybrid example |
| `DA_Perk_PiercingBolts` | Piercing Bolts | Defense | Event | Every 4th shot of each Ballista pierces up to 2 extra targets | anti-armor focus fire | §23.2 Defense example |
| `DA_Perk_SunderingBlows` | Sundering Blows | Hero + Defense | Stat | Armor Broken applied by the Hero lasts +50% | longer window for Ballista/Army (§10.2) | §33 05:00, NEW-PRK-5 |
| `DA_Perk_ExposingParry` | Exposing Parry | Hero + Army + Defense | Event | Successful Parry applies Marked to the attacker for 6 s | creates Marked: Archer/Ballista priority (§10.2) | A-06 (perk is the prototype Marked source) |
| `DA_Perk_ConcussiveShells` | Concussive Shells | Defense + Hero | Stat | Bombard hits deal +15 poise damage | creates Staggered → Hero follow-up (§9.7) | proposal |
| `DA_Perk_Shieldbreakers` | Shieldbreakers | Army + Hero | Stat | Infantry hits deal +10 poise damage | creates Staggered → Hero follow-up | proposal |
| `DA_Perk_SecondWind` | Second Wind | Hero | Event | Successful Parry restores 15 stamina | rewards high-risk parry (§9.5) | proposal |
| `DA_Perk_DrilledRanks` | Drilled Ranks | Army | Generic | Squad damage +10% | none (generic) | §23.3 minority allowed |
| `DA_Perk_QuickCrank` | Quick Crank | Defense | Generic | Combat Tower fire interval −9% (≈ +10% fire rate) | none (generic) | §23.3 minority allowed |
| `DA_Perk_SteadyBreath` | Steady Breath | Hero | Generic | Hero stamina regen +15% | none (generic) | §23.3 minority allowed |
| (VS) `DA_Perk_ShieldWallDrill` | Shield Wall Drill | Army | Stat | `Stat.Army.ShieldWallDuration` +X% (SQD `T-SQD-19` reads it) | provisional, needs squad abilities | §23.2 Army example, R-PRK-21 |

P3 pool = 11 perks. Generic 3/11 (27%). Hybrid 5/11 (45%).

Candidate builds (R-PRK-19):

| Build | Perks | Play pattern it should create |
|---|---|---|
| Breaker | Breaker's Breath + Sundering Blows + Piercing Bolts | Hero hunts Armored with Heavy; player routes Armored into the Ballista lane |
| Parry Commander | Rallying Parry + Exposing Parry + Second Wind | Hero fights next to Infantry and seeks parries; Archers follow Marked targets |
| Siege Engineer | Concussive Shells + Shieldbreakers + Quick Crank | Player funnels Swarm/Armored into Bombard + Infantry; Hero cleans up Staggered enemies |

## 5. Player Actions

- Pick one of three perk cards (mouse click or keys 1/2/3).
- Read owned perks in the perk tray (hover shows description).
- Later option: reroll an offer (only if `T-PRK-14` is approved).
- Indirect: play around owned perks (parry near Infantry, Heavy on Armored, place Bombard at the choke).

## 6. Success / Failure

- Success (P2): a zone bonus applied through the query changes the squad's stat while the zone is held and stops when it is released.
- Success (P3): every offer is valid (3 distinct, unowned perks; generic cap respected); the picked perk works for the rest of the run; at G3 at least one build from Section 4.3 is assembled and changes play.
- Failure: offer contains duplicates or owned perks; an effect keeps running after its source is gone; a perk is lost on Hero death; the choice step blocks the run forever (e.g., empty pool with no fallback).
- Partial: pool smaller than the offer size → offer shows fewer cards; pool empty → step completes with no offer and a log warning.

## 7. Scope

### In Scope
- P2: stat modifier query, actor-scoped modifiers, change event, initial zone stat tags, debug listing.
- P3: `UPerkDefinition`, `UPerkPoolDefinition`, instanced `UPerkEffect` types (stat modifier + 4 event effects), `UPerkManagerComponent` on `ARunPlayerState`, weighted 1-of-3 offer generation, choice UI + perk tray, stat read points in Hero/Army/Defense code for perk stats, 11-perk initial pool, telemetry of offers/picks through UXF, cheats.

### Out of Scope
- Squad-ability perks (Shield Wall) until VS (R-PRK-21).
- Reroll unless G3 playtest asks for it (R-PRK-13).
- Perk stacking/duplicates, perk rarity tiers, perk removal/sell, perk upgrades (none in GDD).
- Meta unlock of perk families (§24.2, MET at VS).
- Conversion Building buffs (§17, CNV at VS) even though they may reuse the stat query later.
- Elemental/status perks (§10.2 Elemental [DEFERRED]).
- Mid-run save of perks (A-11); data is kept serializable (D-14).

## 8. Anti-Goals

- Not a deep build-craft system: no perk trees, no hundreds of perks, no inventory of items (§3).
- Not a GAS-style attribute/effect framework (D-04).
- Perks are not meta progression (§24.1): nothing persists after the run.
- No pool dominated by "+X% damage" cards (§23.3).
- No pick that requires micro of individual soldiers (§3, §13.1).

## 9. Dependencies

| Needs | From | Why |
|---|---|---|
| `ARunPlayerState`, run phase/steps, `URunDefinition` offer schedule (A-07), perk-offer step hook | RUN `T-RUN-01`, `T-RUN-05` | Owner of perks; when offers happen |
| `UHeroCombatComponent::OnParrySucceeded(Attacker)`; stamina (restore entry point); Heavy `FHeroAttackData.StateDuration` | CMB `T-CMB-03`, `T-CMB-06`, `T-CMB-09` | Event triggers, Hero stat reads |
| `UCombatStateComponent::OnStateAdded(Tag, Instigator)` (new states only), `ApplyState` (Marked), Armor Broken | SYN `T-SYN-01`, `T-SYN-02`, `T-SYN-03` | "Restore stamina on Armor Break", Marked on parry |
| Squad registry (`UCommandComponent`), squad type tag, soldier attack/damage code | SQD `T-SQD-01`, `T-SQD-07`, `T-SQD-10` | Nearby-Infantry buff, Army stat reads |
| Tower stat reads (`Stat.Tower.*`), `OnTowerShotPreparing(Tower, ShotIndex, FTowerShotParams&)`, projectile `PierceCount`, `Structure.Type.*` tags, structure placed/registered events | DEF `T-DEF-02`, `T-DEF-07`, `T-DEF-22` | Nth-shot pierce, Defense stat reads |
| Zone bonus magnitudes and hold/release events | ZON `T-ZON-03`, `T-ZON-04` | First user of the query (P2) |
| Feedback rows, HUD slot + Modal presentation derived from controller `OnPlayerModeChanged` (D-19), telemetry `LogEvent` + run summary | UXF `T-UXF-01`, `T-UXF-02`, `T-UXF-08`, `T-UXF-16`; FND controller `PushMode`/`PopMode` | Section 14 |
| Cancel Tactical Focus on modal open | TFM `T-TFM-01` | Choice UI opens cleanly |
| Tags, tuning settings, test harness | FND `T-FND-04`, `T-FND-07`, `T-FND-10` | Base |

## 10. Edge Cases

| Case | Expected behavior |
|---|---|
| Hero dies with timed Infantry buff active | Buff continues on the squad until its timer ends; hero-trigger effects unbind; perks stay owned |
| Hero respawns (new pawn) | Hero-trigger effects rebind to the new pawn; Hero stat perks apply immediately |
| Squad wiped while it holds a perk/zone modifier | Modifier ignored and pruned; no error |
| Ballista destroyed mid-count | Its shot counter is dropped; new Ballistas start at 0 (per-tower counter, NEW-PRK-6) |
| Offer requested while one is pending | Ignored with a warning; the pending offer stays |
| Pool has fewer candidates than offer size | Offer shows the remaining candidates; 0 candidates → step completes, warning logged |
| Offer opens while Tactical Focus is held | Focus is cancelled first (instant cancel §29.2), then the modal opens |
| Offer during Commander Spirit Mode | Works; perks belong to the player state |
| Parry triggers two perks (Rallying Parry + Exposing Parry + Second Wind) | All fire; each shows its own trigger feedback |
| Same timed buff re-triggered while active | Duration refreshes; magnitude does not stack |
| Zone and perk modify the same stat | Both apply through the formula (R-PRK-03); no per-stat cap in prototype |
| Boss reset (§34.4) | Perks unaffected; no rollback of perk state |
| Run ends (win/lose/quit) | All effects deactivate, all modifiers from perks removed |

## 11. Acceptance Criteria

### P2
- **AC-PRK-01** With base 10, an Add +2 and a Multiply +0.5 on the same target and stat, the query returns 18; after removing the source it returns 10.
- **AC-PRK-02** A modifier on squad A does not change squad B's value.
- **AC-PRK-03** A squad holding a High Ground zone reads a higher ranged range; on leaving the zone it returns to base within one read (verified with ZON `T-ZON-04`).

### P3
- **AC-PRK-04** After waves 1, 3 and 4 (A-07) the run shows 3 distinct, unowned perks; the next wave starts only after a pick.
- **AC-PRK-05** Over 10,000 seeded offers, no offer contains a duplicate, an owned perk, or more than `MaxGenericPerOffer` generic perks; hybrids appear about `HybridWeightMultiplier` times as often as equal-weight non-hybrids (±10%).
- **AC-PRK-06** The same seed produces the same offer sequence.
- **AC-PRK-07** Pool data validation fails when generic perks exceed `MaxGenericPoolShare`.
- **AC-PRK-08** Each of the 11 pool perks produces its described effect in PIE or its Functional Test.
- **AC-PRK-09** Perks survive Hero death; hero-trigger effects do nothing while dead and work after respawn; Army/Defense perks work during Commander Spirit Mode.
- **AC-PRK-10** Owned perks show in the perk tray; each trigger pulses its icon and plays its feedback.
- **AC-PRK-11** Offers and picks appear in the run telemetry log.
- **AC-PRK-12** At G3, at least one Section 4.3 build was assembled in a playtest run and the tester notes a changed play pattern (playtest notes).

## 12. Open Questions / Assumptions

| ID | Question / assumption | Class | Default until answered |
|---|---|---|---|
| A-06 | Marked has no prototype source except debug/perk | Master plan | `Exposing Parry` is the perk source |
| A-07 | Offers after waves 1, 3, 4 | Master plan | In `URunDefinition` |
| NEW-PRK-1 | Stat combine formula `(Base + ΣAdd) × (1 + ΣMul)`, clamp ≥ 0, exact tag match | REQUIRED | As stated |
| NEW-PRK-2 | Offer-level cap of generic perks | IMPROVEMENT | 1 per offer |
| NEW-PRK-3 | Perks unique per run, no stacking | REQUIRED | Unique |
| NEW-PRK-4 | Does the perk pick hold the intermission timer? Is the world paused? | REQUIRED (RUN) | RUN step waits for the pick; world not paused |
| NEW-PRK-5 | §33 shows "Heavy Attack causes Armor Broken" as a perk pick, but §10.2 lists Warlord Heavy as a baseline Armor Broken source (same question as SYN NEW-SYN-02) | REQUIRED (design) | Baseline per §10.2 (SYN `T-SYN-02`); pool has the amplifier `Sundering Blows`. If the answer is "perk", the Heavy `AppliedStates` move into a PRK effect (no SYN code change) |
| NEW-PRK-6 | "Every Nth Ballista shot": counter per tower or shared across all Ballistas? | REQUIRED | Per tower |
| NEW-PRK-7 | 7 proposed perks beyond the §23.2 examples need design approval | REQUIRED | Approve/replace in `T-PRK-06` |
| NEW-PRK-8 | Build-affinity weighting (bias offers toward owned categories) | FUTURE | Not built; only if G3 shows builds never assemble |

## 13. System Contract (GDD §38)

### 13.1 Stat modifier query (P2)

| Item | Content |
|---|---|
| Responsibility | Store active stat modifiers and answer "what is stat S for target T given base B" |
| Inputs | Add/remove modifier requests from ZON (P2) and perk effects (P3); queries from consumers (SQD, DEF, CMB) |
| Outputs | Modified value; `OnStatModifiersChanged(Stat)` event |
| State | Active modifier list: handle, stat, op, magnitude, scope (target actor or run-wide filter), source. World lifetime |
| Events | `OnStatModifiersChanged(Stat tag)` |
| Data model | `Stat.<Domain>.<Name>` tags (registry below). Magnitudes live in the source's data (`UTacticalZoneDefinition`, `UPerkDefinition`) |
| Failure cases | Target destroyed (squad wipe, tower destroyed): modifier ignored and pruned. Source destroyed without removal: modifiers pruned when the source is invalid. Unknown stat: base returned |
| Performance | Linear scan over a small list (expected < 50 modifiers), called at event rate, not per frame |

Stat registry (proposed leaves; names may be refined by the consumer feature):

| Stat tag | Op used | Read by | Phase | Used by |
|---|---|---|---|---|
| `Stat.Army.RangedRange` | Multiply | SQD archer target acquisition / fire | P2 | ZON High Ground |
| `Stat.Army.BlockEfficiency` | Add | SQD Infantry damage taken | P2 | ZON Chokepoint |
| `Stat.Army.ReformSpeed` | Multiply | SQD reform / reinforce | P2 | ZON Rally Point |
| `Stat.Hero.StaminaRegen` | Multiply | CMB stamina regen | P3 | Steady Breath |
| `Stat.Hero.ArmorBrokenDuration` | Multiply | CMB/SYN Heavy hit state duration | P3 | Sundering Blows |
| `Stat.Army.AttackSpeed` | Multiply | SQD soldier attack rate | P3 | Rallying Parry |
| `Stat.Army.Damage` | Multiply | SQD soldier hit damage | P3 | Drilled Ranks |
| `Stat.Army.PoiseDamage` | Add | SQD soldier hit poise | P3 | Shieldbreakers |
| `Stat.Tower.FireInterval` | Multiply | DEF tower weapon before each shot (`T-DEF-22`) | P3 | Quick Crank |
| `Stat.Tower.Damage`, `Stat.Tower.Range` | Multiply | DEF tower weapon (`T-DEF-22`) | P3 | none in the P3 pool (available) |
| `Stat.Tower.PoiseDamage` | Add | DEF tower hit poise (read added by `T-PRK-11`) | P3 | Concussive Shells |
| `Stat.Army.ShieldWallDuration` | Multiply | SQD Shield Wall (`T-SQD-19`) | VS | Shield Wall Drill (provisional) |

### 13.2 Perk system (P3)

| Item | Content |
|---|---|
| Responsibility | Hold the player's perks for one run, generate offers, apply/remove effects, report triggers |
| Inputs | RUN offer step; player pick; hero/squad/tower gameplay events; perk pool data |
| Outputs | Offer (3 perk IDs); active effects; stat modifiers; `OnOfferReady`, `OnPerkAcquired`, `OnPerkTriggered`; feedback + telemetry |
| State | Owned perks (Primary Asset IDs + wave acquired), pending offer, offers made, random seed, runtime effect instances. Owner `ARunPlayerState` (§34.2 Player State) |
| Events | `OnOfferReady(Offer)`, `OnOfferResolved(Perk)`, `OnPerkAcquired(Perk)`, `OnPerkTriggered(Perk, Location)` |
| Data model | `UPerkDefinition`: display name, description, icon, categories (`Perk.Category.*`), generic flag, weight, synergy note, instanced effects, trigger feedback tag. `UPerkPoolDefinition`: perk list + offer rules (offer size, hybrid multiplier, generic caps, rerolls). `URunDefinition` (RUN) references the pool and the offer schedule |
| Failure cases | §34.4 Hero death (R-PRK-17), squad wipe, tower destroyed (Section 10), boss reset (no effect), player leaving Siege Site (no effect), Core critical / blocked path opened (no perk impact). Perk-specific: invalid data (validation), empty pool, pick index out of range (ignored + warning), offer while pending |
| Performance | Event-driven; no Tick. Timers only for timed buffs. Offer generation is O(pool) per offer |

## 14. Feedback Contract (GDD §34.5)

Tags are listed in the UXF master table (`13-hud-feedback/spec.md` Section 14, rows FC-56..58); rows authored in `T-PRK-05`.

| State | Visual | Audio | UI | Readability rule | Tag |
|---|---|---|---|---|---|
| Perk offer opens | Cards slide in, world keeps running | Offer chime | Modal with 3 cards, category badges, "Hybrid" label | Each card readable in ≤ 3 s: name, categories, one-line effect with numbers | `Feedback.Perk.Offered` |
| Perk chosen | Card flourish | Confirm sound | Icon added to perk tray | Tray order = pick order | `Feedback.Perk.Chosen` |
| Perk effect fires | Effect VFX on the affected actor (stamina flash on Hero, aura on buffed squad, glowing pierce bolt, Marked icon) | Subtle cue, never louder than §28.3 alerts | Tray icon pulse | The player can tell which perk fired without opening a menu | `Feedback.Perk.Triggered` (+ optional leaf per perk) |
| Zone bonus active (P2) | Small bonus icon on squad marker | none | Squad panel bonus icon | Visible but subtle (§15.4) | owned by ZON / UXF FC-43 |
