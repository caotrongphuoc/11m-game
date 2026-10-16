# Eleven Meter animated banner

[`eleven_meter_banner.gif`](eleven_meter_banner.gif) is a generated documentation banner,
not a new firmware screen or camera recording. It follows the visual structure of the
[Zomwar README banner](https://github.com/caotrongphuoc/zomwar-game): a 2:1 canvas, large
pixel title, a sprite row, and five preview panels. No Zomwar artwork or source code is
copied; the green stadium treatment and composition are implemented locally.

The banner uses the existing Eleven Meter OLED font, isolated Shooter/Ball/Keeper GIFs,
animated menu, round-4 UART gameplay playback, static Match Over/final-score previews,
and the repository's AK Base Kit photograph. The source bitmap arrays are unchanged.
The sprite row cycles isolated documentation animations independently; only the gameplay
panel represents the saved round. The result panel is an illustrative screen preview.

The 1280×640 GIF contains 23 frames with 160 ms holds, looping in 3.68 seconds. A fixed
128-color palette avoids frame-to-frame color shifts. Sprite and screen resizing uses
nearest-neighbor sampling; the kit photograph uses Lanczos resizing.

Regenerate from the repository root with Pillow installed:

```bash
python3 tools/render_game_screens.py
python3 tools/render_game_objects.py
python3 tools/render_game_banner.py
```

The first two commands refresh the existing preview inputs; the last command overwrites
only the generated banner GIF. No firmware changes or new kit captures are required.
