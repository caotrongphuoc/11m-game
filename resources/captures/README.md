# OLED framebuffer captures

Development capture/render scripts were removed during engineer-approved final cleanup.
The raw UART dumps, JSON samples, generated media, and historical measurements below
remain unchanged. Restore the relevant scripts and dependencies from Git history before
regenerating assets or making a new diagnostic capture.

## Menu-to-Home demo

The saved recordings predate the Full Time screen rename. Their original JSON
phase label `rip` and on-screen `MATCH OVER` text are preserved as capture evidence.
New captures use `full_time` and `FULL TIME`; capture/render tools accept both
versions. The current static Full Time preview is generated from drawing primitives,
not a new board capture. Existing demo videos and match GIFs remain historical recordings.

[`em_game_demo_uart.json`](em_game_demo_uart.json) is the source for the README's
autoplay [`demo GIF`](../videos/eleven_meter_demo.gif) and optional
[`MP4`](../videos/eleven_meter_demo.mp4). It records the real kit's OLED through the
Type-C CH340 UART at 115200 8N1 on **2026-10-16**, covering menu, Hard selection,
rounds 1–5, Match Over, an AI win and final score, then Home back to the menu.

The temporary capture image was based on firmware `307e766` (physical kick-button
mapping), with updater checksum **`0xA3C9`**, **56,724-byte file**. Only app-layer shell
instrumentation was added temporarily:

- Console/system printing was enabled; other four print/debug flags stayed off.
- `lcd d` paused and resumed the existing gameplay timer during synchronous UART output
  to avoid queuing ticks. No second gameplay timer, sprite, or game-rule changes were added.
- A temporary `lcd k d/u/m` command posted the existing Down/Up/Mode button signals to
  Display. The active screen translated them normally; no shared gameplay state was read
  or changed by the shell. This command is not present in the restored production image.

The capture driver chooses Hard and starts, lets each normal selection timeout trigger
a center shot, then presses Up for Home after Game Over. Accuracy, keeper randomness,
scoring, and winner evaluation are unmodified. The selected result is **YOU 2–3 CPU**.
The capture requires a naturally losing match; it does
not fabricate a loss from a winning recording. This is automated screen-input capture,
not physical button testing or camera footage.

Gameplay samples have **80 ms** presentation holds; UART transmission time is excluded.
The initial menu and Hard selection each have 1.2-second holds, Match Over two seconds,
the final score three seconds, and Home menu two seconds. These static holds are editing
choices for readability, not a real-time timing measurement. All saved framebuffer bytes
are preserved; rendering only enlarges their pixels. The GIF loops automatically.

The selected record contains **191 samples**, a **24.28-second** playback, and final
score **2–3**. The score framebuffer was inspected with AK's `decode_ak_lcd` tool.
Before and after the capture session, `fatal l` remained at **18 FATALs / 114 restarts**;
the MF31 record is historical. Message pool peaks were **7/32 pure** and **5/16 common**.
The logs were passed to AK's analyzer with this unchanged baseline for context.

All temporary firmware instrumentation was removed. The normal quiet image was clean-built
and restored successfully, updater checksum **`0xC0B5`** verified, **51,220-byte file**,
**51,216 bytes Flash**, **13,636 bytes RAM**. The physical left/middle/right kick mapping
is retained; bitmap arrays, `task_display`, kernel, and production log flags are unchanged.

The saved samples allow asset regeneration without a kit once the historical demo
renderer is restored. A new recording requires the temporary capture build described
above, not the quiet play image. Record at 80 ms active intervals and retain the actual
firmware checksum. Recording can affect the visible game through normal inputs;
do not interact with the kit concurrently.

## Original static gameplay capture

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

## PNG decoding

The historical screen renderer requires Pillow. It reads the saved dump, requires exactly **1,024 bytes**, and
decodes the OLED page layout: byte `(y // 8) * 128 + x`, bit `y % 8`, with the least
significant bit at the top of each page. The original frame is 128×64 monochrome pixels;
the README image is a 512×256 nearest-neighbor enlargement. The script regenerates the
capture image from the saved bytes rather than replacing it with the synthetic scene.

## Animated README screens

Figures 3 and 4 use these raw 512×256 animated OLED previews generated with the
historical screen renderer:

- **Figure 3 — [`scr_game_menu.gif`](../images/screens/scr_game_menu.gif):** generated
  menu preview selecting Easy → Normal → Hard → Normal, with 900 ms per selection.
  It uses the existing firmware layout, font, and drawing primitives; BEST 4 / HARD
  is illustrative data. It is not a UART recording and adds no firmware animation.
- **Figure 4 — [`scr_game_penalty.gif`](../images/screens/scr_game_penalty.gif):** all
  round-4 gameplay samples from [`em_game_match_uart.json`](em_game_match_uart.json),
  in capture order, covering countdown, shot, keeper response, and result. Pixels
  and 160 ms sample durations are preserved, excluding the UART transmission pauses.
  It is sampled playback, not a native-frame-rate video.

Both GIFs use a fixed black/white palette and clear each previous frame to prevent
trails. Original static PNGs remain available; the static gameplay capture described
above is no longer the image displayed as Figure 4 in the README.

The README places these raw previews inside 720×480 banner-themed cards. Restore the
historical card renderer before refreshing wrappers from changed raw GIFs.
The OLED viewport remains 512×256 with unchanged pixels and timing; the
[card notes](../images/screens/framed/README.md) describe the decorative borders.

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

The historical match capture helper read `lcd d` responses at 115200 8N1 and saved the decoded
page-major bytes, phase, and round for each sample. Its completion check requires rounds
1–5, Match Over, and Game Over. It requires the temporary capture build, not the quiet
production image. The recorded JSON is sufficient to regenerate the GIF without a kit
after restoring the historical object renderer from Git history.

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
