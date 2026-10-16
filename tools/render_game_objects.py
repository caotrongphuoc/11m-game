#!/usr/bin/env python3
"""Export original sprites, movement previews, and captured Match playback."""

from pathlib import Path
import json
import re

from PIL import Image

from render_game_screens import BITMAP_SOURCE, ROOT, parse_array


OUTPUT = ROOT / "resources/images/objects"
APP = ROOT / "application/sources/app"
GAME = APP / "game_eleven_meter"
SCALE = 4
PALETTE = [0, 0, 0, 255, 255, 255] + [0] * 762
BITMAPS = {
    "bitmap_penalty_shooter_prepare": (40, 40),
    "bitmap_penalty_shooter_kick": (40, 40),
    "bitmap_penalty_shooter_follow": (40, 40),
    "bitmap_penalty_ball": (7, 7),
    "bitmap_penalty_keeper_idle": (40, 40),
    "bitmap_penalty_keeper_dive_left": (40, 40),
    "bitmap_penalty_keeper_dive_right": (40, 40),
    "bitmap_penalty_goal": (93, 38),
}


def constant(source: Path, symbol: str) -> int:
    match = re.search(rf"^#define\s+{re.escape(symbol)}\s+\((-?\d+)\)", source.read_text(), re.MULTILINE)
    if match is None:
        raise ValueError(f"expected integer constant {symbol} in {source}")
    return int(match.group(1))


def interpolate(start: int, end: int, frame: int, steps: int) -> int:
    # C++ integer division truncates toward zero, unlike Python's negative //.
    delta = (end - start) * frame
    return start + (abs(delta) // steps) * (-1 if delta < 0 else 1)


def load_bitmaps() -> dict[str, Image.Image]:
    images = {}
    for symbol, (width, height) in BITMAPS.items():
        data = parse_array(BITMAP_SOURCE, symbol)
        if len(data) != ((width + 7) // 8) * height:
            raise ValueError(f"invalid bitmap size: {symbol}")
        images[symbol] = Image.frombytes("1", (width, height), bytes(data))
    return images


def save_gif(name: str, samples: list[tuple[Image.Image, tuple[int, int], int]], padding: int = 2) -> None:
    left = min(origin[0] for _, origin, _ in samples) - padding
    top = min(origin[1] for _, origin, _ in samples) - padding
    right = max(origin[0] + image.width for image, origin, _ in samples) + padding
    bottom = max(origin[1] + image.height for image, origin, _ in samples) + padding
    frames = []
    durations = []
    for image, (x, y), duration in samples:
        canvas = Image.new("P", (right - left, bottom - top), 0)
        canvas.putpalette(PALETTE)
        canvas.paste(1, (x - left, y - top), image)
        frames.append(canvas.resize((canvas.width * SCALE, canvas.height * SCALE), Image.NEAREST))
        durations.append(duration)
    frames[0].save(OUTPUT / name, format="GIF", save_all=True, append_images=frames[1:],
                   duration=durations, loop=0, disposal=2, optimize=False)


def render_shooter(images: dict[str, Image.Image]) -> None:
    source = GAME / "em_game_shooter.h"
    start_x = constant(source, "EM_GAME_SHOOTER_START_X")
    start_y = constant(source, "EM_GAME_SHOOTER_START_Y")
    steps = constant(source, "EM_GAME_SHOOTER_STEP_COUNT")
    kick_step = constant(source, "EM_GAME_SHOOTER_KICK_STEP_X")
    interval = constant(APP / "app.h", "EM_GAME_SHOOTER_ANIM_TICK_INTERVAL")
    screen = APP / "screens/scr_game_penalty.cpp"
    width = constant(screen, "SCR_GAME_PENALTY_SHOOTER_BITMAP_WIDTH")
    height = constant(screen, "SCR_GAME_PENALTY_SHOOTER_BITMAP_HEIGHT")
    bottom_padding = constant(screen, "SCR_GAME_PENALTY_SHOOTER_BITMAP_BOTTOM_PADDING")
    image = images["bitmap_penalty_shooter_prepare"]
    y = start_y - height + bottom_padding
    samples = [(image, (start_x - width // 2, y), 640)]
    for frame in range(steps + 1):
        x = start_x + ((steps - frame) * kick_step) // steps
        samples.append((image, (x - width // 2, y), interval if frame < steps else 640))
    save_gif("em_game_shooter.gif", samples)


def render_ball(images: dict[str, Image.Image]) -> None:
    source = GAME / "em_game_ball.h"
    start_x = constant(source, "EM_GAME_BALL_START_X")
    start_y = constant(source, "EM_GAME_BALL_START_Y")
    target_x = constant(source, "EM_GAME_BALL_TARGET_RIGHT_X")
    target_y = constant(source, "EM_GAME_BALL_TARGET_Y")
    steps = constant(source, "EM_GAME_BALL_STEP_COUNT")
    interval = constant(APP / "app.h", "EM_GAME_BALL_ANIM_TICK_INTERVAL")
    image = images["bitmap_penalty_ball"]
    samples = [(image, (start_x - image.width // 2, start_y - image.height // 2), 640)]
    for frame in range(steps + 1):
        x = interpolate(start_x, target_x, frame, steps)
        y = interpolate(start_y, target_y, frame, steps)
        samples.append((image, (x - image.width // 2, y - image.height // 2), interval if frame < steps else 640))
    save_gif("em_game_ball.gif", samples)


def render_keeper(images: dict[str, Image.Image]) -> None:
    source = GAME / "em_game_keeper.h"
    screen = APP / "screens/scr_game_penalty.cpp"
    start_x = constant(source, "EM_GAME_KEEPER_START_X")
    start_y = constant(source, "EM_GAME_KEEPER_START_Y")
    steps = constant(source, "EM_GAME_KEEPER_STEP_COUNT")
    interval = constant(APP / "app.h", "EM_GAME_KEEPER_ANIM_TICK_INTERVAL")
    width = constant(screen, "SCR_GAME_PENALTY_KEEPER_DIVE_BITMAP_WIDTH")
    idle_offset = constant(screen, "SCR_GAME_PENALTY_KEEPER_IDLE_Y_OFFSET")
    idle = (images["bitmap_penalty_keeper_idle"], (start_x - width // 2, start_y + idle_offset), 640)
    samples = [idle]
    for direction in ("left", "right"):
        target_x = constant(source, f"EM_GAME_KEEPER_TARGET_{direction.upper()}_X")
        offset = constant(screen, f"SCR_GAME_PENALTY_KEEPER_DIVE_{direction.upper()}_Y_OFFSET")
        for frame in range(steps + 1):
            x = interpolate(start_x, target_x, frame, steps)
            samples.append((images[f"bitmap_penalty_keeper_dive_{direction}"],
                            (x - width // 2, start_y + offset), interval if frame < steps else 480))
        samples.append(idle)
    save_gif("em_game_keeper.gif", samples)


def main() -> None:
    OUTPUT.mkdir(parents=True, exist_ok=True)
    images = load_bitmaps()
    for symbol, image in images.items():
        save_gif(f"{symbol}.gif", [(image, (0, 0), 1000)], padding=0)
    render_shooter(images)
    render_ball(images)
    render_keeper(images)
    capture = json.loads((ROOT / "resources/captures/em_game_match_uart.json").read_text())
    if not capture["complete"] or {frame["round"] for frame in capture["frames"] if frame["round"]} != {1, 2, 3, 4, 5}:
        raise ValueError("Match GIF requires a complete five-round capture")
    if capture["frames"][-1]["phase"] != "game_over" or not any(frame["phase"] == "rip" for frame in capture["frames"]):
        raise ValueError("Match GIF requires the match-over transition and final score")
    samples = []
    for frame in capture["frames"]:
        data = bytes.fromhex(frame["data_hex"])
        if len(data) != 1024:
            raise ValueError("invalid Match framebuffer size")
        image = Image.new("1", (128, 64))
        for y in range(64):
            for x in range(128):
                if data[(y // 8) * 128 + x] & (1 << (y % 8)):
                    image.putpixel((x, y), 1)
        samples.append((image, (0, 0), frame["duration_ms"]))
    save_gif("em_game_match.gif", samples, padding=0)
    print(f"Exported {len(images)} original bitmap GIFs, 3 movement GIFs, and the captured Match GIF")


if __name__ == "__main__":
    main()
