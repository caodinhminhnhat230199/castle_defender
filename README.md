# Game (UE5)

## Windows build and verification

The project is `CastleDefender.uproject`, pinned to UE 5.8. Install that engine, the Windows C++ toolchain and Git LFS; run `git lfs pull` after cloning.

```powershell
Tools/build.bat
Tools/run_tests.bat
Tools/create_foundation_assets.bat
Tools/package.bat
```

The launchers match the existing test launcher and use execution-policy bypass only for their child PowerShell process. They do not change machine or user policy. Set `UE_ROOT` only if launcher discovery cannot locate the pinned engine. Runner regression checks: `powershell -NoProfile -ExecutionPolicy Bypass -File Tools/test_test_report.ps1`.

Dự án game Unreal Engine 5, phát triển cùng Claude Code. Repo đã có sẵn bộ skill trong `.claude/skills/` và cấu hình plugin trong `.claude/settings.json`.

## Setup sau khi clone

### 1. Cài công cụ

- [Claude Code](https://claude.com/claude-code): CLI hoặc desktop app.
- `git` và `unzip` (macOS có sẵn).
- Tuỳ chọn: Node.js, chỉ cần khi muốn cài CodeGraph bằng `npm`.

### 2. Mở project trong Claude Code

```bash
cd <thư-mục-project>
claude
```

Khi được hỏi:

- **Trust this folder?** Chọn Yes.
- **Install plugins** `caveman`, `ponytail`: chọn Install. Hai plugin này đã được khai báo trong `.claude/settings.json`.

### 3. Chạy file init

Gõ vào Claude Code:

```
execute project_init.md
```

Agent sẽ tự làm các việc sau:

1. Kiểm tra và cài skill còn thiếu vào `.claude/skills/`. Skill đã có thì bỏ qua, không ghi đè.
2. Cài plugin `caveman` và `ponytail` nếu bước 2 chưa cài.
3. Cài **CodeGraph**: CLI và MCP server cài global, index cài riêng cho từng project. Agent sẽ hỏi trước khi tải installer và hỏi về telemetry (mặc định tắt).
4. Chạy bước kiểm tra. Kết quả phải ra 11 dòng `PASS`.

Chi tiết từng bước xem trong [project_init.md](project_init.md).

### 4. Mở session mới

Thoát và chạy lại `claude` để nạp skill và MCP server mới.

### 5. Tạo index CodeGraph

Chỉ chạy khi project đã có code C++ trong `Source/`. Mỗi máy chạy một lần:

```bash
codegraph init --yes
```

Index lưu trong `.codegraph/`. Không commit thư mục này.

## Skill có sẵn

| Skill | Dùng khi |
|---|---|
| `game-development-workflow` | Lên yêu cầu, spec, kế hoạch, prototype, vertical slice, playtest, QA, release |
| `ue5-project-architecture` | Kiến trúc UE5: module/plugin, Gameplay Framework, C++/Blueprint, GAS, AI, UI, save |
| `idea-refine` | Brainstorm, mài ý tưởng và cơ chế chơi |
| `source-driven-development` | Đối chiếu với tài liệu chính thức của UE5 |
| `debugging-and-error-recovery` | Tìm nguyên nhân gốc của lỗi build, crash, bug |
| `code-review-and-quality` | Review code trước khi merge |
| `documentation-and-adrs` | Ghi lại các quyết định kiến trúc |
| `git-workflow-and-versioning` | Branch, commit, PR, release |
| `planning-with-files` | Lưu kế hoạch ra file để làm tiếp qua nhiều session |
| `stop-slop` | Viết tài liệu, design doc không có văn phong AI |

Gọi skill bằng `/<tên-skill>`, hoặc mô tả việc cần làm để Claude tự chọn skill phù hợp.

## Lưu ý

- **Quy tắc cho coding agent:** mọi agent (Claude Code, Codex, Cursor, Copilot, …) làm theo [AGENTS.md](AGENTS.md). Claude Code đọc file này qua [CLAUDE.md](CLAUDE.md). Mỗi phiên làm việc ghi nhật ký vào [ai/game/progress.md](ai/game/progress.md).
- **Chỉ thêm hoặc bớt skill trong `.claude/skills/`.** Không sửa skill global trong `~/.claude`. Khi thay đổi skill, nhớ cập nhật `project_init.md` và bảng ở trên.
- **Không commit thư mục build của UE5:** `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`. Cũng không commit `.codegraph/`.
- **Thiếu skill `game-development-workflow` hoặc `ue5-project-architecture`?** Xin file zip từ lead, đặt vào thư mục gốc của project, rồi chạy lại `execute project_init.md`.
