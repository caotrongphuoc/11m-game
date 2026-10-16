#!/usr/bin/env python3
"""Encode saved OLED match samples as a silent, nearest-neighbor MP4 demo."""

import json
import subprocess

from PIL import Image

from render_game_screens import ROOT


CAPTURE = ROOT / "resources/captures/em_game_match_uart.json"
OUTPUT = ROOT / "resources/videos/eleven_meter_demo.mp4"
FPS = 25
SCALE = 6


def main() -> None:
    record = json.loads(CAPTURE.read_text(encoding="utf-8"))
    samples = record["frames"]
    rounds = {sample["round"] for sample in samples if sample["phase"] == "gameplay"}
    if not record["complete"] or rounds != {1, 2, 3, 4, 5}:
        raise ValueError("demo requires a complete five-round capture")
    if samples[-1]["phase"] != "game_over" or not any(s["phase"] == "rip" for s in samples):
        raise ValueError("demo requires Match Over and the final score")
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


if __name__ == "__main__":
    main()
