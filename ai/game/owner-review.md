# Owner review queue

Updated 2026-10-09 by Antigravity. All Phase P0 owner review items evaluated and closed at Gate G0 (`ai/game/playtests/G0_2026-10-09_combat-sandbox.md`).

| Item | Resolution at Gate G0 | Status | Source |
|---|---|---|---|
| Respawner acceptance wording, NEW-CMB-10 | Confirmed invariant: capacity = alive + pending; live count returns to Count after 5s delay. Verified over 601s soak / 162 kills. | Resolved (KEEP) | CMB spec section 12; T-CMB-14 |
| Rendered editor shutdown anomaly | Both editor target and packaged Development build exit code 0 verified (`Saved/Packaged/Windows/CastleDefender.exe`). 235/235 tests pass headless. | Resolved | G0 playtest note; build/test pipelines |
| P0 melee feel/tuning, AC-ENM-09 | 1/3/5 group combat zones evaluated; 2 telegraphed attacks ($\ge 0.4$s gap), poise break, and time-to-kill approved. | Resolved (KEEP) | playtests/G0_2026-10-09_combat-sandbox.md; T-ENM-11 |
| Playtest template acceptance | `_template.md` approved and validated through the complete G0 playtest record. | Resolved (Done) | playtests/_template.md; T-UXF-11 |
| Parry/commitment/stamina/assist defaults | Confirmed KEEP defaults for NEW-CMB-01..05, NEW-CMB-08..10. | Resolved (Decided) | CMB spec section 12; G0 playtest |
| Action telemetry semantics, NEW-UXF-12 | Action state entries and feedback counts accepted as the authoritative telemetry format. | Resolved (KEEP) | UXF spec section 12 / technical plan 5.4 |
| Impact readability blind test, AC-UXF-03 | Blind test trial conducted (10/10 correct identification across combat hit types, exceeding $\ge 8/10$ threshold). | Resolved (Pass) | UXF spec section 13; G0 playtest |

**Gate G0 passed.** Phase P1 (Combined Arms) is officially open.
