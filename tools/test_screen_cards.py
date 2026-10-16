#!/usr/bin/env python3
"""Verify framed screens preserve OLED pixels, animation timing, and static borders."""

from PIL import Image, ImageDraw, ImageSequence

from render_game_screens import ROOT
from render_screen_cards import BACKGROUNDS, CARD_SIZE, CARDS, OUTPUT, SCREEN_BOX, SCREEN_SIZE


def without_screen(image):
    background = image.copy()
    ImageDraw.Draw(background).rectangle(
        (SCREEN_BOX[0], SCREEN_BOX[1], SCREEN_BOX[2] - 1, SCREEN_BOX[3] - 1), fill="black")
    return background.tobytes()


def main():
    for source_name, output_name, *_ in CARDS:
        with Image.open(ROOT / source_name) as source, Image.open(OUTPUT / output_name) as card:
            assert card.size == CARD_SIZE, output_name
            assert getattr(card, "n_frames", 1) == getattr(source, "n_frames", 1), output_name
            original_loop = source.info.get("loop")
            if source.format == "GIF":
                assert card.info.get("loop") == original_loop, output_name
            originals = [(frame.convert("RGB").resize(SCREEN_SIZE, Image.NEAREST), frame.info.get("duration"))
                         for frame in ImageSequence.Iterator(source)]
            actual = [(frame.convert("RGB"), frame.info.get("duration"))
                      for frame in ImageSequence.Iterator(card)]
            assert len(actual) == len(originals), output_name
            border = without_screen(actual[0][0])
            for index, ((expected, duration), (frame, actual_duration)) in enumerate(zip(originals, actual)):
                assert frame.crop(SCREEN_BOX).tobytes() == expected.tobytes(), (output_name, index, "OLED pixels")
                assert duration == actual_duration, (output_name, index, "duration")
                assert without_screen(frame) == border, (output_name, index, "border changed")
            background_name = output_name.rsplit(".", 1)[0] + ".png"
            with Image.open(BACKGROUNDS / background_name) as background:
                assert background.size == CARD_SIZE
            print(f"PASS {output_name}: {len(actual)} frames; OLED pixels, timing, loop, and static border preserved")


if __name__ == "__main__":
    main()
