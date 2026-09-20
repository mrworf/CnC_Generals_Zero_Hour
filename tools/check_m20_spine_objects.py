#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys


REQUIRED = {
    "GameMain.cpp": ["GameMain(int, char**)"],
    "GameEngine.cpp": [
        "GameEngine::init(int, char**)",
        "GameEngine::execute()",
        "GameEngine::update()",
        "GameEngine::reset()",
        "GameEngine::~GameEngine()",
    ],
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--target", default="zh_original_lifecycle_spine")
    args = parser.parse_args()
    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    failures: list[str] = []
    matched: dict[str, pathlib.Path] = {}
    marker = f"CMakeFiles/{args.target}.dir/"
    for entry in commands:
        name = pathlib.Path(entry.get("file", "")).name
        if name not in REQUIRED or marker not in pathlib.Path(entry.get("output", "")).as_posix():
            continue
        output = pathlib.Path(entry["output"])
        if not output.is_absolute():
            output = pathlib.Path(entry["directory"]) / output
        matched[name] = output
    for name, symbols in REQUIRED.items():
        obj = matched.get(name)
        if obj is None or not obj.is_file():
            failures.append(f"missing exact active object for {name}")
            continue
        defined = subprocess.run(["nm", "-C", "--defined-only", str(obj)], check=True,
                                 text=True, stdout=subprocess.PIPE).stdout
        for symbol in symbols:
            if not any(symbol in line and " T " in line for line in defined.splitlines()):
                failures.append(f"missing strong source symbol in {name}: {symbol}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M20 original lifecycle spine objects: ok GameMain.cpp GameEngine.cpp")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
