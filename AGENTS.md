<!--
Copy this file to .github/copilot-instructions.md in your AK firmware project.
It "steers" the AI agent to always consult the AK MCP docs server before writing
firmware, so generated code follows kernel conventions and stays out of the core.
(Cursor: copy the same text into .cursor/rules/ak.md. Claude Code: into CLAUDE.md.
Codex: into AGENTS.md.)
-->

# AK firmware — agent instructions

This project is firmware for the **AK (Active Kernel)** event-driven MCU framework.

You have access to the **`ak-docs` MCP server**. Use it as the source of truth — do
not guess AK APIs from memory.

**Starting a new project?** If this is a fresh/empty workspace (no `application/` yet),
call **`start_ak_project`** first — it downloads the latest base-kit release and gives you
the commands to lay it out. Run them, then customize per the steps below.

Before writing or changing any firmware:

1. Call **`get_ak_guardrails`** first. Never modify `application/sources/ak/` (kernel),
   `boot/`, `application/sources/networks/`, or `application/sources/common/`.
   Build features in `application/sources/app/` and `application/sources/driver/` only.
2. For a new task/driver/screen, call **`get_ak_guide`** (`create-task`,
   `create-driver`, `create-screen`, `use-timer`, `isr-bridge`, `tune-pools`) and follow
   the steps and skeleton exactly.
3. For any kernel function/macro, call **`get_ak_api`** to get the exact signature and
   arguments. Use **`search_ak_docs`** when you don't know the symbol name.

**Debugging a running board?** All debugging is over the UART console (115200 8N1):
call **`get_ak_guide("debug-uart-shell")`** first. Capture output non-interactively with
`python ak-console.py --port <P> --cmd "ver" --cmd "fatal l"` (or `--watch 15` for live
logs), then paste the raw text into **`analyze_ak_log`** and follow its Next steps.
For display issues, capture `--cmd "lcd d"` and paste the dump into **`decode_ak_lcd`**
to see the OLED contents.
Run only read-only shell commands on your own; destructive ones (`reboot`, `fatal t/!/@/r`,
`ram r`, `eps r`, `flash i`, `boot r/t`, `fwu`, `dbg s`) need the engineer's explicit OK.

Hard rules (also returned by `get_ak_guardrails`):

- Handlers must be **non-blocking** — no `delay()`, no busy-wait. Use a timer that posts a
  signal instead.
- Tasks communicate **only via messages** (`task_post_*`), never direct calls or shared
  globals.
- User signals start at `AK_USER_DEFINE_SIG` (10); task priorities are `LEVEL_1..7`
  (0 is reserved).
- Common message payload ≤ 64 bytes; max 7 references per message; pools are fixed size.

## Coding rules
This is a penalty shootout game project (11-meter penalty kick), using prefix `em_game_*` (Eleven Meter). ALWAYS follow:
- `docs/02-guide-coding-rules.md` (detailed rules adopted from zomwar-game)
- Reference sample repo: https://github.com/caotrongphuoc/zomwar-game (especially `application/sources/app/game/game_zomwar/` and `application/sources/app/screens/`)

Naming conventions for this project:
- Game folder: `application/sources/app/game/game_eleven_meter/`
- Object files: `em_game_<object>.h/cpp` (e.g. `em_game_ball.h`, `em_game_keeper.cpp`)
- Screen files: `scr_em_<name>.h/cpp` in `application/sources/app/screens/`
- Header guards: `__EM_GAME_<OBJECT>_H__`
- Signals: `EM_GAME_<OBJECT>_<ACTION>` anchored to `EM_GAME_DEFINE_SIG`
- Task IDs: `EM_GAME_<NAME>_ID` with handler `em_game_<name>_handle`
- Typedefs: `em_game_<object>_t`
- Functions: `em_game_<object>_<action>()`
- Project-level macros: `EM_GAME_*`

File structure, coding style (Allman braces, tab 4, `int* p`, `if (x)`), and commit messages — ALL must follow the coding rules document above.