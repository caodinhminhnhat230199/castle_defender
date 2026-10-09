# Owner review queue

Updated 2026-10-09 by Codex. These items remain separate from mechanical implementation/testing. Continue eligible P0 work; G0 and later phases remain closed until their required evidence/decisions are recorded.

| Item | Current evidence / question | Owner action | Source |
|---|---|---|---|
| Respawner acceptance wording, NEW-CMB-10 | T-CMB-14 specifies both a five-second replacement delay and alive count always equals Count. A 601-second/162-kill soak verifies alive + pending capacity and delayed restoration. | Confirm the intended invariant/wording; task remains Review. | CMB spec section 12; CMB tasks T-CMB-14 |
| Rendered editor shutdown anomaly | Some earlier D3D11 runs returned native AV after clean logged shutdown; later complete HUD/telemetry runs exit 0. No new matching crash report was found for the late exits. | Keep this limitation visible at G0; investigate/reproduce before broad stability approval. | progress entries for T-CMB-14/UXF-02 |
| P0 melee feel/tuning, AC-ENM-09 | Saved playable DA/BP, red variant and 1/3/5 areas are implemented. Scripted five-minute God/no-sound soak verifies actions/attacks/damage/replacement; it cannot establish HP pressure or boredom/readability. | Play each group normally with God off; record time-to-kill, HP/stamina pressure, Dodge/Parry readability and KEEP/CHANGE/DELETE. | playtests/2026-10-09_G0_melee-baseline.md; T-ENM-11 |
| Playtest template acceptance | Template and G0 recording dry run are ready; T-UXF-11 is Review. | Review the fields and confirm the template fits gate recording. | playtests/_template.md; 2026-10-09_G0_recording-dry-run.md |
| Parry/commitment/stamina/assist defaults | NEW-CMB-01..04/08 remain documented defaults, implemented mechanical behavior has rendered/spec evidence. | Evaluate these defaults at the G0 feel pass; record changes through their owning specs/data. | CMB spec section 12 |
| Action telemetry semantics, NEW-UXF-12 | T-UXF-08 records the specified action-state entries and played feedback. Physical/rejected key attempts and unchanged combo links need an additional provider event if wanted. | Decide whether broader attempt counting is needed at G0; no inferred polling counts were added. | UXF spec section 12 / technical plan 5.4 |
| Impact readability blind test, AC-UXF-03 | Hit stop, camera shakes, physical surfaces and DT_Feedback rows authored. All automated specs and functional tests pass. | Conduct blind test with tester naming hit type in ≥ 8/10 at G0. | UXF spec section 13; UXF tasks T-UXF-03 |

Blind sound/readability and final feedback/gate audit requirements remain in their owning tasks. CLI green, editor builds and mechanical fixture results do not approve G0 feel, packaging/release readiness or owner decisions. No commit/push/PR authorization is inferred from continuation work.
