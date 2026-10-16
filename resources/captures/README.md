# OLED framebuffer capture for Figure 4

[`scr_game_penalty_uart.txt`](scr_game_penalty_uart.txt) contains the complete raw `lcd d`
reply captured from the STM32L151 kit on **2026-10-16**, through the Type-C CH340 bridge
at `/dev/ttyUSB0`, 115200 8N1. It is the source of
[`scr_game_penalty.png`](../images/screens/scr_game_penalty.png).

## Selected frame

The real frame shows **R4/5, YOU 2, CPU 2, GOAL**, with the original standing player at the
left, the ball in the middle, and the goalkeeper diving to the right. The three sprites
are fully visible and do not overlap. No sprites were moved or redrawn after capture.

The capture build used gameplay and sprite data from firmware `33ac918`, plus temporary
app-layer diagnostic helpers:

- Shell and system output were enabled; application debug and signal tracing stayed off.
- Menu entry started a normal match automatically; Game Over retried automatically to
  allow selection of a suitable frame. Shots still used the normal timeout and random
  keeper choice.
- While transmitting each synchronous `lcd d` reply, the capture helper paused the existing
  gameplay timer, then resumed the same 40 ms timer if it had been active. This prevented
  UART transmission time from queuing gameplay ticks during the dump.

The diagnostic image had updater checksum **`0x73F0`** and file length **56,708 bytes**.
The UART dump was decoded with AK's `decode_ak_lcd` tool and visually checked. All temporary
firmware helpers were removed after capture; the normal quiet build is separate from
this diagnostic image. The saved diagnostic frame is not a claim that console output is
enabled in the normal play build.

After capture, the original quiet image was rebuilt and flashed successfully: **51,220-byte
file**, updater checksum **`0xC0B0`**, application Flash **51,216 bytes**, RAM **13,636 bytes**.
The updater verified the checksum. No capture helpers or changes to the six log flags
remain in the firmware sources.

Before restoring quiet firmware, the saved FATAL count was still **18**, unchanged from
the pre-capture baseline; its MF31 record is historical, not a new crash. The diagnostic
build reported pool peaks of **8/32 pure** and **5/16 common** messages, and a maximum
measured OLED frame time of **38,328 microseconds**. These values describe the capture
session, not a new timing measurement of the quiet build.

## Reproduce the PNG

From the repository root:

```bash
python3 tools/render_game_screens.py
```

Pillow is required. The script reads the saved dump, requires exactly **1,024 bytes**, and
decodes the OLED page layout: byte `(y // 8) * 128 + x`, bit `y % 8`, with the least
significant bit at the top of each page. The original frame is 128×64 monochrome pixels;
the README image is a 512×256 nearest-neighbor enlargement. The script regenerates the
capture image from the saved bytes rather than replacing it with the synthetic scene.
