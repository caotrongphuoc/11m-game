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

# Eleven Meter project instructions

## Project context

This is a penalty shootout game (11-meter penalty kick) built on top of a blank **AK Embedded Base Kit** for STM32L151. Prefix for this game: `em_game_*` (Eleven Meter).

### Base source
- Upstream: https://github.com/the-ak-foundation/ak-base-kit-stm32l151
- The base kit ships as an empty template: only `application/`, `boot/`, `hardware/`.
- This project has already created and wired its game objects under `application/sources/app/game_eleven_meter/` and its screens under `application/sources/app/screens/`. Extend the existing modules; do not scaffold them again.

### Reference (READ-ONLY — for rules and patterns, do NOT copy code)
- Sample project: https://github.com/caotrongphuoc/zomwar-game
  - Study `application/sources/app/game/game_zomwar/` for object file layout
  - Study `application/sources/app/screens/` for screen file layout
  - Study `application/sources/app/app.h` and `task_list.h/cpp` for signal & task registration
  - Coding rules reference: https://github.com/caotrongphuoc/zomwar-game/blob/main/docs/02-guide-coding-rules.md
- Coding rules: `docs/02-guide-coding-rules.md`
- Object ownership and message sequences: `docs/03-design-sequence-object.md`
- End-to-end runtime flow: `docs/04-design-sequence-runtime.md`

**Important**: study the patterns and re-implement with `em_game_*` prefix. Do not blindly copy zomwar sources into this repo.

## Naming for this project (em_game_*)
- Game folder: `application/sources/app/game_eleven_meter/`
- Screens folder: `application/sources/app/screens/`
- Object files: `em_game_<object>.h/cpp` (e.g. `em_game_ball.h`)
- Screen files: `scr_game_<name>.h/cpp` (follow zomwar pattern, e.g. `scr_game_penalty.cpp`)
- Header guards: `__EM_GAME_<OBJECT>_H__` for game objects, `__SCR_GAME_<NAME>_H__` for screens
- Signals: `EM_GAME_<OBJECT>_<ACTION>` anchored to `EM_GAME_DEFINE_SIG`
- Task IDs: `EM_GAME_<NAME>_ID` with handler `em_game_<name>_handle`
- Typedefs: `em_game_<object>_t`
- Functions: `em_game_<object>_<action>()`
- Project-level macros: `EM_GAME_*`

## Object ownership and communication

- Each gameplay task owns its mutable state as `static` data in its `.cpp` file.
- Do not expose mutable task state with `extern`, getters, or direct cross-task function calls.
- Headers expose task handlers, object-owned enums, and message payload contracts only.
- Match coordinates rounds and forwards accepted input; Shooter owns kick selection; Ball owns movement; Keeper owns dive selection; Goal resolves Ball and Keeper reports; Display owns render snapshots.
- Tasks exchange commands, results, and view snapshots only through `task_post_*` messages.
- Validate incoming common-message lengths and values before using their payloads.
- Protect every common-message payload with a `static_assert` against `AK_COMMON_MSG_DATA_SIZE`.
- Follow `docs/03-design-sequence-object.md` and `docs/04-design-sequence-runtime.md` when changing gameplay message flow.

## Current runtime architecture

- `scr_game_penalty` owns the single periodic `EM_GAME_TIME_TICK` timer at 40 ms.
- Each tick posts `UPDATE` signals to Match, Shooter, Ball, and Keeper. Objects use local elapsed-time accumulators for their own animation intervals.
- Match owns round coordination and every game screen transition. Screens only translate button input into Match signals.
- Shooter owns kick selection, Ball owns movement, Keeper owns dive selection, Goal resolves the two reports, and Display owns validated render snapshots.
- Extend this message-driven loop. Do not introduce direct object calls, shared mutable gameplay data, blocking delays, or a second gameplay timer.

## Zomwar alignment boundary

- Match Zomwar's feature-oriented file layout, `em_game_*` object naming, `scr_game_*` screen naming, visual source sections, screen rendering helpers, and display-owned shared tick pattern.
- Keep Eleven Meter directly under `application/sources/app/game_eleven_meter/`; do not add an intermediate `app/game/` directory.
- Zomwar is read-only reference code. Do not copy its mutable `extern` object globals, direct cross-object calls, or screen-owned gameplay state because those patterns conflict with current AK ownership rules.
- When Zomwar and AK documentation disagree, follow AK documentation and record the safe divergence in `docs/02-guide-coding-rules.md`.

## Build and validation

- Build the application with `make -C application`. Tool paths may be overridden with `GCC_PATH` and `PROGRAMMER_PATH`.
- Before handing off firmware changes, run `make -C application clean`, `make -C application`, and `git diff --check`.
- Report Flash and RAM usage from the successful build.
- Hardware validation uses the UART console at 115200 8N1. Display validation should include an `lcd d` framebuffer dump when a board is available.
- Never claim on-device validation when only compilation was performed.

## Coding style
Use the root `.clang-format` as the source of truth: Allman braces, tab indent 4, `int* p`, `if (x)`, no auto line-wrap, no sort includes. Run `clang-format -i` before committing.

## Commit message workflow

After completing ANY logical unit of work, always provide a ready-to-copy
commit command in this exact format:

    git commit -m "[ACTION] short description"

Rules:
- ACTION tag in UPPERCASE, chosen from: ADD / UPDATE / FIX / REMOVE / DOC / MERGE
- Description in lowercase, imperative mood (add, fix, rename, wire...)
- No trailing period
- Keep the whole line under ~80 characters
- Name the specific module/file/signal when relevant

Present each command in a fenced bash code block so it can be copied directly.

If a chunk of work naturally splits into multiple commits (e.g. signals in
app.h vs task registration vs stubs), provide the commits in ORDER, each as
its own code block. Never merge unrelated changes into one commit —
one [ACTION] = one topic.

When work follows an agreed TODO list, complete one TODO, provide one commit command for that TODO, and wait for the engineer to confirm the commit before continuing. Split it only when the engineer explicitly requests multiple commits.

Example output after finishing a phase:

```bash
    git commit -m "[ADD] em_game signal blocks and timer intervals"
```

```bash
    git commit -m "[ADD] register 5 em_game_* tasks in task list"
```

```bash
    git commit -m "[ADD] em_game task stubs and makefile chain"
```
