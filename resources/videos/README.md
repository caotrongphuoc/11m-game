# Eleven Meter demo

[`eleven_meter_demo.mp4`](eleven_meter_demo.mp4) is a silent OLED playback of one complete
five-round match: timeout shots, goalkeeper responses, Match Over, and the final
**YOU 3–2 CPU** result. It is generated from the real kit framebuffer samples in
[`em_game_match_uart.json`](../captures/em_game_match_uart.json), not a camera recording
of the kit or a new live-input play test.

The video preserves the saved sample timing: 160 ms per sample and a two-second final
score hold, totaling **19.76 seconds**. UART transmission pauses are excluded. Its
25 fps container repeats each captured sample; it does not imply 25 unique captured
frames per second or interpolate missing movement. Original 128×64 pixels are enlarged
6× to **768×384** using nearest-neighbor scaling before high-quality H.264 encoding
(CRF 18, High profile, level 3.0, YUV420P), with MP4 fast-start metadata and no audio
track. MP4 compression is lossy; the raw capture remains the pixel-exact source.

Regenerate from the repository root with Python 3, Pillow, and FFmpeg with `libx264`:

```bash
python3 tools/render_game_demo.py
```

This overwrites only the generated MP4. No firmware, sprite, or capture data is changed.
See the [capture notes](../captures/README.md#full-match-gif) for the diagnostic capture
build, timing limitations, and restoration of the quiet production firmware.
