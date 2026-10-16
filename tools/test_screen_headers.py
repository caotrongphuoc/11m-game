#!/usr/bin/env python3
"""Compile each screen header independently using the application's ARM flags."""

from pathlib import Path
import re
import shlex
import subprocess


ROOT = Path(__file__).resolve().parents[1]
APP = ROOT / "application"
SCREENS = APP / "sources/app/screens"


def main():
    dry_run = subprocess.check_output(
        ["make", "-n", "-B", "-C", str(APP),
         "build_ak-base-kit-stm32l151-application/scr_game_menu.o"], text=True)
    command = next(shlex.split(line) for line in dry_run.splitlines()
                   if "arm-none-eabi-g++ " in line)
    flags = [arg for arg in command[1:]
             if arg.startswith(("-I", "-D", "-U", "-std=", "-march=", "-mfloat-abi="))
             or arg == "-mthumb"]
    compiler = [command[0], *flags, "-Wall", "-Wextra", "-Werror",
                "-x", "c++", "-fsyntax-only", "-"]
    headers = sorted(SCREENS.glob("scr_*.h"))
    registry = (SCREENS / "screens.h").read_text()
    for header in headers:
        name = header.stem
        suffix = name.removeprefix("scr_")
        text = header.read_text()
        assert '#include "screens.h"' not in text, header.name
        assert f'#include "{header.name}"' not in registry, header.name
        declarations = (
            f"extern view_dynamic_t dyn_view_{suffix};",
            f"extern view_screen_t {name};",
            f"extern void {name}_handle(ak_msg_t* msg);",
        )
        for declaration in declarations:
            assert declaration in text, (header.name, declaration)
            assert declaration in registry, (header.name, declaration)
        implementation = header.with_suffix(".cpp").read_text()
        assert re.search(rf"void {name}_handle\(ak_msg_t\* msg\)", implementation)
        standalone = f'#include "{header.name}"\n#include "{header.name}"\n'
        subprocess.run(compiler, input=standalone, text=True, cwd=APP, check=True)
        subprocess.run(compiler, input=standalone + '#include "screens.h"\n',
                       text=True, cwd=APP, check=True)
        subprocess.run(compiler, input='#include "screens.h"\n' + standalone,
                       text=True, cwd=APP, check=True)
        print(f"PASS {header.name}: matching registry declarations, standalone and both include orders", flush=True)
    assert '#include "screens.h"' not in (APP / "sources/app/task_display.h").read_text()
    for header in ("screens.h", "task_display.h"):
        subprocess.run(compiler, input=f'#include "{header}"\n', text=True, cwd=APP, check=True)
    print("PASS screens.h and task_display.h: standalone compilation")


if __name__ == "__main__":
    main()
