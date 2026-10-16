#!/usr/bin/env python3
"""Host-test Full Time input routing and new/legacy capture compatibility."""

import copy
import json
from pathlib import Path
import re
import subprocess
import tempfile

from capture_game_match import classify
from render_game_demo import CAPTURE, validate_capture
from render_game_screens import FONT_SOURCE, ROOT, parse_array, render_full_time


def main():
    source = (ROOT / "application/sources/app/screens/scr_game_full_time.cpp").read_text()
    match = re.search(r"void scr_game_full_time_handle\(ak_msg_t\* msg\)\s*\{", source)
    assert match is not None
    handler = source[match.start():]
    symbols = set(re.findall(r"\b(?:AC_|EM_GAME_|SCREEN_)[A-Z0-9_]+\b", handler))
    symbols.update(("AC_DISPLAY_BUTTON_UP_PRESSED", "AC_DISPLAY_BUTTON_DOWN_PRESSED"))
    harness = r'''
#include <cassert>
struct ak_msg_t { int sig; };
static int posts, removals;
#define APP_DBG_SIG(...) ((void)0)
void task_post_pure_msg(int task, int signal)
{
    assert(task == EM_GAME_MATCH_ID);
    assert(signal == EM_GAME_MATCH_SKIP_FULL_TIME);
    ++posts;
}
void timer_remove_attr(int task, int signal)
{
    assert(task == AC_TASK_DISPLAY_ID);
    assert(signal == AC_DISPLAY_SHOW_IDLE);
    ++removals;
}
'''
    checks = r'''
int main()
{
    ak_msg_t entry{SCREEN_ENTRY};
    scr_game_full_time_handle(&entry);
    assert(removals == 1 && posts == 0);
    ak_msg_t skip{AC_DISPLAY_BUTTON_MODE_PRESSED};
    scr_game_full_time_handle(&skip);
    assert(removals == 1 && posts == 1);
    const int ignored_signals[] = {AC_DISPLAY_BUTTON_UP_PRESSED,
                                   AC_DISPLAY_BUTTON_DOWN_PRESSED, -1};
    for (int signal : ignored_signals)
    {
        ak_msg_t ignored{signal};
        scr_game_full_time_handle(&ignored);
    }
    assert(removals == 1 && posts == 1);
}
'''
    with tempfile.TemporaryDirectory(prefix="em-full-time-") as directory:
        test = Path(directory) / "test.cpp"
        test.write_text("enum { " + ", ".join(sorted(symbols)) +
                        " };\n" + harness + handler + checks)
        binary = Path(directory) / "test"
        subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", str(test), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)

    font = parse_array(FONT_SOURCE, "font")
    image = render_full_time(font).image
    data = bytes(sum((1 << bit) if image.getpixel((x, page * 8 + bit)) else 0
                     for bit in range(8)) for page in range(8) for x in range(128))
    assert classify(data, font) == ("full_time", None)
    record = json.loads(CAPTURE.read_text())
    legacy = next(frame for frame in record["frames"] if frame["phase"] == "rip")
    assert classify(bytes.fromhex(legacy["data_hex"]), font) == ("full_time", None)
    validate_capture(record)
    renamed = copy.deepcopy(record)
    for frame in renamed["frames"]:
        if frame["phase"] == "rip":
            frame["phase"] = "full_time"
    validate_capture(renamed)
    print("PASS: Full Time entry/skip routing, preview detection, and legacy/new capture phases")


if __name__ == "__main__":
    main()
