"""Document both authored WWShade macro branches without claiming active use."""

import argparse
import json
import pathlib
import subprocess
import sys


def expand(header: pathlib.Path, enabled: bool) -> str:
    source = '#include "shdlib.h"\nvoid source_flush() { SHD_FLUSH; }\n'
    command = ["c++", "-E", "-P", "-x", "c++", "-I", str(header.parent)]
    if enabled:
        command.append("-DUSE_WWSHADE=1")
    command.append("-")
    return subprocess.run(command, input=source, capture_output=True, text=True, check=True).stdout


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--header", type=pathlib.Path, required=True)
    args = parser.parse_args()
    commands = json.loads(args.compile_commands.read_text())
    source = next((item for item in commands if item["file"].endswith("/WW3D2/ww3d.cpp")
                   and "CMakeFiles/zh_w3d.dir" in item["command"]), None)
    if source is None or "-DUSE_WWSHADE" in source["command"]:
        print("current canonical WW3D CPU configuration is not the authored disabled branch", file=sys.stderr)
        return 1
    disabled, enabled = expand(args.header, False), expand(args.header, True)
    if "void source_flush() { ; }" not in disabled or "SHD_Flush();" not in enabled:
        print("original WWShade source macro branch changed", file=sys.stderr)
        return 1
    print("original WWShade authored branches: current disabled, enabled calls original SHD_Flush")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
