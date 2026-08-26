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

## Flash

Through the AK UART bootloader:

```bash
make -C application flash dev=/dev/ttyUSB0
```

Through ST-Link:

```bash
make -C application flash PROGRAMMER_PATH=/path/to/STM32CubeProgrammer/bin
```

## Play

- On the menu, use **Up** or **Down** to choose Easy, Normal, or Hard.
- Press **Mode** to start.
- During a kick, press **Up** for left, **Mode** for center, or **Down** for right.
- At game over, press **Mode** to retry or **Up** to return home.

## Inspect the board

Connect to the UART console at 115200 8N1. Safe inspection commands include:

```text
ver
fatal l
lcd d
```

Use the AK documentation MCP `decode_ak_lcd` tool to convert the `lcd d` framebuffer dump into text art and PNG.
