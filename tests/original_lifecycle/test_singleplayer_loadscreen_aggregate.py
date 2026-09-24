#!/usr/bin/env python3
"""Couple the accepted generated SinglePlayerLoadScreen prerequisites.

The aggregate preserves the independent mapped-image, Mouse, generic-video,
and source-owner boundaries.  It admits no retail input or additional owner.
"""
from __future__ import annotations
import argparse
import subprocess
import sys
from pathlib import Path

EXPECTED = {
    "images": "original generated mapped-image provider: ini=1 generations=2 descriptors=0 resources=0",
    "mouse": "original generated mouse provider: generations=2 resources=0",
    "video": "original generated stream-buffer: generations=2 resources=0",
    "owner": "M20 original headless update: ok frames=2 ticks=2 network=offline-null devices=0",
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
            raise RuntimeError(f"{name} witness did not close its generated two-generation contract")
    print("original generated single-player aggregate: generations=2 providers=0 owner=0")
    return 0

if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
