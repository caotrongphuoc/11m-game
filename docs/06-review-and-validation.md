# Eleven Meter review and validation

Reviewed on **2026-10-16**, against firmware commit `33ac918`
(`simplify shooter animation without leg swing`). This documentation update does not change
firmware, compile flags, or the board image.

## Review scope and result

Reviewed Match, Shooter, Ball, Keeper, Goal, Scoreboard, game screens, Display snapshot
validation, EEPROM score storage, OLED callback wiring, and build/log configuration.
No new blocking gameplay defect was identified in the reviewed paths.

- Match accepts kicks only during selection and results only during reveal. Scoreboard
  enforces one result per round and exactly five accepted results before completion.
- Goal validates report lengths and values, accepts one report per sender, and resolves
  once both arrive. Wide balls are misses; otherwise a matching keeper zone is a save.
- Mutable gameplay state stays within its owner. Payload contracts have size assertions;
  received common-message payloads are checked before use. Scoreboard is a Match helper,
  not a separate task sharing data.
- The single 40 ms gameplay tick drives local animation counters. No blocking delay or
  busy-wait was found in the gameplay handlers. Match stops the tick before RIP/Game Over.
- Shooter retains the original standing bitmap and uses only a two-pixel step. The complete
  bitmap source matches the pre-artwork-change version at `88568b3`.
- OLED startup supplies the existing board pin callbacks to reduce per-bit dispatch work.
  Message pools remain fixed at 32 pure, 16 common, 8 dynamic messages, and 16 timers.
  The six console print/debug flags and kernel UART task tracing are disabled for play.
- BEST stores goals plus difficulty, prefers harder difficulty on equal goals, and validates
  the saved record's magic/checksum and field ranges when loading.

## Build and flash evidence

The firmware commit completed `make -C application clean`, a successful application build,
and `git diff --check`. The documentation review also checked `make -C application info`.

| Item | Recorded result |
|---|---|
| MCU | STM32L151CBT6, SSD1309 OLED configuration |
| Application address | `0x08003000` |
| Application Flash | 51,216 / 118,784 bytes (43.12%) |
| RAM reported by build | 13,636 / 16,384 bytes (83.23%) |
| Binary file length | 51,220 bytes |
| UART update | Completed through Type-C / CH340 at `/dev/ttyUSB0` |
| Updater checksum | `0xC0B0`, verified OK |

These are build and updater measurements, not runtime stack/heap high-water marks or a
claim that all remaining RAM is available for new features.

## Engineer-confirmed board checks

The engineer confirmed the following checks as OK on 2026-10-16 after the latest animation
update. These are manual confirmations, not an automated test log:

| Check | Result |
|---|---|
| Complete five attempts on Easy, Normal, and Hard | Confirmed OK |
| Selection timeout and automatic center kick | Confirmed OK |
| Retry and Home from Game Over | Confirmed OK |
| BEST persists after power-off | Confirmed OK |
| Repeated play without recurrence of the earlier MF31 failure | Confirmed OK |
| Standing-sprite animation without raised-leg frames | Accepted by engineer |

The number of matches and test duration were not recorded. This is not a quantified soak
test. The final quiet image does not print `fatal l` or `lcd d` replies, so no new fatal-counter
sweep or final-image framebuffer dump was claimed in that review. A subsequent documentation
update captured Figure 4 on a temporary diagnostic build; see its [raw dump and capture notes](../resources/captures/README.md).
The other screen images remain generated previews. To collect diagnostic evidence later, follow
[Getting Started](01-guide-getting-started.md#inspect-the-board).

## Remaining limits

- EEPROM write failure and power interruption during a write were not fault-injected.
  `save_score_record()` returns failure status, but Match currently does not surface it;
  an unsuccessful write can leave the new BEST visible in RAM until the next boot.
- Frame-time headroom on the final quiet image was not recorded. Diagnostic UART dumps
  themselves can stall gameplay, so capture long dumps at the menu or Game Over screen.
- Alternate OLED variants and external MCP deployment were outside this board review.

The confirmed play checks are complete for this revision. Repeat them after changes to
message flow, timers, rendering, pools, or score persistence.
