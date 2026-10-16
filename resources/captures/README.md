# OLED framebuffer captures

## Figure 4

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

## Full match GIF

[`em_game_match_uart.json`](em_game_match_uart.json) stores the complete 1,024-byte OLED
framebuffers sampled from the kit on **2026-10-16**, over the same Type-C UART connection.
It is the source of [`em_game_match.gif`](../images/objects/em_game_match.gif), shown in
the README's Match row. All five rounds, Match Over, and Game Over are included.
The capture contains **112 samples**; the resulting GIF has **112 frames** and a
**19.76-second** loop, ending with **YOU 3–2 CPU**. Every decoded GIF frame and duration
was checked against the saved samples, and the final framebuffer was inspected with
AK's `decode_ak_lcd` tool.

The temporary capture image used updater checksum **`0x040E`** and a **56,700-byte file**.
It started one normal match automatically from the menu, with normal timeout shots and
random keeper choices, without automatically retrying Game Over. As for Figure 4, it
paused the existing gameplay timer during each blocking UART framebuffer transmission
and resumed that same timer afterward; it did not add a gameplay timer or change sprites.

`tools/capture_game_match.py` reads `lcd d` responses at 115200 8N1 and saves the decoded
page-major bytes, phase, and round for each sample. Its completion check requires rounds
1–5, Match Over, and Game Over. It requires the temporary capture build, not the quiet
production image. The recorded JSON is sufficient to regenerate the GIF without a kit:

```bash
python3 tools/render_game_objects.py
```

The GIF is a sampled playback, not a real-time or native-frame-rate recording. Each
sample is held for the configured **160 ms** active interval, excluding UART transfer
time; the final score is held for two seconds. Identical adjacent samples may be merged
by the GIF encoder with their durations combined. Pixel data is only enlarged 4× using
nearest-neighbor scaling, without moving or redrawing sprites.

After this capture, the saved FATAL count remained **18**, unchanged from the baseline;
pure/common pool peaks were **8/32** and **5/16**. All temporary firmware helpers were
removed, the quiet image was clean-built and restored with updater checksum **`0xC0B0`**
verified successfully (**51,216 bytes Flash**, **13,636 bytes RAM**). Firmware sources,
including `task_display` and the original bitmap arrays, are unchanged by this update.
