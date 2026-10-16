#!/usr/bin/env python3
"""Wrap unchanged OLED previews in child backgrounds from the main banner theme."""

from PIL import Image, ImageDraw, ImageSequence

from game_art_theme import GREEN, WHITE, lettering, stadium_background
from render_game_screens import FONT_SOURCE, ROOT, parse_array


OUTPUT = ROOT / "resources/images/screens/framed"
BACKGROUNDS = OUTPUT / "backgrounds"
CARD_SIZE = (720, 480)
SCREEN_SIZE = (512, 256)
SCREEN_ORIGIN = (104, 128)
SCREEN_BOX = (104, 128, 616, 384)
# Source paths stay raw; these outputs must never feed back into the banner inputs.
CARDS = (
    ("resources/images/screens/scr_game_menu.gif", "scr_game_menu.gif", "DIFFICULTY", "SELECT YOUR CHALLENGE", "01", 0),
    ("resources/images/screens/scr_game_penalty.gif", "scr_game_penalty.gif", "GAMEPLAY", "COUNTDOWN / SHOT / RESULT", "02", 160),
    ("resources/images/screens/scr_game_rip.png", "scr_game_rip.png", "MATCH OVER", "FIVE KICKS COMPLETE", "03", 320),
    ("resources/images/screens/scr_game_over.png", "scr_game_over.png", "FINAL SCORE", "RESULT / RETRY / HOME", "04", 560),
    ("resources/videos/eleven_meter_demo.gif", "em_game_demo.gif", "GAME DEMO", "MENU / MATCH / SCORE / HOME", "05", 240),
    ("resources/images/objects/em_game_match.gif", "em_game_match.gif", "FULL MATCH", "FIVE ROUNDS / FINAL SCORE", "06", 480),
)


def put_text(canvas, font, text, origin, scale=2, color=GREEN):
    mask = lettering(font, text, scale)
    if origin[0] + mask.width > CARD_SIZE[0] - 24:
        raise ValueError(f"card label exceeds its bounds: {text}")
    canvas.paste(color, origin, mask)


def child_background(parent, font, title, subtitle, number, crop_x):
    canvas = parent.crop((crop_x, 80, crop_x + CARD_SIZE[0], 80 + CARD_SIZE[1]))
    draw = ImageDraw.Draw(canvas)
    draw.rectangle((24, 20, 695, 459), outline="#204a37", width=2)
    put_text(canvas, font, "ELEVEN METER", (40, 38), 3, WHITE)
    title_width = lettering(font, title, 2).width
    put_text(canvas, font, title, (680 - title_width, 43))
    put_text(canvas, font, subtitle, (40, 82), color=(151, 184, 169))
    draw.rectangle((88, 112, 631, 399), fill="#020605", outline=GREEN, width=2)
    draw.rectangle((100, 124, 619, 387), fill="#000000", outline="#204a37", width=2)
    for x, direction in ((88, 1), (631, -1)):
        for y, vertical in ((112, 1), (399, -1)):
            draw.line((x, y + vertical * 18, x, y, x + direction * 18, y), fill=GREEN, width=3)
    put_text(canvas, font, "AK BASE KIT / 128X64 OLED", (40, 430), color=(151, 184, 169))
    put_text(canvas, font, f"SCREEN {number}", (574, 430))
    return canvas


def wrap_screen(source, background):
    if source.width * 64 != source.height * 128:
        raise ValueError("OLED source must retain its 128:64 aspect ratio")
    canvas = background.copy()
    # Integer logical-pixel scaling: original OLED pixels occupy exactly 4x4 pixels.
    canvas.paste(source.convert("RGB").resize(SCREEN_SIZE, Image.NEAREST), SCREEN_ORIGIN)
    return canvas


def render_card(source_path, output_name, background):
    with Image.open(source_path) as source:
        if source.format != "GIF":
            wrap_screen(source, background).save(OUTPUT / output_name, optimize=True)
            return 1
        # Reserve exact black and white so palette reduction cannot tint OLED pixels.
        palette = Image.new("P", (1, 1))
        colors = background.quantize(colors=126, dither=Image.NONE).getpalette()[:378]
        palette.putpalette(([0, 0, 0, 255, 255, 255] + colors + [0] * 768)[:768])
        frames, durations = [], []
        for frame in ImageSequence.Iterator(source):
            canvas = wrap_screen(frame, background)
            frames.append(canvas.quantize(palette=palette, dither=Image.NONE))
            durations.append(frame.info["duration"])
        frames[0].save(OUTPUT / output_name, save_all=True, append_images=frames[1:],
                       duration=durations, loop=source.info.get("loop", 0), disposal=2, optimize=False)
        return len(frames)


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    BACKGROUNDS.mkdir(parents=True, exist_ok=True)
    parent = stadium_background()
    font = parse_array(FONT_SOURCE, "font")
    for source, name, title, subtitle, number, crop_x in CARDS:
        background = child_background(parent, font, title, subtitle, number, crop_x)
        background.save(BACKGROUNDS / (name.rsplit(".", 1)[0] + ".png"), optimize=True)
        count = render_card(ROOT / source, name, background)
        print(f"Rendered {name}: {count} source frames")


if __name__ == "__main__":
    main()
