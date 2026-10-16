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

Scoreboard is a stateless helper operating on Match-owned data, not a registered AK task.
Its direct helper calls from Match do not cross task ownership boundaries. Screen render
helpers may read Display's validated view because they execute inside Display's handler;
gameplay tasks must not use that view as shared state.

## Sprites and performance

Preserve the original shooter bitmap data. The current animation uses only the standing
bitmap and changes Shooter's position through view messages; the old kick/follow assets
are retained but not displayed. Any future artwork change requires an explicit request.
Keep the generated previews synchronized with screen changes using
`python3 tools/render_game_screens.py`; label previews separately from UART captures.

The OLED driver accepts pin-write callbacks supplied at startup from `io_cfg`. This avoids
repeated Arduino pin dispatch on each transmitted bit while keeping board IO in its existing
platform seam. Retain the fallback path for callers that do not supply callbacks.
Keep one 40 ms gameplay timer and fixed message pools. Do not mask a handler backlog by
adding timers or increasing pools without measuring the producer and consumer rates.
The normal play build disables console print/debug flags; use a clean diagnostic build
for timing or framebuffer inspection, then restore the quiet build.

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
