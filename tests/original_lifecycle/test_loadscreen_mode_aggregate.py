#!/usr/bin/env python3
"""Witness the ordered, mode-exclusive generated source load-screen owners."""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


MARKERS = (
    "M20 original headless update: ok frames=2 ticks=2 network=offline-null devices=0",
    "M22 original multiplayer loadscreen: ok generations=2 windows=0 display_strings=0 pixels=0",
    "M22 original mode-exclusive loadscreen aggregate: ok singleplayer_generations=2 multiplayer_generations=2 windows=0 display_strings=0 pixels=0",
)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--owner", type=Path, required=True)
    args = parser.parse_args()
    if not args.owner.is_file():
        raise RuntimeError("missing mode-owner witness")
    result = subprocess.run([str(args.owner)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    output = result.stdout + result.stderr
    positions = [output.find(marker) for marker in MARKERS]
    if result.returncode or any(position < 0 for position in positions):
        raise RuntimeError("source owner did not close every generated mode lifecycle")
    if positions != sorted(positions):
        raise RuntimeError("source owners did not report the required single-player then multiplayer order")
    print("original generated mode-exclusive loadscreen aggregate: generations=2+2 windows=0 pixels=0")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
