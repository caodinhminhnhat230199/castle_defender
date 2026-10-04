# Hero Combat — WIP và handoff

Cập nhật: 2026-10-04 · Agent: Codex · Branch: `feat/01-hero-combat`.

Tài liệu này lưu danh sách công việc còn lại và lý do chưa hoàn tất để teammate tiếp tục. Đây là snapshot; trước khi làm hãy đối chiếu [tasks.md](tasks.md), [technical-plan.md](technical-plan.md), [spec.md](spec.md), [progress.md](../progress.md) và Git diff hiện tại.

## Workflow đã được thống nhất

- Mọi task Hero Combat dùng chung branch `feat/01-hero-combat`, chia thành nhiều commit nhỏ theo task. Không tạo branch riêng cho từng task và không commit feature trực tiếp vào `main`.
- Chỉ tạo PR vào `main` khi tất cả task của feature đã Done và có kết quả kiểm chứng được ghi lại. Vẫn tuân thủ gate pha và dependency; không làm trước task của pha chưa mở.
- Khi lưu handoff này, feature đang ở P0; chưa tạo PR hoặc push. Sáu commit Hero Combat cũ đã được giữ lại, cùng các commit checkpoint `b9f9737` (code/tests), `6960bf6` (asset/tools) và `a1ae690` (tài liệu/workflow).

## Tình trạng tổng thể

Theo [tasks.md](tasks.md), feature có **21 task: 5 Done, 4 Review, 12 Todo**. Như vậy còn **16 task chưa hoàn tất**.

Các task Done: **T-CMB-01, 02, 03, 04, 13**.

## 4 task đã có code và kiểm thử, nhưng còn thiếu nội dung hoặc tích hợp để đạt Done

| Task | Nội dung | Vì sao chưa hoàn tất |
|---|---|---|
| **T-CMB-05** | Chuỗi 3 đòn đánh nhẹ | Logic combo đã làm. Hero lưu trong project còn thiếu mesh, Animation Blueprint và vũ khí/socket trace; montage hiện dùng `Tutorial_Idle`. Cần animation đánh thật và kiểm tra input, đường quét vũ khí trong PIE có hình ảnh. |
| **T-CMB-07** | Né với khoảng bất tử | Logic hướng né, stamina và i-frame đã làm. Còn thiếu animation né có root motion để kiểm chứng khoảng di chuyển thực tế. Né khi lock-on sẽ tích hợp cùng T-CMB-10. |
| **T-CMB-11** | Phản ứng trúng đòn, chết và hồi sinh | Logic phản ứng, sự kiện chết và respawn đã làm. Còn thiếu animation phù hợp và kiểm chứng trình bày khi chơi; phát feedback chết thuộc UXF, dọn lock-on sẽ tích hợp khi T-CMB-10 có mặt. |
| **T-CMB-21** | Debugger combat và hiển thị trace | Debugger đã hoạt động. Còn thiếu kiểm chứng đường quét vũ khí được animation điều khiển, cùng coverage đầy đủ cho các hành động P0A chưa có như Heavy và rotation assist. |

Các thiếu sót này được ghi cụ thể trong [handoff và kết quả kiểm chứng](../progress.md).

## 8 task Todo thuộc pha P0 hiện tại

| Task | Nội dung | Vì sao chưa bắt đầu |
|---|---|---|
| **T-CMB-06** | Đánh mạnh, sát thương poise cao | Chờ **T-SYN-01**: hệ thống poise, trạng thái có thời hạn và poise break → Staggered. Task này vẫn Todo; phần Foundation hiện có chưa thay thế được nó. |
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
2. Hoàn thiện content cho **T-CMB-05/07/11**: mesh/ABP/vũ khí, montage Slot, socket `Trace_Start`/`Trace_End`, animation đánh/né/trúng đòn/chết phù hợp. Animation né cần root motion. Ghi nguồn/license của asset placeholder.
3. Chạy build, automation và PIE có hình ảnh để kiểm chứng input, chuyển động, hit trace, phản ứng/chết/respawn. Chỉ nâng từng task lên Done khi đủ acceptance criteria; cập nhật `tasks.md` và `progress.md`.
4. Xử lý chuỗi dependency **T-UXF-01 → T-SYN-01 → T-CMB-06 → T-CMB-20**; hoàn tất phần kiểm chứng còn thiếu của **T-CMB-21** và ghi checkpoint P0A.
5. Tiếp tục **T-CMB-08/10 → 09/14 → 15 → 16** khi dependency tương ứng đã Done. T-CMB-14 cần enemy provider; G0 cần các task ENM/SYN/UXF liên quan hoàn tất.
6. **T-CMB-12/17/18/19** tiếp tục chờ pha tương ứng mở. Feature này trải qua **P0, P2 và VS**, nên “xong tất cả task” còn bao gồm công việc ở những pha chưa mở.

## Kiểm chứng đã có và giới hạn

- Handoff trước đã ghi Editor/Development/Shipping build thành công và automation gate **74 test thành công, không có warning/failure/NotRun/InProcess**. Đây là kết quả đã ghi từ trước; không chạy lại Unreal chỉ để lưu tài liệu này.
- Có scripted PIE cho combo, i-frame, hit/death/respawn và rendered PIE cho debugger/fallback sweep. Các fixture tạm thời trong PIE không có nghĩa saved hero đã được lắp ráp đầy đủ.
- Montage hiện vẫn dùng `Tutorial_Idle`; chưa có đủ bằng chứng cho animation/vũ khí thực tế và toàn bộ acceptance criteria của combat. Không đánh dấu Review thành Done chỉ dựa trên test xanh.
- Lệnh chính: `Tools/build.bat`, `Tools/run_tests.bat`; kiểm tra PIE trên `L_CombatSandbox`. Bộ Functional Test đầy đủ thuộc T-CMB-15.
- Bằng chứng chi tiết, đường dẫn log và các bước Editor còn thiếu nằm trong [progress.md](../progress.md). Dùng trạng thái task và bằng chứng mới nhất làm nguồn quyết định; không coi snapshot này là xác nhận hoàn thành.
