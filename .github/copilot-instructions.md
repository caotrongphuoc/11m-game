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
