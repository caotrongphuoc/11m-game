# Eleven Meter demo

The README displays [`eleven_meter_demo.gif`](eleven_meter_demo.gif) automatically,
with [`eleven_meter_demo.mp4`](eleven_meter_demo.mp4) as an optional silent video.
Both show the same complete flow: **menu → select Hard → five rounds → Match Over →
AI WINS and final score → Home → menu**.

The loop lasts **24.28 seconds**, with **191 framebuffer samples**. The MP4 contains
607 repeated/held frames at 25 fps; the GIF preserves the captured monochrome pixels.

The source is the actual kit framebuffer capture in
[`em_game_demo_uart.json`](../captures/em_game_demo_uart.json), separate from the older
winning-match object GIF. A temporary app-layer shell helper posts normal button events
to the active screen to choose Hard, start, and return Home. This selected match uses
normal timeout-center shots and ends **YOU 2–3 CPU**. Keeper choice, accuracy, score,
and winner follow normal gameplay.
No goals, winner, score, or sprites were forced or edited.
This records screen output, not physical button actuation or camera footage of the kit.

Gameplay samples use **80 ms** holds, excluding UART transmission pauses. Each menu
selection is held for 1.2 seconds, Match Over for two seconds, the final score for three
seconds, and the returned menu for two seconds. Static holds are presentation timing;
this is not real-time or native-frame-rate footage. The 25 fps MP4 repeats samples rather
than interpolating movement. Original 128×64 pixels are enlarged 6× to **768×384** using
nearest-neighbor scaling. The GIF uses a fixed black/white palette and clears previous
frames to prevent trails. MP4 uses high-quality H.264 encoding
(CRF 18, High profile, level 3.0, YUV420P), with MP4 fast-start metadata and no audio
track. MP4 compression is lossy; the raw capture remains the pixel-exact source.

Regenerate from the repository root with Python 3, Pillow, and FFmpeg with `libx264`:

```bash
python3 tools/render_game_demo.py
```

This overwrites only the generated GIF and MP4. The renderer requires the complete
ordered menu-to-Home sequence, all five rounds, and the recorded AI win. No firmware,
sprite, or capture data is changed. See the
[capture notes](../captures/README.md#menu-to-home-demo) for the diagnostic capture build,
timing limitations, and restoration of the quiet production firmware.
