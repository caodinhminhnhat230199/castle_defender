# Hero Combat — WIP và handoff

## Current handoff (2026-10-09, Codex)

This section supersedes the historical snapshot below. Branch: `feat/01-hero-combat`. CMB has **17 Done, 4 Todo, no Review tasks**. All P0 CMB tasks are Done; [G0 passed on 2026-10-09](../playtests/G0_2026-10-09_combat-sandbox.md), opening P1. Check [tasks.md](tasks.md), [progress.md](../progress.md) and the live Git diff before acting.

- **Completed P0:** T-CMB-01..11, 13..16, 20..21. T-ENM-11/12 and T-UXF-09/10/11 are recorded Done; the [owner review](../owner-review.md) records G0 resolutions. Skill adaptation does not reopen these tasks.
- **Remaining CMB:** T-CMB-12 (Interact, P2); T-CMB-17/18/19 (provisional VS proximity buff, trait design spike and production animation polish). None is eligible in current P1, even when individual dependencies are Done.
- **Current P1 path:** T-SQD-01 is In Progress (squad spawning/registry foundation); see the newest progress entry for build/content/PIE limits. T-ENM-13 and T-SYN-02 remain dependency-ready Todo. T-UXF-04 waits for T-SQD-01/T-ENM-06. Retain the shared branch and phase rules.
- **Skills:** use the [five adapted guides](../skill-integration.md) and [alignment review](../skill-alignment-review-2026-10-09.md). Components, stamina, the three-hit chain/reset on Dodge, single-use Parry and existing animation/feedback ownership remain the design.
- **Fresh verification:** editor build and full automation gate pass, 235/235, zero warnings/failures/NotRun, editor exit 0 (`Saved/skill-recheck-editor-build.log`, `Saved/skill-recheck-full-tests.log`). Build has the existing MSVC preference notice. This review did not repeat rendered PIE, human feel/audio testing or packaging; use the recorded G0 acceptance for those results.
- **Delivery:** this session preserves uncommitted skill/documentation work and original uploads. No branch change, commit, push or PR. Earlier push authorization/checkpoints are historical, not a new remote verification or permission to publish this review.

## Historical snapshot (superseded where it conflicts with the handoff above)

Cập nhật: 2026-10-05 · Agent: Claude Code (phần mesh/animation; bản trước: Codex 2026-10-04) · Branch: `feat/01-hero-combat`.

Tài liệu này lưu danh sách công việc còn lại và lý do chưa hoàn tất để teammate tiếp tục. Đây là snapshot; trước khi làm hãy đối chiếu [tasks.md](tasks.md), [technical-plan.md](technical-plan.md), [spec.md](spec.md), [progress.md](../progress.md) và Git diff hiện tại.

## Workflow đã được thống nhất

- Mọi task Hero Combat dùng chung branch `feat/01-hero-combat`, chia thành nhiều commit nhỏ theo task. Không tạo branch riêng cho từng task và không commit feature trực tiếp vào `main`.
- Chỉ tạo PR vào `main` khi tất cả task của feature đã Done và có kết quả kiểm chứng được ghi lại. Vẫn tuân thủ gate pha và dependency; không làm trước task của pha chưa mở.
- Khi lưu handoff này, feature đang ở P0; chưa tạo PR hoặc push. Phần mesh/animation, foot IK, hướng camera và Heavy đã được commit trên `feat/01-hero-combat` (2026-10-05, user đồng ý).
- Chỉ dùng 1 branch `feat/01-hero-combat`, nhiều commit (user chọn 2026-10-06). Dependency thuộc feature khác (UXF, SYN, …) cũng commit thẳng trên branch này, không tạo branch riêng. T-UXF-01 và T-SYN-01 trước đây làm trên `feat/13-hud-feedback` / `feat/04-battlefield-synergy`; cả hai đã merge vào `feat/01-hero-combat` và đã xoá.

## Tình trạng tổng thể

Theo [tasks.md](tasks.md), feature có **21 task: 5 Done, 5 Review, 11 Todo**. Như vậy còn **16 task chưa hoàn tất**.

Các task Done: **T-CMB-01, 02, 03, 04, 13**.

## 5 task đã có code và kiểm thử, nhưng còn thiếu nội dung hoặc tích hợp để đạt Done

| Task | Nội dung | Vì sao chưa hoàn tất |
|---|---|---|
| **T-CMB-05** | Chuỗi 3 đòn đánh nhẹ | Đã lắp Manny + `ABP_Warlord` + lưỡi placeholder trên `hand_r`; montage dùng 3 đòn unarmed của template. PIE headless với sweep thật: dummy 100→90→80→66. Còn thiếu: chơi thử PIE có hình ảnh với input thật và không có warning mới. |
| **T-CMB-07** | Né với khoảng bất tử | Né dùng `MM_Dash` có root motion (B phát ngược, scale 0.4); PIE: né lùi ~225 cm. Template không có clip né ngang nên L/R tạm dùng dash tiến. Còn thiếu: kiểm tra có hình ảnh AC-CMB-07 với input thật. Né khi lock-on tích hợp cùng T-CMB-10. |
| **T-CMB-11** | Phản ứng trúng đòn, chết và hồi sinh | Đã có clip hit react trước/sau và death (giữ tư thế tới khi respawn); PIE: HitReact_F, chết, respawn sau 3 s. Còn thiếu: kiểm tra có hình ảnh. Phát feedback chết thuộc UXF; dọn lock-on tích hợp khi T-CMB-10 có mặt. |
| **T-CMB-21** | Debugger combat và hiển thị trace | Debugger đã hoạt động. Còn thiếu kiểm chứng đường quét vũ khí được animation điều khiển, cùng coverage đầy đủ cho các hành động P0A chưa có như Heavy và rotation assist. |

Các thiếu sót này được ghi cụ thể trong [handoff và kết quả kiểm chứng](../progress.md).

## 7 task Todo thuộc pha P0 hiện tại (T-CMB-06 đã lên Review)

| Task | Nội dung | Vì sao chưa hoàn tất |
|---|---|---|
| **T-CMB-06** | Đánh mạnh, sát thương poise cao | **Review (2026-10-05).** T-UXF-01 và T-SYN-01 đã Done và đã merge vào branch này. Test: Heavy + Light làm vỡ poise của dummy đúng số đòn theo DA, hook Armor Broken, từ chối khi thiếu stamina; 107/107 test. Còn chờ: chơi thử có hình ảnh với `game.debug.CombatStates 1` (Heavy → Light → Light → Staggered). |
| **T-CMB-20** | Hỗ trợ xoay hướng khi đánh | Phụ thuộc **T-CMB-05 và 06** hoàn tất, để tích hợp và kiểm chứng trên cả Light/Heavy. |
| **T-CMB-08** | Đỡ đòn và vỡ thế đỡ | Chờ **SYN-01**, cùng **CMB-07/11/20/21** đạt Done. Thuộc phần P0B, sau checkpoint P0A. |
| **T-CMB-09** | Parry và cửa sổ phản công | Chờ **CMB-08** và **SYN-01**, vì dùng chung xử lý phòng thủ và poise của đối phương. |
| **T-CMB-10** | Lock-on và chuyển mục tiêu | Chờ **CMB-07/11/20/21** hoàn tất trước khi tích hợp né, chết, hỗ trợ xoay hướng và debugger với mục tiêu khóa. |
| **T-CMB-14** | Respawn enemy và các kịch bản sandbox | Chờ **T-ENM-01 và 03**: enemy thực tế và hành vi đánh cận chiến. Cả hai vẫn Todo; dummy hiện tại chưa đủ cho task này. |
| **T-CMB-15** | Bộ kiểm thử combat đầy đủ | Các cơ chế cần kiểm thử chưa hoàn tất. Đã có automation test theo từng task, nhưng chưa có bộ Functional Test đầy đủ cho Heavy, Block, Parry, Lock-on, assist và các tình huống tích hợp. |
| **T-CMB-16** | Playtest và xét gate G0 | Chờ **CMB-14/15**, enemy, synergy, HUD/feedback và telemetry đạt yêu cầu. Build/test hiện có chưa thay thế được playtest combat và hồ sơ đánh giá G0. |

Dependency ngoài feature được theo dõi tại [04-battlefield-synergy/tasks.md](../04-battlefield-synergy/tasks.md), [02-enemies/tasks.md](../02-enemies/tasks.md) và [13-hud-feedback/tasks.md](../13-hud-feedback/tasks.md).

## 4 task Todo thuộc các pha sau

Các task này chưa được phép triển khai ở P0.

| Task | Pha | Nội dung và lý do chờ |
|---|---|---|
| **T-CMB-12** | P2 | Tương tác `IInteractable`. P2 chưa mở; kế hoạch chủ động để task này đến pha đó. |
| **T-CMB-17** | VS | Buff squad gần Warlord. Chờ G3 và hệ thống squad/stat modifier; thiết kế còn provisional, phải đánh giá lại. |
| **T-CMB-18** | VS | Thử nghiệm thiết kế rally/charge/hold-line. Chờ G3 và CMB-17; cần quyết định thiết kế trước khi có code production. |
| **T-CMB-19** | VS | Polish animation production. Chờ combat được kiểm chứng qua CMB-15/16 và bộ animation VS; hiện vẫn đang dùng placeholder. |

## Thứ tự tiếp tục

1. Đọc root `AGENTS.md`, handoff mới nhất trong `progress.md`, kiểm tra `git status` và diff. Tiếp tục trên `feat/01-hero-combat`, giữ nguyên công việc đang có của teammate.
2. **Đã xong (2026-10-05):** content placeholder cho **T-CMB-05/07/11**, lấy từ bộ Mannequin có sẵn trong template Third Person của UE 5.8. Fab library trên máy trống, nên không có pack Epic nào khác. Xem mục "Content placeholder" bên dưới. Việc tiếp theo: mở `L_CombatSandbox`, chơi thử có hình ảnh với input thật (Light ×3, chờ reset, Dodge có và không có input, `DebugHitHero 20 0 1`, `KillHero`) cùng `game.debug.Combat 1` / `game.debug.CombatTrace 1`. Nếu đạt và log không có warning mới thì nâng task lên Done.
3. Chạy build, automation và PIE có hình ảnh để kiểm chứng input, chuyển động, hit trace, phản ứng/chết/respawn. Chỉ nâng từng task lên Done khi đủ acceptance criteria; cập nhật `tasks.md` và `progress.md`.
4. **Đã xong (2026-10-05):** T-UXF-01 và T-SYN-01 Done, đã merge vào `feat/01-hero-combat`, T-CMB-06 lên Review. Còn lại: **T-CMB-20** (cần T-CMB-05 và 06 Done); hoàn tất phần kiểm chứng còn thiếu của **T-CMB-21** và ghi checkpoint P0A.
5. Tiếp tục **T-CMB-08/10 → 09/14 → 15 → 16** khi dependency tương ứng đã Done. T-CMB-14 cần enemy provider; G0 cần các task ENM/SYN/UXF liên quan hoàn tất.
6. **T-CMB-12/17/18/19** tiếp tục chờ pha tương ứng mở. Feature này trải qua **P0, P2 và VS**, nên “xong tất cả task” còn bao gồm công việc ở những pha chưa mở.

## Content placeholder (2026-10-05)

- **Nguồn:** `E:/Program Files/Epic Games/UE_5.8/Templates/TemplateResources/High/Characters/Content/Mannequins` (engine sample, Epic EULA). `Tools/create_hero_placeholder_content.py` copy phần cần dùng vào `Content/CastleDefender/Placeholder/Mannequins/`. Ghi nhận trong `Placeholder/LICENSES.md`.
- **Model hero (2026-10-06):** `BP_Hero_Warlord` dùng `Placeholder/Characters/Warlord/SKM_Warlord` (user tạo bằng Tripo3D, rig bằng AccuRig), không còn dùng `SKM_Manny_Simple`. Mesh có đủ bone UE5 Manny đúng cha, cộng thêm 47 bone `cc_base_*` (mặt, mắt, hàm, twist) đã gộp vào `SK_Mannequin`, nên mọi montage và `ABP_Warlord` vẫn dùng chung skeleton. Cao 180.8 cm, giống Manny. `M_Warlord` lấy màu từ `Texture.jpg` và được gán làm override material trên component, vì mỗi lần reimport FBX sẽ reset slot của mesh. Chạy lại `Tools/import_hero_model.bat` sau khi thay FBX trong `SourceModels/Hero/`: script tự scale theo Manny, từ chối rig UE4 (cha của bone khác Manny) và chỉ gán vào BP khi bone khớp. Các export Tripo trước đó có layout UE4 hoặc Mixamo nên đã bị từ chối.
- **Hero:** `SKM_Manny_Simple`, `Hero/ABP_Warlord` (copy của `ABP_Unarmed`, có `DefaultSlot`, root motion chỉ từ montage). ABP có Control Rig `CR_Mannequin_FootIK` sau `DefaultSlot`. Trước đây nó ghim chân xuống đất khi đang đánh, nên chân ở Light 3 không theo animation. Giờ ABP có class cha C++ `UHeroAnimInstance`: `FootIKAlpha` về 0 khi có montage đang chạy và trở lại 1 khi locomotion (fade 0.15 s, `FootIKBlendTime`). Giá trị này nối vào pin Alpha của node Control Rig.
- **Vũ khí:** demo P0 không cầm vũ khí (user yêu cầu 2026-10-05; lưỡi cube dọc cẳng tay trước đó trông như mesh bị vỡ). Component C++ `WeaponMesh` (tag `Weapon`, gắn theo `WeaponSocketName` = `hand_r`) vẫn còn nhưng để trống, nên `MeleeTraceComponent` quét cung fallback 50–180 cm trước mặt hero. Khi có kiếm: gán static mesh có socket `Trace_Start`/`Trace_End` vào `WeaponMesh` là trace tự dùng nó.
- **Animation kiếm thật:** template chỉ có đòn đấm/đá unarmed (`MM_Attack_01..03`). Muốn có kiếm: thêm pack miễn phí trên Fab (ví dụ Paragon Greystone/Kwang) vào library rồi retarget sang Manny.
- **Hướng mặt (user quyết định 2026-10-05):** hero luôn quay theo hướng camera, giống God of War (`Movement.bFaceCameraDirection` = true). Bấm S thì lùi lại, không quay người; blendspace 8 hướng của template phát clip đi/chạy lùi. S + Space phát `AM_Warlord_Dodge_B` mà không đổi hướng. Vì L/R chỉ là dash tiến (template không có clip né ngang), `Dodge.bSideClipsFaceInput` = true làm hero quay về phía input khi né ngang, rồi quay lại hướng camera. Khi có clip né ngang thật thì đặt flag này về false.
- **Montage:** Light 01/02/03 = `MM_Attack_01/02/03`. Heavy = `MM_ChargedAttack` từ 0.55 s (hit 0.45–0.67 s khi lao tới 150 cm; chỉ cancel bằng Dodge ở 0.95–1.25 s). Dodge F/L/R = `MM_Dash` 0–0.8 s nén còn 0.6 s, B = dash phát ngược. HitReact F/B = `MM_HitReact_Front_Med_01`/`Back_Med_01`. Death = `MM_Death_Front_01`, auto blend-out tắt. Asset montage cũ được giữ nguyên và đổi clip tại chỗ, nên tham chiếu trong DA không đổi. Chạy lại script không ghi đè cửa sổ đã chỉnh tay.
- **Chưa đạt yêu cầu thiết kế:** hình ảnh là unarmed thay cho kiếm khiên; hit react là clip tư thế cầm súng; L/R chưa có clip né ngang.

## Kiểm chứng đã có và giới hạn

- 2026-10-05, trước khi commit: editor build thành công, `Tools/run_tests.bat` **79/79**, editor exit 0 (`Saved/wip-tests.log`).
- Có scripted PIE cho combo, i-frame, hit/death/respawn và rendered PIE cho debugger/fallback sweep. Các fixture tạm thời trong PIE không có nghĩa saved hero đã được lắp ráp đầy đủ.
- Montage đã dùng clip Mannequin placeholder thay cho `Tutorial_Idle`; vẫn thiếu lần chơi thử có hình ảnh với input thật của user cho toàn bộ acceptance criteria. Không đánh dấu Review thành Done chỉ dựa trên test xanh.
- Lệnh chính: `Tools/build.bat`, `Tools/run_tests.bat`; kiểm tra PIE trên `L_CombatSandbox`. Bộ Functional Test đầy đủ thuộc T-CMB-15.
- Bằng chứng chi tiết, đường dẫn log và các bước Editor còn thiếu nằm trong [progress.md](../progress.md). Dùng trạng thái task và bằng chứng mới nhất làm nguồn quyết định; không coi snapshot này là xác nhận hoàn thành.
