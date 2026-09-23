#!/usr/bin/env python3
"""Couple the accepted generated VideoPlayer/VideoBuffer lifecycle witnesses.

The three executables intentionally retain independent source boundaries: the
original registry, the original base buffer metadata, and the live generated
stream-to-buffer transaction.  This gate runs those boundaries together and
requires each to complete two generations with ownership returned to zero.
It does not admit a retail decoder, W3D texture, or a new production owner.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


EXPECTED = {
    "registry": "original generated video stream registry: generations=2 streams=0",
    "buffer": "original generated video buffer: generations=2 bytes=0",
    "stream_buffer": "original generated stream-buffer: generations=2 resources=0",
}


def run(name: str, executable: Path) -> None:
    completed = subprocess.run(
        [str(executable)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    output = completed.stdout + completed.stderr
    if completed.returncode != 0 or EXPECTED[name] not in output:
        raise RuntimeError(
            f"{name} witness did not close its two-generation ownership contract\n{output}"
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--registry", type=Path, required=True)
    parser.add_argument("--buffer", type=Path, required=True)
    parser.add_argument("--stream-buffer", type=Path, required=True)
    arguments = parser.parse_args()
    for name, executable in vars(arguments).items():
        if not executable.is_file():
            raise RuntimeError(f"missing {name} witness")
        run(name, executable)
    print("original generated video aggregate: generations=2 providers=0 buffers=0")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
