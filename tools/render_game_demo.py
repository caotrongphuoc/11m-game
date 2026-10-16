#!/usr/bin/env python3
"""Encode the saved menu-to-Home OLED capture as an autoplay GIF and silent MP4."""

import json
import subprocess

from PIL import Image

from render_game_screens import ROOT


CAPTURE = ROOT / "resources/captures/em_game_demo_uart.json"
OUTPUT = ROOT / "resources/videos/eleven_meter_demo.mp4"
GIF_OUTPUT = OUTPUT.with_suffix(".gif")
FPS = 25
SCALE = 6


def validate_capture(record) -> None:
    samples = record["frames"]
    rounds = {sample["round"] for sample in samples if sample["phase"] == "gameplay"}
    if not record["complete"] or rounds != {1, 2, 3, 4, 5}:
        raise ValueError("demo requires a complete five-round capture")
    phases = []
    for sample in samples:
        if not phases or phases[-1] != sample["phase"]:
            phases.append(sample["phase"])
    if phases != ["menu_start", "menu_hard", "gameplay", "rip", "game_over", "menu_home"] or record.get("winner") != "ai":
        raise ValueError("demo requires menu, Hard selection, play, loss, final score, and Home in order")
    round_order = [sample["round"] for sample in samples if sample["phase"] == "gameplay"]
    if round_order != sorted(round_order):
        raise ValueError("gameplay rounds are out of order")


def main() -> None:
    record = json.loads(CAPTURE.read_text(encoding="utf-8"))
    validate_capture(record)
    samples = record["frames"]
    # Validate everything before starting an encoder that overwrites the output.
    frames = []
    for sample in samples:
        data = bytes.fromhex(sample["data_hex"])
        duration = sample["duration_ms"]
        if len(data) != 1024 or duration <= 0 or duration * FPS % 1000:
            raise ValueError("invalid framebuffer or duration not representable at 25 fps")
        pixels = bytes(255 if data[(y // 8) * 128 + x] & (1 << (y % 8)) else 0
                       for y in range(64) for x in range(128))
        image = Image.frombytes("L", (128, 64), pixels)
        frames.append((image.resize((128 * SCALE, 64 * SCALE), Image.NEAREST).tobytes(), duration * FPS // 1000))
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    gif_frames = []
    for pixels, _ in frames:
        image = Image.frombytes("L", (128 * SCALE, 64 * SCALE), pixels)
        indexed = Image.new("P", image.size)
        indexed.putpalette([0, 0, 0, 255, 255, 255] + [0] * 762)
        indexed.paste(1, (0, 0), image)
        gif_frames.append(indexed)
    gif_frames[0].save(GIF_OUTPUT, save_all=True, append_images=gif_frames[1:],
                       duration=[sample["duration_ms"] for sample in samples],
                       loop=0, disposal=2, optimize=False)
    command = ["ffmpeg", "-hide_banner", "-loglevel", "error", "-y",
               "-f", "rawvideo", "-pixel_format", "gray", "-video_size", f"{128 * SCALE}x{64 * SCALE}",
               "-framerate", str(FPS), "-i", "pipe:0", "-an", "-c:v", "libx264",
               "-preset", "slow", "-crf", "18", "-profile:v", "high", "-level:v", "3.0",
               "-pix_fmt", "yuv420p", "-movflags", "+faststart", str(OUTPUT)]
    with subprocess.Popen(command, stdin=subprocess.PIPE) as encoder:
        try:
            for pixels, repeats in frames:
                for _ in range(repeats):
                    encoder.stdin.write(pixels)
        finally:
            encoder.stdin.close()
        if encoder.wait() != 0:
            raise RuntimeError("ffmpeg failed to encode the demo")
    print(f"Rendered {OUTPUT.relative_to(ROOT)}: {sum(n for _, n in frames)} frames at {FPS} fps")
    print(f"Rendered {GIF_OUTPUT.relative_to(ROOT)}: {len(samples)} samples, {sum(s['duration_ms'] for s in samples)} ms")


if __name__ == "__main__":
    main()
