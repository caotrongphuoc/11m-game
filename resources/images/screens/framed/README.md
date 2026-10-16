# Banner-themed screen cards

The README displays OLED screens inside 720×480 cards derived from the main Eleven Meter
banner's stadium-green background, pitch outlines, scanlines, pixel font, and colors.
`tools/game_art_theme.py` is the shared source for the banner and these cards; no separate
AI artwork or firmware UI is introduced.

The Objects table intentionally uses **unframed** previews, including Match and
Scoreboard. The generated Match card remains an optional asset, not the table image.

Each child background is a different 720×480 crop of that shared 1280×640 background,
with a title, subtitle, bezel, and footer added **outside** the OLED viewport. The saved
empty cards are in [`backgrounds/`](backgrounds/).

| Card | Original OLED asset | Output |
|---|---|---|
| Difficulty | [Menu preview](../scr_game_menu.gif) | [Menu card](scr_game_menu.gif) |
| Gameplay | [Captured round 4](../scr_game_penalty.gif) | [Gameplay card](scr_game_penalty.gif) |
| Match Over | [Transition preview](../scr_game_rip.png) | [Transition card](scr_game_rip.png) |
| Final Score | [Score preview](../scr_game_over.png) | [Score card](scr_game_over.png) |
| Full Match | [Five-round recording](../../objects/em_game_match.gif) | [Match card](em_game_match.gif) |

The OLED viewport is exactly **512×256**, at **(104, 128)**, with the original logical
128×64 pixels enlarged 4× using nearest-neighbor sampling. No artwork, text, or tint is
placed over the screen. All original source files remain unchanged. This includes the
raw Gameplay Demo GIF, which the README displays without a card. Clicking that GIF
opens the original MP4 demo.

GIF frame order, per-frame durations, total playback duration, and looping are preserved.
A fixed palette reserves pure black and white for the OLED, and each previous frame is
cleared to prevent trails. Decorative backgrounds are not part of the board framebuffer
and should not be interpreted as firmware graphics or new on-device validation.

Regenerate only the cards with Pillow installed:

```bash
python3 tools/render_screen_cards.py
python3 tools/test_screen_cards.py
```

If the underlying preview assets have changed, regenerate those first using their
existing scripts. The banner continues to use the **raw** screen assets, not these cards,
so it does not gain recursively nested frames. Keep `tools/` until the engineer confirms
that the entire game is finished and requests final cleanup.
