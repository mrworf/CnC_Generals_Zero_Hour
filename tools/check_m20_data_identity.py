#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys


REQUIRED = {
    "GlobalData.cpp": "GlobalData::parseGameDataDefinition(",
    "NameKeyGenerator.cpp": "NameKeyGenerator::nameToKey(",
    "GameText.cpp": "CreateGameTextInterface(",
    "MapUtil.cpp": "MapCache::writeCacheINI(",
    "GameState.cpp": "GameState::getSaveDirectory(",
    "XferCRC.cpp": "XferCRC::xferSnapshot(",
    "ThingFactory.cpp": "ThingFactory::parseObjectDefinition(",
    "PosixLocalFileSystem.cpp": "PosixLocalFileSystem::PosixLocalFileSystem(std::filesystem",
}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--provider-target", required=True)
    args = parser.parse_args()

    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    marker = f"CMakeFiles/{args.provider_target}.dir/"
    compiled = {
        pathlib.Path(entry["file"]).name
        for entry in commands
        if marker in pathlib.Path(entry.get("output", "")).as_posix()
    }
    included = args.link_map.read_text(encoding="utf-8", errors="replace").split(
        "Discarded input sections", 1
    )[0]
    symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    failures = []
    for member, symbol in REQUIRED.items():
        if member not in compiled:
            failures.append(f"missing exact production compile unit: {member}")
        if f"lib{args.provider_target}.a({member}.o)" not in included:
            failures.append(f"production provider not extracted live: {member}")
        if symbol not in symbols:
            failures.append(f"missing live production symbol: {symbol}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"M20 original data identity: ok providers={len(REQUIRED)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
