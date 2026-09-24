#!/usr/bin/env python3
"""Witness the generated original MultiPlayerLoadScreen owner transaction."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


EXPECTED = "M22 original multiplayer loadscreen: ok generations=2 windows=0 display_strings=0 pixels=0"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--owner", type=Path, required=True)
    args = parser.parse_args()
    if not args.owner.is_file():
        raise RuntimeError("missing original multiplayer owner witness")
    result = subprocess.run([str(args.owner)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode or EXPECTED not in result.stdout + result.stderr:
        raise RuntimeError("generated original multiplayer owner did not close its two-generation transaction")
    print("original generated multiplayer loadscreen: generations=2 windows=0 pixels=0")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
