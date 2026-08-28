# Eleven Meter coding rules

Eleven Meter follows the presentation and feature-oriented organization used by Zomwar while retaining the stricter ownership and message rules required by the current AK documentation.

## Source layout

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
└── screens/
    ├── scr_game_menu.h/.cpp
    ├── scr_game_penalty.h/.cpp
    ├── scr_game_rip.h/.cpp
    └── scr_game_over.h/.cpp
```

Unlike Zomwar, this project intentionally keeps its single game folder directly under `app/`; do not recreate an intermediate `app/game/` directory.

## Naming

| Item | Pattern | Example |
|---|---|---|
| Game object file | `em_game_<object>.h/.cpp` | `em_game_ball.cpp` |
| Screen file | `scr_game_<name>.h/.cpp` | `scr_game_penalty.cpp` |
| Game signal | `EM_GAME_<OBJECT>_<ACTION>` | `EM_GAME_BALL_KICK` |
| Task ID | `EM_GAME_<OBJECT>_ID` | `EM_GAME_BALL_ID` |
| Type | `em_game_<name>_t` | `em_game_ball_view_t` |
| Function | `em_game_<object>_<action>()` | `em_game_ball_handle()` |
| Object constant | `EM_GAME_<OBJECT>_<PROPERTY>` | `EM_GAME_BALL_START_X` |

## File organization

Use the same visual sections as Zomwar where they make a file easier to scan:

```cpp
/*****************************************************************************/
/* Private state - Ball */
/*****************************************************************************/
```

Headers contain constants, object-owned enums, payload contracts, display snapshots, and public handlers. Implementations contain private state, private helpers, and the handler. Screen implementations use `View` and `Handle` sections; large gameplay views should use object-specific display helpers.

## AK ownership rules

- Keep each task's mutable state `static` in its `.cpp` file.
- Do not expose mutable gameplay globals with `extern`.
- Do not call one gameplay task directly from another.
- Exchange commands, results, and snapshots through `task_post_*` messages.
- Validate common-message length and field ranges before use.
- Add a `static_assert` for every common-message payload.
- Keep handlers non-blocking; use the shared tick or a timer for deferred work.
- Do not log the 40 ms update path because UART output would distort timing.

These rules take precedence over older Zomwar patterns that expose object globals for direct rendering or collision checks.

## Formatting

The root `.clang-format` is authoritative: Allman braces, tab indentation, left-aligned pointers, no automatic line wrapping, and preserved include order.

```bash
clang-format -i application/sources/app/game_eleven_meter/*.h \
  application/sources/app/game_eleven_meter/*.cpp \
  application/sources/app/screens/scr_game_*.h \
  application/sources/app/screens/scr_game_*.cpp
```

Before committing firmware changes, run:

```bash
make -C application clean
make -C application
git diff --check
```
