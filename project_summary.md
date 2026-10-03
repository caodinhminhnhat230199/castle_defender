# Project Summary: Action Strategy Roguelite (UE5)

Tài liệu tổng hợp cấu trúc dự án: repo có gì, tài liệu nằm ở đâu, làm theo thứ tự nào. Chi tiết kỹ thuật và kế hoạch đầy đủ nằm trong [ai/game/main_implement_plan.md](ai/game/main_implement_plan.md).

Cập nhật: 2026-10-02

---

## 1. Dự án là gì

Game Action Strategy Roguelite góc nhìn thứ ba. Người chơi trực tiếp đánh nhau trên chiến trường (Hero), chỉ huy squad (Army) và xây công trình phòng thủ để định hình đường tiến của địch (Tower). Mỗi run kéo dài khoảng 25 phút: 5 wave và 1 boss.

> "You are not controlling an army. You are fighting inside your army."

| | |
|---|---|
| Engine | Unreal Engine 5 (version cụ thể chốt ở task `T-FND-01`) |
| Platform | PC, single-player, ưu tiên keyboard + mouse |
| Team | 1 dev cùng AI coding agent |
| Hướng code | C++ cho gameplay core, Blueprint cho UI/VFX/tuning |
| Nguồn gốc thiết kế | [GDD v2](GDD_Action_Strategy_Roguelite_Optimized_v2.html), source of truth |

---

## 2. Trạng thái hiện tại

| Hạng mục | Trạng thái |
|---|---|
| GDD v2 | Xong |
| Bộ skill Claude Code (`.claude/skills/`) | Đã cài |
| Kế hoạch tổng, kiến trúc, production plan | Xong |
| Spec / plan / task cho từng feature | Xong: 19 folder, 59 file, 271 task. Đã kiểm tra tham chiếu chéo: không thiếu ID nào, không trùng ID task. |
| Project UE5 (`.uproject`, `Source/`) | **Chưa có**. Toàn bộ tên class/file trong tài liệu chỉ là đề xuất. |
| Git | Chưa init. Làm ở task `T-FND-02`. |
| Quy tắc cho coding agent | Xong: `AGENTS.md` dùng chung cho mọi agent, `CLAUDE.md` cho Claude Code, nhật ký `ai/game/progress.md` |

---

## 3. Cấu trúc repo

```text
game/
├── GDD_Action_Strategy_Roguelite_Optimized_v2.html   GDD gốc (thiết kế gameplay)
├── README.md                     Hướng dẫn setup máy mới
├── project_init.md               Script cài skill, plugin, CodeGraph
├── project_summary.md            File này
├── AGENTS.md                     Quy tắc chung cho mọi coding agent
├── CLAUDE.md                     Claude Code: nạp AGENTS.md + ghi chú riêng
├── .claude/
│   ├── settings.json             Khai báo plugin caveman, ponytail
│   └── skills/                   Skill cho Claude Code (xem mục 8)
└── ai/
    └── game/                     Toàn bộ tài liệu triển khai (mục 4)
```

Sau khi tạo project UE5 (phase Foundation), repo có thêm:

```text
game/
├── <Game>.uproject
├── Source/<Game>/                Code C++ chia theo domain
├── Content/<Game>/               Asset chia theo domain
├── Config/
└── Tools/                        Script chạy test
```

`Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/` và `.codegraph/` không được commit.

---

## 4. Bộ tài liệu `ai/game/`

```text
ai/game/
├── main_implement_plan.md        Kế hoạch tổng: phase, gate, feature, quyết định, câu hỏi mở
├── production-plan.md            Asset cần làm theo phase (placeholder → art thật)
├── spec-audit.md                 Rà soát tài liệu: vấn đề còn mở theo phase
├── progress.md                   Nhật ký từng phiên làm việc của agent
│
├── 00-foundation/                F      Setup project UE5 + kiến trúc chung            10 task
│
├── 01-hero-combat/               P0     Combat của Warlord                              19 task
├── 02-enemies/                   P0→P2  Enemy và hành vi theo lane                      17 task
├── 03-squad-command/             P1     Squad, Command Wheel, AI squad                  21 task
├── 04-battlefield-synergy/       P0→P2  State Staggered / Armor Broken / Marked         11 task
├── 05-structures-pathing/        P2     Tower, Barricade, Core, rule chặn đường         25 task
├── 06-tactical-zones/            P2     Tactical Zone, lệnh theo ngữ cảnh                8 task
├── 07-encounter-director/        P2→P3  Wave, threat budget, Threat Forecast            17 task
├── 08-run-flow/                  P2→P3  Vòng run 25 phút, thắng/thua                    18 task
├── 09-commander-spirit/          P3     Hero chết → chế độ chỉ huy → respawn            10 task
├── 10-tactical-focus/            P3     Slow-time có giới hạn                           12 task
├── 11-perks/                     P2→P3  Perk 1-trong-3, stat modifier                   14 task
├── 12-boss/                      P3     Boss 2 phase                                    15 task
├── 13-hud-feedback/              P0→P3  HUD, feedback hình/âm thanh, telemetry          20 task
│
├── 20-meta-save/                 VS     Meta progression, save có version (tạm)        12 task
├── 21-hub-open-world/            VS     Hub, đoạn open world nhỏ (tạm)                 12 task
├── 22-villager-economy/          VS     Food/Gold/Monster Material, villager (tạm)     12 task
├── 23-conversion-buildings/      VS     Đổi Monster Material thành sức mạnh (tạm)       8 task
└── 24-onboarding/                VS     Tutorial (tạm)                                 10 task
```

"(tạm)" = provisional. Các feature này chỉ bắt đầu sau khi qua gate G3, và phải xem lại vì kết quả prototype sẽ thay đổi chúng.

### Mỗi folder feature có 3 file

| File | Trả lời câu hỏi | Nội dung chính |
|---|---|---|
| `spec.md` | Làm gì, vì sao? | Rule `R-xxx-NN` dẫn về section GDD, scope, anti-goals, edge case, acceptance criteria `AC-xxx-NN`, system contract (input/output/state/event/data), feedback contract |
| `technical-plan.md` | Làm thế nào trong UE5? | Owner và lifetime, class UE, data asset, runtime flow, chia C++/Blueprint, rủi ro performance, bảng map requirement sang phần kỹ thuật |
| `tasks.md` | Làm theo thứ tự nào? | Task `T-xxx-NN` chia theo phase. Mỗi task có mục tiêu, dependency, các bước, file dự kiến, test case, acceptance criteria, cách verify. Cuối file có dependency graph. |

Riêng `00-foundation/technical-plan.md` là **kiến trúc UE5 chung** cho cả dự án. Mọi feature phải theo file này.

Tổng cộng 271 task: 217 cho Foundation và prototype (P0–P3), 54 cho Vertical Slice (tạm).

### Hợp đồng giữa các feature

Main plan mục **8a** liệt kê những gì feature này cung cấp cho feature khác, ví dụ:
- mọi đòn đánh đi qua `UCombatLibrary::DeliverHit`;
- spawner trả về null khi chạm giới hạn số enemy;
- hit stop chỉ làm chậm từng actor, không đụng global time dilation (global dành cho Tactical Focus).

Khi làm task cung cấp, đọc cột "Consumers" để không làm vỡ feature khác.

---

## 5. Lộ trình phase

```text
Foundation → P0 Combat Sandbox → P1 Combined Arms → P2 Defense & Pathing → P3 Full Run → Vertical Slice → Launch
                 (G0)               (G1)               (G2)                  (G3)          (VS Gate)
```

| Phase | Câu hỏi cần trả lời | Không qua gate thì |
|---|---|---|
| Foundation | Project build, test, debug được chưa? | Chưa làm gameplay |
| P0 | Combat Hero có đã tay khi chưa có RTS/TD không? | Không thêm army, tower |
| P1 | Hero + squad có hay hơn Hero một mình không? | Sửa command/AI |
| P2 | Đặt tower và pathing có tạo quyết định chiến thuật không? | Sửa placement/pathing |
| P3 | Chơi xong 1 run có muốn chơi lại ngay không? | Không làm villager, open world, class mới |
| VS | Một lát cắt hoàn chỉnh có chứng minh được vòng chơi, pipeline và performance không? | Không scale content |

Không đặt deadline cứng. Chỉ chuyển phase khi qua gate (GDD §32). Checklist từng gate nằm trong main plan, mục 3.

**Các mốc demo** (chi tiết: main plan mục 5, "Demo Milestones"):

| Demo | Sau phase | Chơi được gì | Task để chơi được | Task để qua gate |
|---|---|---|---|---|
| D0 | Một phần F | Nhân vật đi lại trong map trống | 9 | — |
| **D1** | **P0** | Warlord đánh với enemy cận chiến. Lần đầu test cảm giác chơi. | 29 | 40 |
| D2 | P1 | Hero cùng 2 squad, Command Wheel | ~60 | 73 |
| D3 | P2 | Xây tower, enemy phá công trình, 3 wave. Lần đầu thấy bản sắc của game. | ~110 | 128 |
| **D4** | **P3** | Run đầy đủ khoảng 25 phút. Bản đầu tiên đúng là game. | ~180 | 203 |
| D5 | VS | Bản polish cho người ngoài xem | — | 271 |

**Critical path:** Foundation → Hero Combat → Enemy → Squad → Synergy → Structures & Pathing → Director → Run Flow → Boss.

**Rủi ro kỹ thuật lớn nhất:** rule chặn đường và minimum-break path (GDD §14). Có spike `T-DEF-01` để kiểm chứng sớm.

---

## 6. Kiến trúc UE5 tóm tắt

Chi tiết: [00-foundation/technical-plan.md](ai/game/00-foundation/technical-plan.md) và main plan, mục 7.

| Chủ đề | Quyết định |
|---|---|
| Module | 1 runtime module `<Game>`. Không làm plugin cho feature. |
| Folder | Chia theo domain: `Combat, Hero, Army, Enemy, Structures, Navigation, Encounter, Run, Perks, Boss, UI…` |
| C++ / Blueprint | C++ giữ rule, state, AI, pathing. Blueprint lo content, tuning, animation, VFX, UI. |
| GAS | Không dùng ở prototype. Dùng Gameplay Tags và component nhẹ. Xem lại ở G3. |
| Damage | Chung một pipeline: `UHealthComponent` + `FCombatHit` + `UCombatStateComponent` |
| Data | Primary Data Asset cho mọi loại content. Mọi số [TUNABLE] nằm trong data, không hard-code. |
| AI | FSM C++ nhẹ cho enemy, soldier, squad. Squad di chuyển theo anchor + formation slot. |
| Navigation | NavMesh cho di chuyển cục bộ. Lane layer chiến lược quyết định route, vật cản và đường phá ít tốn nhất. |
| UI | UMG, widget chỉ quan sát state. CommonUI để sau. |
| Input | Enhanced Input, mỗi mode một mapping context, sẵn sàng map gamepad về sau |
| Save | Prototype chưa có save. Meta save có version từ Vertical Slice. |
| Performance | Đo trước rồi mới tối ưu. Giới hạn số enemy lấy từ benchmark ở P2. |
| Test | Automation Spec cho logic, Functional Test cho kịch bản gameplay, playtest checklist cho mỗi gate |

---

## 7. Quy ước chung

| Loại | Format | Ví dụ |
|---|---|---|
| Rule trong spec | `R-<FEAT>-NN` | `R-DEF-03` |
| Acceptance criteria | `AC-<FEAT>-NN` | `AC-CMB-04` |
| Task | `T-<FEAT>-NN` | `T-SQD-07` |
| Quyết định kiến trúc | `D-NN` | `D-09` |
| Giả định | `A-NN` | `A-04` |
| Câu hỏi mở | `Q-NN` | `Q-05` |

Mã feature: `FND CMB ENM SQD SYN DEF ZON DIR RUN CSM TFM PRK BOS UXF MET WLD ECO CNV ONB`.

Nhãn từ GDD:
- `[LOCKED]`: phải theo.
- `[TUNABLE]`: đưa thành data.
- `[OPEN]`: không tự chốt.
- `[DEFERRED]`: không làm ở scope hiện tại.

Tên asset: `BP_`, `DA_`, `DT_`, `ABP_`, `AM_`, `IA_`, `IMC_`, `WBP_`, `NS_`, `L_`, `FT_` (map test).

---

## 8. Skill dùng khi nào

| Skill | Dùng khi |
|---|---|
| `game-development-workflow` | Spec, kế hoạch, task, prototype, playtest, QA, gate |
| `ue5-project-architecture` | Chọn class UE, module, data, AI, UI, save, performance |
| `planning-with-files` | Lưu tiến độ ra file để làm tiếp qua nhiều session |
| `debugging-and-error-recovery` | Lỗi build, crash, bug |
| `code-review-and-quality` | Review trước khi merge |
| `documentation-and-adrs` | Ghi lại khi đổi một quyết định `D-xx` |
| `source-driven-development` | Đối chiếu API UE5 với tài liệu chính thức |
| `idea-refine` | Mài ý tưởng mới trước khi đưa vào scope |
| `git-workflow-and-versioning` | Branch, commit, PR |
| `stop-slop` | Viết tài liệu cho gọn, không có văn phong AI |

---

## 9. Cách làm việc hằng ngày

Quy tắc đầy đủ cho agent nằm trong [AGENTS.md](AGENTS.md). Tóm tắt:

1. Đọc các mục mới nhất trong [progress.md](ai/game/progress.md), rồi mở [main_implement_plan.md](ai/game/main_implement_plan.md) để xem đang ở phase nào.
2. Chọn task của phase đó trong `tasks.md` của từng feature, theo đúng dependency.
3. Implement, rồi chạy test (Automation/Functional), rồi kiểm tra trong PIE ở map sandbox của phase.
4. Đánh dấu `Status` của task (`Todo → In Progress → Review → Done`). Commit nhỏ theo từng task. Ghi một mục vào `progress.md`.
5. Hết phase thì chạy gate playtest theo checklist và ghi kết quả KEEP / CHANGE / DELETE.
6. Có ý tưởng mới: chạy intake check (main plan, mục 10). Ý tưởng hay chưa chắc thuộc scope hiện tại.

---

## 10. Việc cần làm tiếp

1. Chốt **Q-15**: tên project, tên module, version UE 5.x, cấu hình PC tham chiếu.
2. Làm hết [00-foundation/tasks.md](ai/game/00-foundation/tasks.md) (T-FND-01 → 10).
3. Bắt đầu P0: `01-hero-combat`, phần P0 của `02-enemies`, `04-battlefield-synergy` (T-SYN-01) và `13-hud-feedback`.
4. Xem lại 11 giả định (A-01…A-11) và 17 câu hỏi mở (Q-01…Q-17) trong main plan, mục 11.
5. Chốt các câu hỏi cấp feature quan trọng (main plan mục 8a, "Feature-level open questions"):
   - **Trước P0:** parry thành công thì gây stagger hay mở vulnerability window? Parry có nút riêng không? (NEW-CMB-01)
   - **Trước P1:** GDD đang mâu thuẫn. §10.2 và §19.1 nói Heavy của Warlord gây Armor Broken mặc định, nhưng ví dụ run ở §33 lại cho đó là perk. Bên nào đúng? (NEW-SYN-02)
   - **Trước P2:** enemy có được đi vòng qua khe hở trong lane không (maze), hay luôn phá thẳng? (NEW-DEF-02)
6. Mỗi `spec.md` có mục 12 liệt kê câu hỏi `NEW-<FEAT>-n` kèm giá trị mặc định. Nếu không trả lời thì dùng mặc định.
