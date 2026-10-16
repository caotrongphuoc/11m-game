#!/usr/bin/env python3
"""Render the README's basic sequence using Pillow and the current game flow."""

import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "resources/images/design-sequence/em_game_eleven_meter_sequence.png"
WIDTH = 1920
ROW_HEIGHT = 66
BACKGROUND = "#111827"
TEXT = "#e5e7eb"
MUTED = "#94a3b8"
PARTICIPANTS = (
    ("Player", "#84cc16"),
    ("Display", "#60a5fa"),
    ("Match", "#c084fc"),
    ("Shooter", "#facc15"),
    ("Ball", "#fb923c"),
    ("Keeper", "#22d3ee"),
    ("Goal", "#f87171"),
    ("EEPROM", "#a3a3a3"),
)

# Each message is a high-level summary, not an alternative AK signal definition.
# Exact message names and payloads remain in docs/02 and docs/03.
PHASES = (
    ("01  MENU AND ROUND SETUP", (
        ("Display", "Match", "Menu entry: initialize"),
        ("Match", "EEPROM", "Load BEST (first initialization)"),
        ("Player", "Display", "Up / Down: difficulty"),
        ("Display", "Match", "Cycle difficulty"),
        ("Match", "Keeper", "Set difficulty"),
        ("Player", "Display", "Mode: start"),
        ("Display", "Match", "Start match"),
        ("Match", "Match", "Reset score; advance to round 1"),
        ("Match", "Display", "Show penalty screen"),
        ("Display", "Shooter", "SETUP"),
        ("Display", "Ball", "SETUP"),
        ("Display", "Keeper", "SETUP"),
        ("Display", "Goal", "SETUP"),
        ("Display", "Display", "Start shared 40 ms tick"),
    )),
    ("02  SELECT AND EXECUTE A KICK", (
        ("Display", "Match", "Tick: update 3-second countdown"),
        ("Player", "Display", "Up / Mode / Down: choose shot"),
        ("Display", "Match", "Request left / center / right"),
        (None, None, "Timeout requests CENTER automatically; Match accepts only one kick per round."),
        ("Match", "Shooter", "Forward accepted kick"),
        ("Shooter", "Ball", "Kick: movement target"),
        ("Shooter", "Keeper", "React: shot zone"),
        ("Shooter", "Display", "Player snapshot: two-pixel step"),
    )),
    ("03  SHARED GAME LOOP", (
        (None, None, "Every 40 ms, the penalty screen posts UPDATE to Match, Shooter, Ball, and Keeper."),
        ("Display", "Match", "UPDATE: round timing"),
        ("Display", "Shooter", "UPDATE: 80 ms animation steps"),
        ("Display", "Ball", "UPDATE: 40 ms flight steps"),
        ("Display", "Keeper", "UPDATE: 40 ms dive steps"),
        ("Shooter", "Display", "Snapshot: standing sprite / recovery"),
        ("Ball", "Display", "Snapshot: ball position"),
        ("Keeper", "Display", "Snapshot: keeper position / dive"),
    )),
    ("04  RESOLVE AND ADVANCE", (
        ("Keeper", "Goal", "Keeper ready: zone"),
        ("Ball", "Goal", "Ball arrived: zone / wide"),
        (None, None, "Reports may arrive in either order. Goal waits for both and resolves once."),
        ("Goal", "Match", "Result: GOAL / SAVE / MISS"),
        ("Match", "Display", "Score / result snapshot"),
        (None, None, "Hold result for 1,200 ms. If fewer than five results: reset objects and start the next round."),
        ("Match", "Goal", "RESET (rounds 2-5 only)"),
        ("Match", "Ball", "RESET (rounds 2-5 only)"),
        ("Match", "Keeper", "RESET (rounds 2-5 only)"),
        ("Match", "Shooter", "RESET (rounds 2-5 only)"),
        (None, None, "Remaining rounds return to phase 02 on the same penalty screen; the shared tick stays active."),
    )),
    ("05  FINISH AFTER FIVE RESULTS", (
        ("Match", "EEPROM", "Save BEST only if improved"),
        ("Match", "Match", "Stop gameplay tick; enter RIP"),
        ("Match", "Display", "Show match-over (RIP) screen"),
        (None, None, "RIP ends after 2,000 ms, or earlier when Mode requests skip."),
        ("Match", "Display", "Show Game Over / final score"),
        (None, None, "YOU = goals; CPU = saves + misses. Three or more goals win the five-kick match."),
        ("Player", "Display", "Mode: retry / Up: home"),
        ("Display", "Match", "RETRY / HOME"),
        (None, None, "Retry resets the match and returns to phase 01 round setup; Home returns to the difficulty menu."),
    )),
)


def render(font_dir: Path) -> None:
    regular = str(font_dir / "DejaVuSans.ttf")
    bold = str(font_dir / "DejaVuSans-Bold.ttf")
    font = ImageFont.truetype(regular, 23)
    small = ImageFont.truetype(regular, 22)
    heading = ImageFont.truetype(bold, 26)
    title = ImageFont.truetype(bold, 37)
    height = 215 + sum(76 + len(rows) * ROW_HEIGHT for _, rows in PHASES) + 85
    image = Image.new("RGB", (WIDTH, height), BACKGROUND)
    draw = ImageDraw.Draw(image)
    centers = {name: 120 + index * 240 for index, (name, _) in enumerate(PARTICIPANTS)}
    draw.text((40, 24), "ELEVEN METER  /  BASIC GAME SEQUENCE", font=title, fill=TEXT)
    draw.text((40, 80), "High-level message flow. Time runs downward; arrows represent commands or snapshots.", font=small, fill=MUTED)

    for name, color in PARTICIPANTS:
        x = centers[name]
        draw.rounded_rectangle((x - 103, 132, x + 103, 188), radius=7, fill=color)
        draw.text((x, 160), name, anchor="mm", font=heading, fill=BACKGROUND)
        for y in range(200, height - 55, 15):
            draw.line((x, y, x, min(y + 7, height - 55)), fill=color, width=2)

    y = 215
    for label, rows in PHASES:
        draw.rectangle((20, y, WIDTH - 20, y + 50), fill="#25344b")
        draw.text((40, y + 25), label, anchor="lm", font=heading, fill=TEXT)
        y += 76
        for sender, receiver, message in rows:
            if sender is None:
                draw.rounded_rectangle((32, y - 12, WIDTH - 32, y + 40), radius=6, fill="#1e293b")
                draw.text((WIDTH // 2, y + 14), message, anchor="mm", font=small, fill="#cbd5e1")
            else:
                start, end = centers[sender], centers[receiver]
                line_y = y + 33
                if start == end:
                    bend = start + 55
                    draw.line(((start, line_y - 20), (bend, line_y - 20), (bend, line_y), (end, line_y)), fill=TEXT, width=2)
                    draw.polygon(((end, line_y), (end + 10, line_y - 5), (end + 10, line_y + 5)), fill=TEXT)
                    text_x, anchor = start + 65, "ls"
                else:
                    direction = 1 if end > start else -1
                    draw.line((start, line_y, end, line_y), fill=TEXT, width=2)
                    draw.polygon(((end, line_y), (end - direction * 12, line_y - 6), (end - direction * 12, line_y + 6)), fill=TEXT)
                    text_x, anchor = (start + end) // 2, "ms"
                # Keep readable labels above short arrows, without reducing font size.
                box = draw.textbbox((text_x, y + 19), message, font=font, anchor=anchor)
                assert box[0] >= 0 and box[2] <= WIDTH, f"label outside image: {message}"
                draw.rectangle((box[0] - 5, box[1] - 3, box[2] + 5, box[3] + 2), fill=BACKGROUND)
                draw.text((text_x, y + 19), message, anchor=anchor, font=font, fill=TEXT)
            y += ROW_HEIGHT

    draw.text((40, height - 35), "Details: docs/02-design-sequence-object.md and docs/03-design-sequence-runtime.md", font=small, fill=MUTED)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    image.save(OUTPUT, optimize=True)
    print(f"Rendered {OUTPUT.relative_to(ROOT)} ({WIDTH}x{height})")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--font-dir", type=Path, default=Path("/usr/share/fonts/truetype/dejavu"), help="directory containing DejaVuSans.ttf and DejaVuSans-Bold.ttf")
    render(parser.parse_args().font_dir)


if __name__ == "__main__":
    main()
