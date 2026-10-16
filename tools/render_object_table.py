#!/usr/bin/env python3
"""Center object previews on equal-size plain canvases without decorative frames."""

from PIL import Image, ImageSequence

from render_game_screens import ROOT


OUTPUT = ROOT / "resources/images/objects/table"
CANVAS_SIZE = (512, 288)  # Displayed at 256x144 in the README.
PALETTE = [0, 0, 0, 255, 255, 255] + [0] * 762
PREVIEWS = (
    ("resources/images/objects/em_game_match.gif", "em_game_match.gif", 1),
    ("resources/images/objects/em_game_shooter.gif", "em_game_shooter.gif", 1),
    ("resources/images/objects/em_game_ball.gif", "em_game_ball.gif", 2),
    ("resources/images/objects/em_game_keeper.gif", "em_game_keeper.gif", 1),
    ("resources/images/objects/bitmap_penalty_goal.gif", "bitmap_penalty_goal.gif", 1),
    ("resources/images/screens/scr_game_over.png", "em_game_scoreboard.png", 1),
)


def compose_preview(image, scale):
    image = image.convert("1")
    size = (image.width * scale, image.height * scale)
    if size[0] > CANVAS_SIZE[0] or size[1] > CANVAS_SIZE[1]:
        raise ValueError("object preview exceeds the uniform canvas")
    image = image.resize(size, Image.NEAREST)
    canvas = Image.new("P", CANVAS_SIZE, 0)
    canvas.putpalette(PALETTE)
    origin = ((CANVAS_SIZE[0] - size[0]) // 2, (CANVAS_SIZE[1] - size[1]) // 2)
    canvas.paste(1, origin, image)
    return canvas


def main():
    OUTPUT.mkdir(parents=True, exist_ok=True)
    for source_name, output_name, scale in PREVIEWS:
        with Image.open(ROOT / source_name) as source:
            if source.format == "GIF":
                loop = source.info.get("loop")
                frames, durations = [], []
                for frame in ImageSequence.Iterator(source):
                    frames.append(compose_preview(frame, scale))
                    durations.append(frame.info.get("duration", 1000))
                options = {} if loop is None else {"loop": loop}
                frames[0].save(OUTPUT / output_name, save_all=True, append_images=frames[1:],
                               duration=durations, disposal=2, optimize=False, **options)
            else:
                compose_preview(source, scale).save(OUTPUT / output_name, optimize=True)
        print(f"Rendered {output_name}: {CANVAS_SIZE[0]}x{CANVAS_SIZE[1]}")


if __name__ == "__main__":
    main()
