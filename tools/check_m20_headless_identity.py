#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys


REQUIRED_PROVIDERS = {
    "GameClient.cpp": "GameClient::update()",
    "MessageStream.cpp": "MessageStream::propagateMessages()",
    "GameLogic.cpp": "GameLogic::update()",
    "Radar.cpp": "Radar::update()",
    "PlayerList.cpp": "PlayerList::PlayerList()",
    "GameWindowManagerScript.cpp": "GameWindowManager::winCreateLayout(AsciiString)",
    "WindowLayout.cpp": "WindowLayout::destroyWindows()",
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--provider-target", required=True)
    parser.add_argument("--spine-target", required=True)
    args = parser.parse_args()

    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    compiled = {(pathlib.Path(entry.get("file", "")).name,
                 pathlib.Path(entry.get("output", "")).as_posix()) for entry in commands}
    failures = []
    spine_marker = f"CMakeFiles/{args.spine_target}.dir/"
    if not any(name == "GameEngine.cpp" and spine_marker in output for name, output in compiled):
        failures.append("actual GameEngine.cpp is not compiled by the lifecycle spine")

    included = args.link_map.read_text(encoding="utf-8", errors="replace").split(
        "Discarded input sections", 1
    )[0]
    symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    for symbol in ("GameEngine::update()", "GameEngine::reset()"):
        if symbol not in symbols:
            failures.append(f"actual lifecycle symbol is not live: {symbol}")
    provider_marker = f"CMakeFiles/{args.provider_target}.dir/"
    for member, symbol in REQUIRED_PROVIDERS.items():
        if not any(name == member and provider_marker in output for name, output in compiled):
            failures.append(f"missing production compile unit: {member}")
        if f"lib{args.provider_target}.a({member}.o)" not in included:
            failures.append(f"provider archive member was not extracted live: {member}")
        if symbol not in symbols:
            failures.append(f"provider symbol is not live: {symbol}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"M20 original headless identity: ok providers={len(REQUIRED_PROVIDERS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
