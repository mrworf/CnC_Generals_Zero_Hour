#!/usr/bin/env python3
"""Couple generated GameInfo slots and parser-backed multiplayer settings."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

EXPECTED = {
    "slots": "original generated game-info slots: generations=2 slots=0",
    "settings": "original generated multiplayer settings: generations=2 templates=0 colors=0",
}

def main() -> int:
    parser = argparse.ArgumentParser()
    for name in EXPECTED:
        parser.add_argument(f"--{name}", type=Path, required=True)
    args = parser.parse_args()
    for name, marker in EXPECTED.items():
        executable = getattr(args, name)
        if not executable.is_file():
            raise RuntimeError(f"missing {name} witness")
        result = subprocess.run([str(executable)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode or marker not in result.stdout + result.stderr:
            raise RuntimeError(f"{name} witness did not close its two-generation source contract")
    print("original generated multiplayer slot/settings aggregate: generations=2 providers=0")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
