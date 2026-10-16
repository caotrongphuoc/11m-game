# Eleven Meter getting started

This guide covers the shortest path from a clean checkout to a running Eleven Meter firmware image.

## Requirements

- AK Embedded Base Kit v3 with STM32L151CBT6
- Arm GNU Toolchain 10.3-2021.10
- STM32CubeProgrammer for ST-Link flashing, or `ak-flash` for UART flashing
- A 115200 8N1 serial terminal for runtime inspection

## Build

Tool paths can be supplied on the command line without editing `application/Makefile`:

```bash
make -C application clean
make -C application \
  GCC_PATH=/path/to/gcc-arm-none-eabi-10.3-2021.10 \
  PROGRAMMER_PATH=/path/to/STM32CubeProgrammer/bin
```

The application binary is generated at:

```text
application/build_ak-base-kit-stm32l151-application/ak-base-kit-stm32l151-application.bin
```

Run a clean build after changing headers or compile flags. The current Makefile generates
dependency files but does not include them, and flag changes do not invalidate existing objects.
Use `make -C application info` to inspect Flash and RAM usage.

## Flash

Through the AK UART bootloader:

```bash
make -C application flash dev=/dev/ttyUSB0
```

On this kit, the Type-C connection exposes a CH340 UART bridge as `/dev/ttyUSB0` on Linux.
Check `/dev/serial/by-id/` if the port changes. Close any serial terminal before flashing.
The application starts at `0x08003000`; this workflow updates the application, not the bootloader.
The updater must report both `checksum OK` and completion.

If the board is stopped in FATAL and cannot accept the updater request, reset it before retrying.
If needed, enter boot mode by holding Mode and Reset, releasing Reset first, then retry the update.

Through ST-Link:

```bash
make -C application flash PROGRAMMER_PATH=/path/to/STM32CubeProgrammer/bin
```

## Play

- On the menu, use **Up** or **Down** to choose Easy, Normal, or Hard.
- Press **Mode** to start.
- During a kick, press **Up** for left, **Mode** for center, or **Down** for right.
- If the three-second countdown expires, the game shoots toward the center automatically.
- Score at least three goals in five attempts to win; saves and misses count for the CPU.
- The original standing player moves forward two pixels and settles back; there is no raised-leg animation.
- At game over, press **Mode** to retry or **Up** to return home.
- BEST survives power-off. Ties in goal count favor the harder difficulty.

## Inspect the board

The default play build disables all six flags in `application/Makefile`:

```make
-ULOGIN_PRINT_EN
-USYS_PRINT_EN
-UAPP_PRINT_EN
-USYS_DBG_EN
-UAPP_DBG_EN
-UAPP_DBG_SIG_EN
```

Consequently, commands such as `ver`, `fatal l`, `lcd d`, and `lcd s` do not print their
normal replies. This does not mean that UART firmware updating is disabled. The six flags
do not silence every independent framework or bootloader print.

For diagnostics, make a temporary build that enables shell replies and system diagnostics
while leaving application debug and signal tracing off. From the repository root:

```bash
make -C application clean
make -C application \
  CONSOLE_OPTION='-DLOGIN_PRINT_EN -DSYS_PRINT_EN -UAPP_PRINT_EN -USYS_DBG_EN -UAPP_DBG_EN -UAPP_DBG_SIG_EN'
ak-flash /dev/ttyUSB0 \
  application/build_ak-base-kit-stm32l151-application/ak-base-kit-stm32l151-application.bin \
  0x08003000
```

This command-line override leaves the checked-in flags unchanged. Tool path overrides
from the Build section can be combined with it.

Then connect at **115200 8N1**, with no flow control, and terminate commands with CR.
Read-only inspection commands include:

```text
ver
fatal l
lcd d
lcd s
```

`lcd s` prints the last draw and transfer durations plus the largest measured frame time,
in microseconds. Draw timing covers `clear()` to `update()`; an update without a clear
measures transfer only. Use the AK documentation MCP `decode_ak_lcd` tool to convert the
`lcd d` framebuffer dump into text art and PNG, and `analyze_ak_log` for captured logs.
The optional `ak-console.py` helper comes from the AK documentation server project; it is
not bundled in this game repository.

Run long framebuffer or saved-log dumps at the menu or Game Over screen. UART output is
synchronous and can stall the gameplay handler long enough to queue timer messages.
A stored FATAL record may predate the current firmware; compare counters across runs.

After diagnostics, restore the quiet play image with a clean default build and flash:

```bash
make -C application clean
make -C application flash dev=/dev/ttyUSB0
```

For the completed board checks and the distinction between manual confirmation and
captured evidence, see [Review and validation](05-review-and-validation.md).
