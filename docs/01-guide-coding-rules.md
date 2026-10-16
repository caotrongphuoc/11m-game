<h1 align="center">Coding rules and style guide</h1>

This guide defines Eleven Meter's naming, source layout, task ownership, formatting,
commit messages, and documentation conventions. Examples use the existing `em_game_*`
modules and `scr_game_*` screens. Apply these rules when extending the game; do not
reorganize working modules or change firmware behavior as part of a formatting update.

## Table of contents

- [I. Naming conventions](#i-naming-conventions)
  - [1. Folder](#1-folder)
  - [2. Source and header files](#2-source-and-header-files)
  - [3. Header guard](#3-header-guard)
  - [4. Macros and compile-time constants](#4-macros-and-compile-time-constants)
  - [5. Signals](#5-signals)
  - [6. Task ID](#6-task-id)
  - [7. Data types and typedef](#7-data-types-and-typedef)
  - [8. Functions](#8-functions)
  - [9. Variables](#9-variables)
  - [10. Ownership and message contracts](#10-ownership-and-message-contracts)
- [II. Code style (clang-format)](#ii-code-style-clang-format)
- [III. Installing clang-format](#iii-installing-clang-format)
- [IV. Running clang-format](#iv-running-clang-format)
- [V. Commit message convention](#v-commit-message-convention)
- [VI. Document file naming convention](#vi-document-file-naming-convention)
- [VII. References](#vii-references)

## I. Naming conventions

Use names that identify the owning module and can be searched without guessing abbreviations.

| Style | Use | Examples |
|---|---|---|
| `lower_snake_case` | Folders, source files, functions, variables, types | `game_eleven_meter`, `em_game_ball_handle`, `s_target_x` |
| `UPPER_SNAKE_CASE` | Constants, signals, task IDs, header guards | `EM_GAME_BALL_START_X`, `EM_GAME_BALL_KICK`, `EM_GAME_BALL_ID` |
| `kebab-case` | Markdown document names | `01-guide-coding-rules.md` |

### 1. Folder

Group game code by feature, keeping object implementations together and screens separate:

```text
application/sources/app/
├── game_eleven_meter/
│   ├── Makefile.mk
│   ├── em_game_match.h/.cpp
│   ├── em_game_shooter.h/.cpp
│   ├── em_game_ball.h/.cpp
│   ├── em_game_keeper.h/.cpp
│   ├── em_game_goal.h/.cpp
│   └── em_game_scoreboard.h/.cpp
├── screens/
│   ├── Makefile.mk
│   ├── scr_game_menu.h/.cpp
│   ├── scr_game_penalty.h/.cpp
│   ├── scr_game_full_time.h/.cpp
│   └── scr_game_over.h/.cpp
├── app.h
└── task_list.h/.cpp
```

Keep `game_eleven_meter/` directly under `app/`; do not add an intermediate `app/game/`
directory. Register new implementation files in the appropriate `Makefile.mk`.

Game features belong in `app/`; hardware-independent drivers belong in `driver/`.
GPIO operations and pin assignments belong in `platform/stm32l/io_cfg.c` and `io_cfg.h`,
with callbacks injected into drivers at startup. Kernel, bootloader, system services,
common framework code, network stacks, and vendored libraries are not game feature code.

The game application does not register Zigbee or RF24 tasks or Zigbee polling, and
does not include the Modbus demo helpers, QR-code screen, or Welcome screen. Their
unused application code has been removed. Original network stacks and vendored
libraries remain read-only, with the network feature flags disabled. The application
Makefile also excludes the unused QR-code library source from compilation.
UART console and link support remain available. Legacy remote gateway IDs in
`task_list_if.h` are protocol slots, not local task registrations; keep their values
stable when pruning local tasks.

### 2. Source and header files

Use `.h` for contracts and `.cpp` for implementations. Each module keeps its prefix:

| Module | Pattern | Example |
|---|---|---|
| Gameplay object or helper | `em_game_<object>.h/.cpp` | `em_game_ball.cpp` |
| Screen | `scr_game_<name>.h/.cpp` | `scr_game_penalty.cpp` |
| Screen artwork | Existing shared bitmap files | `screens_bitmap.h/.cpp` |

Keep task state and private helpers in the implementation. Headers expose the handler,
object-owned enums, constants, payload contracts, and view snapshot types. Helper modules
such as Scoreboard may expose operations on data owned by their caller.

Each screen header declares its own `view_dynamic_t`, `view_screen_t`, and
`scr_*_handle(ak_msg_t* msg)` entry point, includes its required dependencies, and
defines its screen-specific geometry constants. `screens.h` contains the common
includes and a named declaration group for each screen: its dynamic view, screen
object, and handler. Keep these declarations consistent with the owning screen
headers. It does not include the individual screen headers, and screen headers do
not include the registry. Keep `task_display.h` independent of the screen headers
to avoid circular includes. Private rendering helpers remain local to each `.cpp`.
Retain message-only gameplay communication; screen declarations do not expose
mutable gameplay task state.

Run `python3 tools/test_screen_headers.py` to compile each screen header independently
and with the shared registry in both include orders using the application's ARM compiler flags. Keep AK headers
in their supported include order (`fsm.h` before `message.h` in screen headers).

### 3. Header guard

Match the uppercased file name, including the project prefix:

```cpp
#ifndef __EM_GAME_BALL_H__
#define __EM_GAME_BALL_H__

// Ball contracts and handler declaration.

#endif //__EM_GAME_BALL_H__
```

Screen guards use `__SCR_GAME_<NAME>_H__`, for example `__SCR_GAME_PENALTY_H__`.
Preserve these established guards when extending existing files.

### 4. Macros and compile-time constants

Use `EM_GAME_<OBJECT>_<PROPERTY>` for object constants, `SCR_GAME_<SCREEN>_*` for
screen geometry, and `EM_GAME_*` for game-wide values. Put numeric macro values in
parentheses. Choose a named constant instead of repeating a coordinate or interval.

| Category | Existing example | Location |
|---|---|---|
| Start position | `EM_GAME_BALL_START_X` | `em_game_ball.h` |
| Movement length | `EM_GAME_BALL_STEP_COUNT` | `em_game_ball.h` |
| Match length | `EM_GAME_SCOREBOARD_TOTAL_KICKS` | `em_game_scoreboard.h` |
| Sprite geometry | `SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH` | `scr_game_penalty.cpp` |
| Animation interval | `EM_GAME_SHOOTER_ANIM_TICK_INTERVAL` | `app.h` |
| Shared gameplay tick | `EM_GAME_TIME_TICK_INTERVAL` | `app.h` |

```cpp
#define EM_GAME_BALL_START_X (64)
#define EM_GAME_BALL_STEP_COUNT (8)
```

### 5. Signals

Signals are the public message contract. Use the full `EM_GAME_<OBJECT>_<ACTION>` name
in code, documentation, and sequence diagrams, including `SETUP`, `RESET`, and `UPDATE`.
Keep each task's signal block in `app.h`, anchored to `EM_GAME_DEFINE_SIG`.
Application-defined signals must not use the reserved system signal range.

The Ball signal block, for example, defines:

```text
EM_GAME_BALL_SETUP = EM_GAME_DEFINE_SIG
EM_GAME_BALL_KICK
EM_GAME_BALL_UPDATE
EM_GAME_BALL_RESET
```

Interpret signals in the destination task's context; do not invent globally unique
numeric values or change signal numbering to simplify a diagram.

### 6. Task ID

Declare IDs in `task_list.h` as `EM_GAME_<OBJECT>_ID`. Keep their increasing order
consistent with `app_task_table` in `task_list.cpp` and register the matching handler.

| Task ID | Handler | Current priority |
|---|---|---|
| `EM_GAME_MATCH_ID` | `em_game_match_handle` | `TASK_PRI_LEVEL_4` |
| `EM_GAME_SHOOTER_ID` | `em_game_shooter_handle` | `TASK_PRI_LEVEL_4` |
| `EM_GAME_KEEPER_ID` | `em_game_keeper_handle` | `TASK_PRI_LEVEL_4` |
| `EM_GAME_BALL_ID` | `em_game_ball_handle` | `TASK_PRI_LEVEL_4` |
| `EM_GAME_GOAL_ID` | `em_game_goal_handle` | `TASK_PRI_LEVEL_4` |

Application tasks use priorities 1–7; priority 0 is reserved. Scoreboard is a helper,
not a registered task. Display is the framework rendering task, not a gameplay object.

### 7. Data types and typedef

Use `em_game_<object>_<role>_t`, with an anonymous struct or enum and a `_t` suffix.
Include the role when distinguishing a command payload from a render snapshot.

```cpp
typedef struct
{
	uint8_t target;
} em_game_ball_kick_t;
```

Examples include `em_game_ball_view_t`, `em_game_keeper_difficulty_msg_t`, and
`em_game_scoreboard_t`. Keep fields fixed-size and document valid enum values in the
owning header. A message contract describes data, not access to another task's state.

### 8. Functions

Use `lower_snake_case` and the module prefix:

```cpp
void em_game_ball_handle(ak_msg_t* msg);
bool em_game_scoreboard_is_complete(em_game_scoreboard_t* sb);
```

Make implementation-only helpers `static`, for example `em_game_ball_publish_view`.
Screen callbacks use the existing `view_scr_game_*` names; drawing helpers use
`scr_game_<screen>_<element>_display`. The `task_*` prefix is reserved for registered
AK task handlers, not ordinary helpers, screens, or drivers.

### 9. Variables

- Keep task-owned mutable state `static` in its `.cpp`, with the existing `s_` prefix:
  `s_ball`, `s_target_x`, `s_state_elapsed_ms`.
- Do not export mutable gameplay data through `extern` declarations or getters.
- Use descriptive local names such as `target_x`, `difficulty_msg`, and `player_score`.
  Short loop counters are acceptable within small loops.
- Keep state with its owner. A screen must not become a second owner of the round,
  countdown, ball position, keeper choice, or score.

### 10. Ownership and message contracts

| Owner | Responsibility |
|---|---|
| Match | Round state, accepted input, scoreboard, BEST, and game screen transitions |
| Shooter | Kick selection, accuracy, standing-sprite step, and Ball/Keeper commands |
| Ball | Shot movement and arrival report |
| Keeper | Dive choice, animation, and ready report |
| Goal | Resolve validated Ball and Keeper reports into one kick result |
| Display | Validated render snapshots and screen drawing |

Gameplay tasks exchange commands, results, and snapshots only through AK messages.
Do not replace a message with a cross-task function call or shared mutable data.
Before using a common-message payload, check its length, field ranges, and whether
the current phase accepts it. Keep each payload within `AK_COMMON_MSG_DATA_SIZE`
(64 bytes); the application does not use compile-time size assertions. Messages support at most seven references;
pools have fixed capacities.

Scoreboard operations are direct helper calls on Match-owned data, not cross-task
communication. Screen drawing helpers may read Display's validated view because they
execute in Display's context; gameplay tasks may not use that view as shared state.

Handlers must finish without blocking. The penalty screen owns the single 40 ms
`EM_GAME_TIME_TICK` timer on Display, which posts updates to Match, Shooter, Ball, and
Keeper. Use local elapsed-time accumulators for animation intervals; do not introduce a
second gameplay timer, busy-wait, or delay. Match stops the shared tick when the match ends.

Preserve original sprite bytes unless an artwork change is explicitly requested. The
shooter currently uses the standing bitmap and a two-pixel step; retained kick/follow
assets are not part of its animation. Keep rendering bounded and avoid console logging
on the update path. Diagnose producer/consumer rates before changing message pool sizes.

## II. Code style (clang-format)

The root [`.clang-format`](../.clang-format) is authoritative. Do not maintain an
alternative style inside a module or reformat unrelated files during a focused change.

| Setting | Required result |
|---|---|
| `UseTab: ForIndentation`, `IndentWidth: 4`, `TabWidth: 4` | Tabs for indentation; four-column tab width |
| `BreakBeforeBraces: Allman` | Opening braces on their own lines |
| `ColumnLimit: 0` | No automatic line wrapping |
| Short-statement options disabled | Expand short functions, blocks, cases, and conditionals |
| `PointerAlignment: Left` | `int* p` |
| `SpaceBeforeParens: ControlStatements` | `if (x)`, but `function(x)` |
| `IndentCaseLabels: false` | Case labels align with the switch keyword |
| `SortIncludes: false` | Preserve include order |

Use visual section comments to separate private state, private helpers, and handlers:

```cpp
/*****************************************************************************/
/* Private state - Ball */
/*****************************************************************************/
```

Screen implementations separate `View` and `Handle` sections. Split large views into
element-specific drawing helpers rather than mixing drawing and gameplay state changes.

## III. Installing clang-format

On Ubuntu/Debian, install the formatter if it is not already available:

```bash
sudo apt update
sudo apt install clang-format
clang-format --version
```

Use a consistent formatter version within a change and review the diff. The checked-in
configuration determines the style; an editor's built-in default is not a replacement.

## IV. Running clang-format

### 1. Command line

From the repository root, format only the files changed by the current task:

```bash
clang-format -i application/sources/app/game_eleven_meter/em_game_ball.cpp
```

To check that same file without changing it:

```bash
clang-format --dry-run --Werror application/sources/app/game_eleven_meter/em_game_ball.cpp
```

Do not run a repository-wide formatter across framework, bootloader, or vendored code.
Before handing off firmware changes, clean-build and check whitespace:

```bash
make -C application clean
make -C application
git diff --check
```

Report build Flash/RAM usage. Keep normal play logging quiet; use a temporary diagnostic
build for UART inspection, then restore the quiet image. A successful build is not proof
of on-device behavior. Keep capture provenance and distinguish generated previews from
actual framebuffer recordings.

### 2. Editor integration

Configure the C/C++ formatter to use the repository's `.clang-format`, then use
**Format Document** or **Format on Save**. Review the diff before committing, especially
when changing editor or formatter versions. Editor configuration must not introduce a
different brace style, include sorting, or line-wrap limit.

## V. Commit message convention

Use one logical topic per commit:

```text
[ACTION] short description
```

| Action | Purpose |
|---|---|
| `ADD` | Add a feature, module, asset, or file |
| `UPDATE` | Refactor or adjust existing behavior or organization |
| `FIX` | Correct a specific defect |
| `REMOVE` | Remove obsolete code, assets, or files |
| `DOC` | Change documentation or documentation previews |
| `MERGE` | Merge branches |

Keep the action uppercase and the description lowercase, imperative, and without a
trailing period. Keep the complete command line under roughly 80 characters. Name the
affected module or signal when useful; avoid vague descriptions such as `fix bug`.

```bash
git commit -m "[FIX] map penalty kicks to physical button positions"
```

Review the diff and stage only the intended files. After each agreed TODO, provide its
ready-to-copy commit command and wait for the engineer's confirmation before starting
the next TODO. Do not combine unrelated work or push changes without authorization.

## VI. Document file naming convention

Guide and design documents use `<NN>-<category>-<topic>.md`: a two-digit reading-order
number, a category such as `guide` or `design`, and a lowercase hyphenated topic.
Preserve the existing names and numbering when updating a document:

```text
docs/
├── 01-guide-coding-rules.md
├── 02-design-sequence-object.md
├── 03-design-sequence-runtime.md
└── 04-ak-mcp-deployment.md
```

The MCP deployment guide retains its established filename. Do not recreate removed
documents or renumber the remaining files merely to close a gap. Update inbound links
when renaming or removing a document.

Keep images under `resources/images/`, UART source records under `resources/captures/`,
and demo video assets under `resources/videos/`. Use the main README for the basic game
sequence and per-object/runtime guides for detailed Mermaid diagrams. Write exact
signal names and keep diagrams consistent with their task handlers.

Retain `tools/` and its capture, rendering, and test scripts throughout development.
Remove it only after the engineer confirms the whole game is complete and requests
final cleanup. Finishing a single TODO or README update is not that confirmation.

## VII. References

- [Project contribution instructions](../AGENTS.md)
- [Formatting configuration](../.clang-format)
- [Signal definitions](../application/sources/app/app.h)
- [Task registration](../application/sources/app/task_list.cpp)
- [Game Object Sequences](02-design-sequence-object.md)
- [Runtime Signal Processing](03-design-sequence-runtime.md)
- [AK documentation MCP deployment](04-ak-mcp-deployment.md)
- [Framebuffer capture records](../resources/captures/README.md)
