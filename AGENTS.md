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
- Tasks communicate **only via messages** (`task_post_*`), never direct cross-task calls
  or foreign-state access. The approved extern instances below do not change this rule.
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
- Coding rules: `docs/01-guide-coding-rules.md`
- Object ownership and message sequences: `docs/02-design-sequence-object.md`
- End-to-end runtime flow: `docs/03-design-sequence-runtime.md`

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

- Each object header declares an `em_game_<object>_t` state type and one `extern` instance, with exactly one definition in its owning `.cpp`. Match defines and owns the Scoreboard instance.
- The engineer has approved extern object instances. Only the owning task accesses its instance during gameplay; other tasks and screens still use messages and validated snapshots, not direct cross-object calls or foreign state reads/writes.
- Headers expose object state, task handlers, object-owned enums, constants, and message payload contracts. Keep state structures separate from message payload structures.
- Match coordinates rounds and forwards accepted input; Shooter owns kick selection; Ball owns movement; Keeper owns dive selection; Goal resolves Ball and Keeper reports; Display owns render snapshots.
- Tasks exchange commands, results, and view snapshots only through `task_post_*` messages.
- Validate incoming common-message lengths and values before using their payloads.
- Keep every common-message payload within `AK_COMMON_MSG_DATA_SIZE` (64 bytes). Do not add compile-time size assertions; preserve runtime payload validation.
- Follow `docs/02-design-sequence-object.md` and `docs/03-design-sequence-runtime.md` when changing gameplay message flow.

## Current runtime architecture

- `scr_game_penalty` owns the single periodic `EM_GAME_TIME_TICK` timer at 40 ms.
- Each tick posts `UPDATE` signals to Match, Shooter, Ball, and Keeper. Objects use local elapsed-time accumulators for their own animation intervals.
- Match owns round coordination and gameplay screen transitions after initial menu entry. Startup enters the menu through the framework. Game screens translate button input into Match signals.
- Shooter owns kick selection, Ball owns movement, Keeper owns dive selection, Goal resolves the two reports, and Display owns validated render snapshots.
- Extend this message-driven loop. Do not introduce direct object calls, shared mutable gameplay data, blocking delays, or a second gameplay timer.

## Zomwar alignment boundary

- Match Zomwar's feature-oriented file layout, `em_game_*` object naming, `scr_game_*` screen naming, visual source sections, screen rendering helpers, and display-owned shared tick pattern.
- Keep Eleven Meter directly under `application/sources/app/game_eleven_meter/`; do not add an intermediate `app/game/` directory.
- Zomwar is read-only reference code. Match its header layout and extern-instance style as approved by the engineer, but do not copy direct cross-object access/calls or screen-owned gameplay state. Message-driven runtime ownership is unchanged.
- When Zomwar and AK documentation disagree, follow AK documentation and record the safe divergence in `docs/01-guide-coding-rules.md`.

## Build and validation

- Build the application with `make -C application`. Tool paths may be overridden with `GCC_PATH` and `PROGRAMMER_PATH`.
- Before handing off firmware changes, run `make -C application clean`, `make -C application`, and `git diff --check`.
- Report Flash and RAM usage from the successful build.
- Hardware validation uses the UART console at 115200 8N1. Display validation should include an `lcd d` framebuffer dump when a board is available.
- Never claim on-device validation when only compilation was performed.

## Development tools lifecycle

Keep the `tools/` folder throughout game development, including its renderers, capture
helpers, and validation scripts. Remove it only after the engineer confirms the entire
game is finished and requests final cleanup. Completing an individual TODO, README
update, or demo is not permission to remove `tools/`.

## Locked Eleven Meter decisions

- Keep the physical kick mapping local to the penalty screen: left/Down shoots left,
  middle/Up shoots center, right/Mode shoots right. Do not swap BSP pins to achieve it.
- Preserve the original shooter bitmap arrays. Animation uses the standing bitmap
  with the approved two-pixel step; do not restore raised-leg frames without a request.
- Use `scr_game_full_time` / `EM_GAME_MATCH_STATE_FULL_TIME`, not RIP or Defeat, for the
  transition after either a win or a loss. Keep the two-second timeout and Mode skip.
- Keep all six console/app/system print and debug gates disabled in the play build.
  A diagnostic build is temporary and must be restored, not silently shipped.
- Do not reintroduce Zigbee, RF24, Modbus demo helpers, QR-code or Welcome screens,
  application size assertions, removed Getting Started documentation, or the Screen 05
  demo wrapper just to match an older reference tree.
- Display is not a gameplay object. Scoreboard is a Match-owned helper, not a task.
- Gameplay Demo stays directly below the banner and uses the unframed OLED recording.
  Other screen previews keep the approved banner-derived frames. Object-table previews
  remain unframed, centered on equal-size canvases.
- Raw captures, old firmware checksums, timestamps, and their historical `rip` phase
  labels are evidence: do not rewrite them to pretend they came from newer firmware.

# Reusing this pattern for another AK game

This section is a recipe for an **explicitly requested new game**, not permission to
rename or regenerate Eleven Meter. Keep current project decisions above in force.
When copying these instructions to a new repository, replace the project manifest,
object list, controls, paths, and project-specific decisions before implementing.

## 1. Establish the game manifest

Resolve the following from the request or make a clearly stated, low-risk assumption:

| Field | Eleven Meter example | New-game rule |
|---|---|---|
| Name and slug | Eleven Meter / `eleven_meter` | One stable name and filesystem slug. |
| C/C++ prefix | `em_game_` | Unique `<short>_game_` prefix across object symbols. |
| Signal prefix | `EM_GAME_` | Uppercase counterpart, with a named game signal base. |
| Feature directory | `app/game_eleven_meter/` | Use `app/game_<slug>/` directly, without another `game/` layer. |
| Coordinator | Match | One owner of session state, accepted input, and gameplay transitions. |
| Active objects | Shooter, Ball, Keeper, Goal | Give a task only to an independently event-driven object. |
| Passive helpers | Scoreboard, persistence helpers | Call locally from their owner, not as extra tasks. |
| Screens | Menu, Penalty, Full Time, Game Over | Use game-appropriate names and explicit transitions. |
| Controls | Down / Up / Mode | Define each screen's behavior and physical orientation. |
| Timing | One 40 ms shared tick | Choose one gameplay tick from animation and latency needs. |
| Finish condition | Five results, at least three goals wins | Specify win/loss, scoring, tie-breaks, retry, and home. |
| Persistence | BEST goals and difficulty | Specify record format, validation, defaults, and write policy. |

Use the AK base kit as the firmware foundation. For an empty workspace call
`start_ak_project` and execute its exact release/download/layout instructions. For an
existing project, inspect and extend its modules. Zomwar and Eleven Meter are read-only
design references, not source trees to copy wholesale into a different game.

## 2. Follow the implementation order

1. Read the manifest, existing source, `.clang-format`, and reference patterns. Fetch
   AK guardrails, the relevant guides, and exact API documentation before implementation.
2. Write a short ownership/state/transition plan. List pure signals, common payloads,
   accepted phases, setup/reset rules, and the single tick's posting order.
3. Register signal blocks in `app.h`, task IDs/handlers in `task_list.h/.cpp`, and sources
   in feature/screen `Makefile.mk` files. Keep IDs contiguous and table order identical.
4. Implement object headers and handlers, then the coordinator and owner-local helpers.
5. Implement each screen's render helpers and input translation, its header, and its
   declaration group in `screens.h`. Wire only the necessary Display routes/snapshots.
6. Add sprites, sound signals, validated persistence, and boundary/duplicate-input tests.
7. Clean-build and run host checks before approved hardware flashing. Capture a real
   menu → gameplay → result → home cycle when a kit and diagnostic access are available.
8. Finish README, object/runtime sequences, image provenance, and final review. Keep
   each agreed TODO reviewable and wait for its commit confirmation before the next.

Do not infer authorization to flash a different kit, erase saved data, enable a new
network, publish media, or remove development tools from a request to review the game.

## 3. Object header and implementation contract

- Use `<prefix><object>.h/.cpp`, a matching uppercase include guard, and the module's
  own header as the first include in its implementation.
- Header order: standard types → AK includes (`fsm.h`, `port.h`, `message.h`, `timer.h`)
  → application/debug/task declarations → required related contracts → commented
  defines → enums/payload/view types → object state type → extern instance → handlers.
- Keep sprite sizes, offsets, object bounds, animation counts, and tuning values in
  the owning header. Keep signal blocks and task timer constants in `app.h`.
- Define exactly one instance per extern declaration. Keep its initial values explicit
  and preserve a single owner. Define helper state in its coordinator when appropriate.
- Keep state structs distinct from wire payloads. Never send RNG state, pointers,
  guards, or a whole large object when the receiver only needs a small command/view.
- Use `Variable declaration`, `Private helpers`, and `Handle` implementation sections.
  Private helpers stay `static`; exporting object storage does not export helper calls.
- This project's approved extern style is a linkage/layout convention, not permission
  to share runtime access between tasks. For another project, preserve its ownership
  policy; default to private state unless the engineer approves extern instances.
- Use runtime message length/value/phase checks and host tests. Eleven Meter does not
  use `static_assert`; do not add it back as part of template cleanup.

## 4. Screen/header dependency contract

- Every screen header declares its own `view_dynamic_t`, `view_screen_t`, and handler;
  it includes enough dependencies to compile independently and owns screen constants.
- `screens.h` contains the common includes and named extern declaration groups for
  every screen, matching their own headers. Do not replace it with an include-only list.
- Keep object and screen headers out of `task_display.h` when they include Display's
  interface. Include concrete contracts explicitly in `task_display.cpp` and in screens
  that use their enums. Do not rely on accidental transitive includes or introduce cycles.
- Screens have `View` and `Handle` sections and small element-specific drawing helpers.
  Keep drawing local to Display and read validated snapshots, not foreign object state.
- The active gameplay screen owns one periodic tick on Display. Its entry sets up the
  objects and replaces any previous tick. The coordinator stops it before leaving play.
- Objects accumulate time locally. Posting order is not a synchronous update barrier.
  Startup, render-throttling, and transition one-shots are not extra gameplay loops.
- Handle pressed/held/released inputs according to the actual game's mechanics; do not
  copy unused movement or network events solely because they appear in a reference.

## 5. Documentation and media contract

- README order: badges/title → game banner → Gameplay Demo → Documentation →
  Introduction → Hardware → Game Description and Objects → How to Play / Mechanics →
  Basic Game Sequence Logic → Contact & Support.
- Preserve `docs/01-guide-coding-rules.md`, `02-design-sequence-object.md`, and
  `03-design-sequence-runtime.md` roles. Add a Getting Started or deployment guide only
  when that project calls for it; do not resurrect deliberately removed documents.
- Write standalone game-specific rules. Use real object/screen/signal names, not text
  telling the reader to "follow Zomwar". Explain intentional architectural differences.
- The object table lists gameplay objects/helpers, not Display. Show menu, gameplay,
  transition/result, and score screens in flow order, with consistent figure numbers.
- Use Mermaid for detailed sequences and a compact overview image in README. Check
  asynchronous message arrows, participants, branch balance, names, and local links.
- Reuse actual sprite bytes and existing renderer assets. Distinguish generated previews
  from real OLED captures and camera video. Never fabricate board evidence or edit raw
  recordings to claim a newer firmware test. Keep captured timing limitations explicit.
- Preserve original sprites unless redesign is requested. For new media, use consistent
  banner-derived styling and equal-size plain object previews without obscuring pixels.
- Record capture firmware identity and retain enough source data/scripts to reproduce
  generated assets. Verify GIF dimensions, frame count, timing, and unchanged OLED pixels.

## 6. Final review and handoff gate

- Inspect source/header pairs, signal/task/build wiring, initialization, tick lifetime,
  phase guards, payload bounds, result joining, retry/home, persistence, and audio.
- Test invalid lengths/values, duplicate/out-of-phase input, timeout paths, all difficulty
  levels, winner thresholds, and repeated matches as applicable. State what was tested
  on a host versus a device; compilation alone is not gameplay validation.
- Run `make -C application clean`, `make -C application`, formatting checks on changed
  C/C++, and `git diff --check`. Report Flash/RAM from the successful build.
- Eleven Meter's host regression commands are:

  ```bash
  PYTHONDONTWRITEBYTECODE=1 python3 tools/test_object_headers.py
  PYTHONDONTWRITEBYTECODE=1 python3 tools/test_screen_headers.py
  PYTHONDONTWRITEBYTECODE=1 python3 tools/test_penalty_buttons.py
  PYTHONDONTWRITEBYTECODE=1 python3 tools/test_full_time_screen.py
  PYTHONDONTWRITEBYTECODE=1 python3 tools/test_screen_cards.py
  ```

- Adapt tests to a new game's names and contracts, not its gameplay implementation.
  Test header independence/both include orders, exactly one extern definition, initial
  defaults, no foreign-state access, and button/tick routing. Check README/media links.
- With hardware available, follow `debug-uart-shell`, inspect `ver` / `fatal l`, and
  decode an `lcd d` dump when supported. Quiet log gates can suppress those replies:
  absence of diagnostic output does not prove a healthy match or an empty fatal record.
- Flash only with approval to a resolved device and application address. Use
  `make -C application flash dev=<verified-port>` for the kit's Type-C bootloader;
  verify updater checksum and boot evidence. Keep a record of the exact flashed binary.
- Do not enable debug output just to finish a review without acknowledging the build
  change and restoring the approved quiet image. Do not reset or erase BEST/FATAL data.
- If a final edit is comments/docs only, compare the generated binary to the flashed
  one before deciding a reflash is needed. Report remaining risks and validation limits.
- Provide a ready-to-copy commit command, do not stage or commit automatically, and
  preserve `tools/` until final cleanup is explicitly requested.

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
