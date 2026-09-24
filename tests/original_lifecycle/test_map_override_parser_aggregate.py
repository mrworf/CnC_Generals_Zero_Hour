#!/usr/bin/env python3
"""Couple the original map-override parser's generated lifecycle witnesses."""

import argparse
from pathlib import Path
import subprocess
import sys


def run(source_root: Path, relative: str, executable: Path) -> None:
    result = subprocess.run(
        [sys.executable, str(source_root / relative), "--executable", str(executable),
         "--source-root", str(source_root)],
        text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=180)
    if result.returncode:
        raise SystemExit("M22 08F1 generated map-override child failed; output redacted")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    root = args.source_root.resolve()
    source = (root / "GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp").read_text()
    start = source.index("void GameLogic::loadMapINI")
    body = source[start:source.index("// ------------------------------------------------------------------------------------------------", start)]
    required = ("INI ini;", "ini.load(", "map.str", "initMapStringFile", "doSmartAssetPurgeAndPreload")
    if any(token not in body for token in required):
        raise SystemExit("M22 08F1 original map-override source order changed")
    run(root, "tests/original_persistence/test_gamedata_baseline.py", args.executable.resolve())
    run(root, "tests/original_persistence/test_power_baseline.py", args.executable.resolve())
    print("M22 08F1 original map-override parser: generated overrides=2 reset=1 failures=closed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
