# Production Plan: Content for Prototypes and Vertical Slice

| | |
|---|---|
| GDD sections | §9.8, §20, §21, §22, §28.2–28.3, §32, §35 |
| Scope | P0–P3 placeholder content, VS representative content |
| Status | Draft v1. VS part is provisional until G3. |

Rule from `game-development-workflow`: gameplay-critical blockouts before polish; placeholder art through all prototypes; no final art before dimensions, timings and interactions pass their gate.

## 1. Production Overview

| Phase | Art bar | Goal of content |
|---|---|---|
| P0–P3 | Placeholder: mannequins, grey boxes, primitive meshes, sample/free animation, simple Niagara, library SFX | Test feel and readability only. Silhouette and color coding must already follow §28.2. |
| VS | Near target quality for one biome, one Siege Site, one boss, Warlord, 3 squads, 3–4 towers | Prove art direction (Q-13), pipeline cost and performance. |
| Launch | Scale with the VS pipeline | Not planned here. |

## 2. Asset Inventory

### Characters and animation

| Asset | Phase | Placeholder source | Gameplay-critical notes | VS target |
|---|---|---|---|---|
| Warlord hero | P0 | UE mannequin + sample locomotion (e.g. Epic Game Animation Sample) + free melee pack from Fab (check license) | Sword + shield. Montages: light ×3, heavy (clear startup), dodge 4/8-dir, block idle/hit/break, parry + counter, hit reactions (light/heavy), stagger, death, sprint, lock-on strafe, interact | Final mesh, rig, anim set retimed to tuned windows |
| Melee test enemy | P0 | Mannequin variant, different color + material | Telegraph pose readable at 10 m; hit react; stagger; death | Replaced by archetypes |
| Swarm | P1 | Small, fast mannequin variant | Readable as a group; cheap anim (shared skeleton) | Final, LOD-heavy |
| Armored | P1 | Bulky variant + armor plates (static meshes) | Armor plates visibly drop/flash on Armor Broken | Final |
| Giant / Siege | P2 | Scaled-up mannequin + ram/hammer prop | Structure-attack anim; silhouette distinct from Armored | Final |
| Infantry soldier | P1 | Mannequin + shield, ally color | Same skeleton as hero for anim reuse | Final |
| Archer soldier | P1 | Mannequin + bow | Draw/release anim; arrow projectile | Final |
| Spearman | VS | — | Brace pose | Final |
| Boss (Siege Behemoth variant) | P3 | Large blockout + scaled anim or simple rig | Telegraphs per attack; phase-change pose | Polished boss |
| Villagers | VS | — | Work loops, flee | Final |

### Structures and props

| Asset | Phase | Placeholder | Notes |
|---|---|---|---|
| Core | P2 | Large primitive with HP-state material | Damage states at 66/33% visible |
| Barricade | P2 | Box/spike wall mesh | Footprint = grid cells; damaged state |
| Ballista | P2 | Primitive frame + bolt | Shape reads "single-target heavy" |
| Bombard | P2 | Primitive mortar + ball | Shape reads "AoE"; arc projectile |
| Build ghost/preview | P2 | Translucent valid/invalid material | Green/red + blocked-route warning |
| Lane/route markers | P2 | Decals/splines (debug style) | Forecast lane direction reuse |
| Tactical Zone markers | P2 | Ground decal + floating icon | Visible in Tactical Focus |
| Conversion building (one) | VS | — | Utility silhouette |
| 4th tower (if approved) | VS | — | Role must pass §21.3 |

### VFX

| Effect | Phase | Notes |
|---|---|---|
| Hit normal / armored / parry / stagger | P0 | Four clearly different sparks/colors (§9.8) |
| Block impact, block break | P0 | Shield flash, break burst |
| Marked / Armor Broken / Staggered state | P1 | Consistent icon + body VFX across hero/army/tower sources (§28.2) |
| Projectile trails + Bombard splash | P2 | Splash radius readable |
| Structure damage / destruction | P2 | Debris burst, dust; cheap |
| Tactical Focus overlay | P3 | Post-process desaturate + outline |
| Commander Spirit view | P3 | Distinct post-process |
| Boss telegraphs | P3 | Ground indicators per attack |

VFX density uses Niagara scalability/LOD from the start (§31.2).

### Audio (GDD §28.3 event list first)

| Event | Phase |
|---|---|
| Impact per armor/material (flesh, armor, shield, wood/stone structure) | P0 |
| Successful parry | P0 |
| Armor break | P1 |
| Tactical order acknowledged (per squad type voice/bark or stinger) | P1 |
| Squad low strength | P1 |
| Tower critical HP | P2 |
| Core under attack | P2 |
| Wave start / forecast reveal | P3 |
| Boss phase change | P3 |
| Tactical Focus enter/exit, Commander Spirit enter/respawn | P3 |

Placeholder from royalty-free libraries; MetaSounds for variation. Audio concurrency limits set per category before P2 (many hits at once).

### UI

| Element | Phase |
|---|---|
| HP/stamina, lock-on marker | P0 |
| Command Wheel, squad cards/markers, order icons | P1 |
| Enemy class icons, state icons | P1 |
| Build menu, placement feedback, structure HP bars, Core HP | P2 |
| Forecast panel, wave timer, perk choice cards, Focus meter, respawn timer, boss bar, resolve screen | P3 |
| Hub, unlock screens, settings | VS |

### Levels

| Map | Phase | Content |
|---|---|---|
| `L_CombatSandbox` | P0 | Flat arena + some cover, enemy spawner, tuning kiosk (debug) |
| `L_CombinedArms` | P1 | Open field + one choke + one bridge-like narrow; 2 squads |
| `L_SiegeSite_Proto` | P2–P3 | Core, 2 lanes, build zones, bridge + high ground + chokepoint zones, boss entry |
| `FT_*` test maps | F–P3 | One per Functional Test scenario |
| VS Siege Site + biome slice + hub + open world segment | VS | Handcrafted macro terrain (§5.2, §27.1) |

## 3. Placeholder / Final Asset Strategy

- All placeholders live in `Content/<Game>/Placeholder/` so they are easy to find and delete before VS art lock.
- One shared humanoid skeleton for hero, soldiers and humanoid enemies until VS (animation reuse, retargeting).
- Color code from P0: ally blue family, enemy red family, elite/siege marked with a distinct accent; state colors fixed (Marked, Armor Broken, Staggered) and never reused for anything else.
- Final art starts only for assets whose gameplay dimensions passed a gate (e.g., Warlord after G0 timings are tuned, structures after G2 footprints are stable).

## 4. Production Order

```text
P0: Warlord anim set (placeholder) → hit VFX/SFX set → L_CombatSandbox
P1: Soldier variants + archer anim → Swarm/Armored variants → state VFX → command UI → L_CombinedArms
P2: Structure blockouts → Giant/Siege → projectile VFX → structure/Core audio → L_SiegeSite_Proto blockout
P3: Boss blockout + telegraphs → forecast/perk/focus/respawn UI → Focus + Spirit post-process → pacing audio
VS: art direction lock (Q-13) → Warlord final → enemies final → structures final → biome/Siege Site art pass → boss polish → UI skin → audio pass
```

## 5. Dependencies

| Content | Blocked by |
|---|---|
| Warlord montages with final timings | CMB tuning at G0 |
| Structure footprints final | DEF placement model decision (Q-05) at G2 |
| Siege Site final layout | G2 lane/zone learnings, G3 pacing |
| Boss final | BOS phase design proven at G3 |
| Any VS art | G3 pass + Q-13 art direction |

## 6. Reuse Opportunities

- One humanoid skeleton and shared locomotion for hero, soldiers, humanoid enemies.
- Enemy variants as material/scale/prop swaps on shared meshes in prototypes.
- One world-marker widget for squads, enemies and structures (UXF).
- State VFX shared across all sources (hero, army, tower), tied to state tags.
- Siege Site proto map reused across P2 and P3.

## 7. Engine Integration Checkpoints

| Checkpoint | When | Check |
|---|---|---|
| Anim notify windows match data | Each CMB timing change | Hit/i-frame/cancel/parry windows driven by notify states, values match `DA_HeroClass_Warlord` |
| Skeleton/retarget sanity | P1 start | Soldiers and enemies play hero-skeleton anims |
| Collision/nav footprints | P2 | Structure collision = grid footprint = nav modifier area |
| Audio concurrency | P2 | 50+ simultaneous hit events do not clip or starve important cues (§28.3 priority list) |
| Scalability | P3 | Niagara/VFX scalability settings applied; Focus overlay cost measured |

## 8. Performance / Content Budget Notes

- Enemy on-screen count: set by `T-DEF-12` benchmark (Q-04). Until then, design waves for ≤ 60 concurrent enemies as a working number, not a commitment.
- Skeletal mesh count is the expected bottleneck: plan animation LOD/update-rate optimization (verify UE feature names such as URO for the pinned version) before cutting enemy counts.
- VFX: max simultaneous hit effects capped by Niagara scalability; crowd readability over raw count (§31.2).
- Texture/memory budgets: defined at VS against the reference PC, not before.

## 9. Risks

| Risk | Mitigation |
|---|---|
| Placeholder animation hides or fakes combat feel | Prefer sample packs with real weight; tune timings in data; record G0 with notes about anim limits |
| Licensing of free packs | Check Fab/third-party license before use; keep a `Placeholder/LICENSES.md` list |
| Art direction undecided (Q-13) blocks VS | Run a small style test (one character, one structure, one terrain patch) right after G3 |
| Readability lost when real art arrives | Keep §28.2 color/silhouette rules as acceptance criteria for every final asset |

## 10. Acceptance Criteria

- Every prototype gate playtest runs on placeholder content only.
- Ally/enemy, Swarm/Armored/Giant, tower roles and the three combat states are distinguishable in a 1–2 second glance at gameplay camera distance (§28.1–28.2), checked at G1, G2 and G3.
- Every §28.3 audio event has a placeholder sound by the end of the phase that introduces it.
- `Content/<Game>/Placeholder/` is empty of gameplay-referenced assets at VS art lock.
