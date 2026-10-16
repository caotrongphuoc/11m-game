#!/usr/bin/env python3
"""Check object header contracts, extern definitions, and header include order."""

from pathlib import Path
import re
import shlex
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[1]
APP = ROOT / "application"
OBJECTS = APP / "sources/app/game_eleven_meter"


def main():
    dry_run = subprocess.check_output(
        ["make", "-n", "-B", "-C", str(APP),
         "build_ak-base-kit-stm32l151-application/em_game_ball.o"], text=True)
    command = next(shlex.split(line) for line in dry_run.splitlines()
                   if "arm-none-eabi-g++ " in line)
    flags = [arg for arg in command[1:]
             if arg.startswith(("-I", "-D", "-U", "-std=", "-march=", "-mfloat-abi="))
             or arg == "-mthumb"]
    compiler = [command[0], *flags, "-Wall", "-Wextra", "-Werror",
                "-x", "c++", "-fsyntax-only", "-"]
    headers = sorted(OBJECTS.glob("*.h"))
    sources = list((APP / "sources/app").rglob("*.cpp"))
    for header in headers:
        name = header.stem
        text = header.read_text()
        assert f"extern {name}_t {name};" in text, header.name
        assert "static_assert" not in text, header.name
        definitions = []
        for source in sources:
            source_text = source.read_text()
            if re.search(rf"^{name}_t {name}\s*=", source_text, re.M):
                definitions.append(source)
        owner = OBJECTS / ("em_game_match.cpp" if name == "em_game_scoreboard" else name + ".cpp")
        assert definitions == [owner], (name, definitions)
        # No consumer may bypass the owning handler by using its exported state.
        for source in sources:
            if source != owner:
                body = re.sub(r"^#include .*\n", "", source.read_text(), flags=re.M)
                assert not re.search(rf"\b{name}\b", body), (name, source)
        standalone = f'#include "{header.name}"\n#include "{header.name}"\n'
        subprocess.run(compiler, input=standalone, text=True, cwd=APP, check=True)
        for before, after in ((standalone, '#include "screens.h"\n'),
                              ('#include "task_display.h"\n', standalone)):
            subprocess.run(compiler, input=before + after, text=True, cwd=APP, check=True)
        print(f"PASS {header.name}: standalone/includes, one extern definition, owner-only state use", flush=True)
    for order in (headers, list(reversed(headers))):
        unit = "".join(f'#include "{header.name}"\n' for header in order)
        subprocess.run(compiler, input=unit, text=True, cwd=APP, check=True)
    print("PASS all object headers: forward and reverse include order")

    # Execute the real aggregate initializers on the host without MCU headers.
    # This checks defaults, not AK scheduling or physical board behavior.
    unit = '#include <cstdint>\n#include <cassert>\nstruct ak_msg_t;\n'
    for name in ("goal", "keeper", "scoreboard", "match", "ball", "shooter"):
        text = (OBJECTS / f"em_game_{name}.h").read_text()
        unit += re.sub(r"^#include .*\n", "", text, flags=re.M) + "\n"
    for source in sorted(OBJECTS.glob("*.cpp")):
        unit += "\n".join(re.findall(
            r"^em_game_\w+_t em_game_\w+\s*=.*?;", source.read_text(), re.M | re.S)) + "\n"
    unit += r'''
int main()
{
    assert(em_game_match.difficulty == EM_GAME_MATCH_DIFFICULTY_NORMAL);
    assert(em_game_match.best_difficulty == EM_GAME_MATCH_DIFFICULTY_EASY);
    assert(em_game_match.state == EM_GAME_MATCH_STATE_MENU);
    assert(em_game_match.last_result == EM_GAME_GOAL_RESULT_NONE);
    assert(!em_game_match.initialized && em_game_match.state_elapsed_ms == 0);
    assert(em_game_match.best_goals == 0 && em_game_match.countdown_seconds == 0);
    assert(em_game_keeper.difficulty == EM_GAME_KEEPER_DIFFICULTY_NORMAL);
    assert(!em_game_keeper.reaction_started && em_game_keeper.random_seed == 0);
    assert(em_game_ball.view.visible == 0 && !em_game_ball.kick_started);
    assert(em_game_shooter.random_seed == 0 && !em_game_shooter.kick_started);
    assert(!em_game_goal.ball_ready && !em_game_goal.keeper_ready && !em_game_goal.resolved);
    assert(em_game_scoreboard.round == 0 && em_game_scoreboard.goals == 0);
    assert(em_game_scoreboard.winner == EM_GAME_SCOREBOARD_WINNER_NONE);
}
'''
    with tempfile.TemporaryDirectory(prefix="em-object-defaults-") as directory:
        binary = str(Path(directory) / "test")
        subprocess.run(["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        "-x", "c++", "-", "-o", binary], input=unit, text=True, check=True)
        subprocess.run([binary], check=True)
    print("PASS all six object instances: initial defaults preserved")


if __name__ == "__main__":
    main()
