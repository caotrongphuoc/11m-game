#!/usr/bin/env python3
"""Render a 2:1 animated banner using existing Eleven Meter screen/sprite assets."""

from PIL import Image, ImageDraw, ImageOps, ImageSequence

from render_game_screens import FONT_SOURCE, ROOT, parse_array
from game_art_theme import GREEN, HEIGHT, WHITE, WIDTH, label, lettering, stadium_background


OUTPUT = ROOT / "resources/images/eleven_meter_banner.gif"
IMAGES = ROOT / "resources/images"


def load_animation(path):
    with Image.open(path) as image:
        return [(frame.convert("RGB"), frame.info.get("duration", 160))
                for frame in ImageSequence.Iterator(image)]


def at_time(frames, time_ms):
    time_ms %= sum(duration for _, duration in frames)
    for image, duration in frames:
        if time_ms < duration:
            return image
        time_ms -= duration
    raise ValueError("invalid animation timing")


def main():
    font = parse_array(FONT_SOURCE, "font")
    menu = load_animation(IMAGES / "screens/scr_game_menu.gif")
    gameplay = load_animation(IMAGES / "screens/scr_game_penalty.gif")
    sprites = [load_animation(IMAGES / f"objects/em_game_{name}.gif")
               for name in ("shooter", "ball", "keeper")]
    with Image.open(IMAGES / "board/ak-embedded-base-kit-version-3.jpg") as source:
        board = ImageOps.contain(source.convert("RGB"), (224, 112), Image.LANCZOS)
    with Image.open(IMAGES / "screens/scr_game_rip.png") as source:
        rip = source.convert("RGB").resize((224, 112), Image.NEAREST)
    with Image.open(IMAGES / "screens/scr_game_over.png") as source:
        over = source.convert("RGB").resize((224, 112), Image.NEAREST)

    base = stadium_background()
    title = lettering(font, "ELEVEN METER", 15)
    x = (WIDTH - title.width) // 2
    base.paste((24, 94, 58), (x + 9, 73), title)
    base.paste((97, 131, 118), (x - 4, 60), title)
    base.paste(WHITE, (x, 64), title)
    label(base, font, "FIVE KICKS. ONE WINNER.", 218, 4)
    label(base, font, "STM32L151 / ACTIVE KERNEL / AK BASE KIT", 266, 2, (151, 184, 169))
    label(base, font, "ORIGINAL SPRITES", 390, 2, (100, 146, 122))
    label(base, font, "PENALTY SHOOTOUT ON THE AK EMBEDDED BASE KIT", 608, 2, (151, 184, 169))

    frames = []
    time_ms = 0
    for game_frame, duration in gameplay:
        frame = base.copy()
        for center, animation in zip((275, 640, 1005), sprites):
            sprite = at_time(animation, time_ms)
            sprite = sprite.resize((sprite.width // 2, sprite.height // 2), Image.NEAREST)
            frame.paste(WHITE, (center - sprite.width // 2, 378 - sprite.height), sprite.convert("L"))
        cards = (at_time(menu, time_ms).resize((224, 112), Image.NEAREST),
                 game_frame.resize((224, 112), Image.NEAREST), board, rip, over)
        draw = ImageDraw.Draw(frame)
        for index, (caption, image) in enumerate(zip(
                ("DIFFICULTY", "GAMEPLAY", "AK BASE KIT", "MATCH OVER", "FINAL SCORE"), cards)):
            left = 16 + index * 252
            label(frame, font, caption, 438, 2, center=left + 120)
            draw.rectangle((left, 460, left + 239, 587), fill="#020605", outline=GREEN, width=2)
            frame.paste(image, (left + (240 - image.width) // 2, 468))
        frames.append((frame, duration))
        time_ms += duration
    # A fixed palette keeps white sprite pixels stable between frames.
    palette = frames[0][0].quantize(colors=128, dither=Image.NONE)
    encoded = [frame.quantize(palette=palette, dither=Image.NONE) for frame, _ in frames]
    encoded[0].save(OUTPUT, save_all=True, append_images=encoded[1:],
                    duration=[duration for _, duration in frames], loop=0, disposal=2, optimize=False)
    print(f"Rendered {OUTPUT.relative_to(ROOT)}: {len(frames)} frames, {time_ms} ms")


if __name__ == "__main__":
    main()
