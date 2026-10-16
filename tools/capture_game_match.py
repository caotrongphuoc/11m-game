#!/usr/bin/env python3
"""Record one full match from UART lcd d replies on the temporary capture build."""

import argparse
import json
import os
from pathlib import Path
import re
import select
import termios
import time

from render_game_screens import FONT_SOURCE, ROOT, parse_array


def text_matches(data: bytes, font: list[int], x: int, y: int, text: str, color: bool = True) -> bool:
    for index, character in enumerate(text):
        for column in range(5):
            bits = font[ord(character) * 5 + column]
            for row in range(8):
                px, py = x + index * 6 + column, y + row
                lit = bool(data[(py // 8) * 128 + px] & (1 << (py % 8)))
                if lit != (color if bits & (1 << row) else not color):
                    return False
    return True


def classify(data: bytes, font: list[int]) -> tuple[str, int | None]:
    if text_matches(data, font, 25, 3, "ELEVEN METER"):
        return "menu", None
    if text_matches(data, font, 37, 2, "GAME OVER"):
        return "game_over", None
    if text_matches(data, font, 34, 5, "MATCH OVER", False):
        return "rip", None
    if text_matches(data, font, 41, 2, "R"):
        for round_number in range(1, 6):
            if text_matches(data, font, 47, 2, str(round_number)):
                return "gameplay", round_number
    return "startup_or_menu", None


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyUSB0")
    parser.add_argument("--output", type=Path, help="defaults to separate match/demo capture files")
    parser.add_argument("--firmware-checksum", required=True, help="updater checksum of the temporary capture image")
    parser.add_argument("--interval-ms", type=int, default=160)
    parser.add_argument("--timeout", type=int, default=240)
    parser.add_argument("--demo-flow", action="store_true", help="capture menu, a normal Hard loss, final score, and Home using temporary lcd k button injection")
    args = parser.parse_args()
    if args.interval_ms < 80 or args.timeout <= 0:
        parser.error("use an interval of at least 80 ms and a positive timeout")
    if args.demo_flow and args.interval_ms % 40:
        parser.error("demo intervals must be multiples of 40 ms for MP4 timing")
    if args.output is None:
        args.output = ROOT / "resources/captures" / ("em_game_demo_uart.json" if args.demo_flow else "em_game_match_uart.json")
    font = parse_array(FONT_SOURCE, "font")
    record = {"format": "ak-oled-page-major-lsb", "width": 128, "height": 64,
              "firmware_checksum": args.firmware_checksum, "captured_date": "2026-10-16",
              "interval_ms": args.interval_ms, "complete": False, "frames": []}
    if args.demo_flow:
        record["flow"] = "menu-play-loss-score-home"
    args.output.parent.mkdir(parents=True, exist_ok=True)

    def checkpoint() -> None:
        args.output.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")

    fd = os.open(args.port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        attrs = termios.tcgetattr(fd)
        attrs[0] = attrs[1] = attrs[3] = 0
        attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
        attrs[4] = attrs[5] = termios.B115200
        attrs[6][termios.VMIN] = attrs[6][termios.VTIME] = 0
        termios.tcsetattr(fd, termios.TCSANOW, attrs)
        deadline = time.monotonic() + args.timeout
        rounds = set()
        saw_rip = False
        previous = None
        menu_step = 0
        started = False
        returning_home = False
        while time.monotonic() < deadline:
            os.write(fd, b"lcd d\r")
            reply = bytearray()
            reply_deadline = time.monotonic() + 2.0
            while time.monotonic() < reply_deadline:
                if select.select([fd], [], [], 0.02)[0]:
                    reply.extend(os.read(fd, 8192))
                    if b"[DUMP] frame buffer lcd => end" in reply:
                        break
            match = re.search(rb"\[DUMP\] frame buffer lcd => start(.*?)\[DUMP\] frame buffer lcd => end", reply, re.S)
            if match is None:
                continue
            data = bytes(int(value, 16) for value in re.findall(rb"0x([0-9a-fA-F]{2})\b", match.group(1)))
            if len(data) != 1024:
                raise ValueError(f"incomplete LCD reply: {len(data)} bytes")
            phase, round_number = classify(data, font)
            duration = args.interval_ms
            button = None
            if args.demo_flow:
                if phase == "startup_or_menu":
                    continue
                if phase == "menu":
                    if returning_home:
                        phase, duration = "menu_home", 2000
                    elif not started:
                        if menu_step == 0:
                            phase, duration, button = "menu_start", 1200, b"lcd k d\r"
                            menu_step = 1
                        elif menu_step == 1 and text_matches(data, font, 97, 26, "HARD", False):
                            phase, duration, button = "menu_hard", 1200, b"lcd k m\r"
                            started = True
                        else:
                            os.write(fd, b"lcd k d\r")
                            time.sleep(args.interval_ms / 1000)
                            continue
                if phase == "rip":
                    if saw_rip:
                        time.sleep(args.interval_ms / 1000)
                        continue
                    duration = 2000
                if phase == "game_over":
                    if not text_matches(data, font, 43, 17, "AI WINS!", False):
                        raise ValueError("normal match was not a loss; capture again without changing game outcomes")
                    record["winner"] = "ai"
                    duration, button, returning_home = 3000, b"lcd k u\r", True
            elif phase == "game_over":
                duration = 2000
            if round_number is not None:
                rounds.add(round_number)
            saw_rip = saw_rip or phase == "rip"
            record["frames"].append({"phase": phase, "round": round_number,
                                     "duration_ms": duration,
                                     "data_hex": data.hex()})
            if (phase, round_number) != previous:
                print(f"Captured {phase}, round={round_number}, frame={len(record['frames'])}", flush=True)
                previous = phase, round_number
            checkpoint()
            if button:
                os.write(fd, button)
            if phase == ("menu_home" if args.demo_flow else "game_over"):
                if rounds != {1, 2, 3, 4, 5} or not saw_rip:
                    raise ValueError(f"incomplete match: rounds={sorted(rounds)}, RIP={saw_rip}")
                record["complete"] = True
                checkpoint()
                print(f"Complete match: {len(record['frames'])} frames saved to {args.output}", flush=True)
                return
            # Only this active interval is used for GIF timing; UART transfer pauses
            # gameplay in the temporary capture build and is not replayed as delay.
            time.sleep(args.interval_ms / 1000)
        raise TimeoutError("full match was not captured within the recording window")
    finally:
        os.close(fd)


if __name__ == "__main__":
    main()
