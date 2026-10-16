#!/usr/bin/env python3
"""Host-test the real penalty handler with message/timer boundaries stubbed out."""

from pathlib import Path
import re
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "application/sources/app/screens/scr_game_penalty.cpp"


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    match = re.search(r"void scr_game_penalty_handle\(ak_msg_t\* msg\)\s*\{", source)
    if match is None:
        raise ValueError("cannot locate the penalty handler")
    handler = source[match.start():]
    # Numeric assignments are irrelevant here: test the screen's symbolic routing,
    # not AK scheduling, allocations, GPIO reads, or the Match acceptance policy.
    symbols = sorted(set(re.findall(r"\b(?:AC_|EM_GAME_|SCREEN_|TIMER_)[A-Z0-9_]+\b", handler)))
    declarations = "enum { " + ", ".join(symbols) + " };\n"
    harness = r'''
#include <cassert>
#include <utility>
#include <vector>
struct ak_msg_t { int sig; };
static std::vector<std::pair<int, int>> messages;
static int timer_calls;
#define APP_DBG_SIG(...) ((void)0)
void task_post_pure_msg(int task, int signal) { messages.emplace_back(task, signal); }
void timer_remove_attr(int, int) { ++timer_calls; }
void timer_set(int, int, int, int) { ++timer_calls; }
'''
    checks = r'''
int main()
{
    const std::pair<int, int> buttons[] = {
        {AC_DISPLAY_BUTTON_DOWN_PRESSED, EM_GAME_MATCH_KICK_LEFT},
        {AC_DISPLAY_BUTTON_UP_PRESSED, EM_GAME_MATCH_KICK_CENTER},
        {AC_DISPLAY_BUTTON_MODE_PRESSED, EM_GAME_MATCH_KICK_RIGHT},
    };
    for (int repeat = 0; repeat < 20; ++repeat)
    {
        for (const auto& button : buttons)
        {
            messages.clear();
            timer_calls = 0;
            ak_msg_t msg{button.first};
            scr_game_penalty_handle(&msg);
            assert(messages.size() == 1);
            assert(messages[0].first == EM_GAME_MATCH_ID);
            assert(messages[0].second == button.second);
            assert(timer_calls == 0);
        }
    }
    messages.clear();
    ak_msg_t tick{EM_GAME_TIME_TICK};
    scr_game_penalty_handle(&tick);
    const std::vector<std::pair<int, int>> expected = {
        {EM_GAME_MATCH_ID, EM_GAME_MATCH_UPDATE},
        {EM_GAME_SHOOTER_ID, EM_GAME_SHOOTER_UPDATE},
        {EM_GAME_BALL_ID, EM_GAME_BALL_UPDATE},
        {EM_GAME_KEEPER_ID, EM_GAME_KEEPER_UPDATE},
    };
    assert(messages == expected);
    messages.clear();
    ak_msg_t unknown{255};
    scr_game_penalty_handle(&unknown);
    assert(messages.empty());
}
'''
    with tempfile.TemporaryDirectory(prefix="em-penalty-buttons-") as directory:
        test = Path(directory) / "penalty_buttons.cpp"
        binary = Path(directory) / "penalty_buttons"
        test.write_text(harness + declarations + handler + checks, encoding="utf-8")
        subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", str(test), "-o", str(binary)], check=True)
        subprocess.run([str(binary)], check=True)
    print("PASS: 60 button events route once to Match; shared tick routing and unknown-signal handling unchanged")


if __name__ == "__main__":
    main()
