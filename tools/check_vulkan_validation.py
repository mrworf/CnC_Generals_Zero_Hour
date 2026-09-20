#!/usr/bin/env python3
"""Fail closed unless the Khronos Vulkan validation layer is discoverable."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

LAYER = "VK_LAYER_KHRONOS_validation"


def parse(text: str) -> bool:
    return any(line.lstrip().startswith(LAYER) for line in text.splitlines())


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, help="Parse captured vulkaninfo text instead of probing the host")
    args = parser.parse_args()
    if args.input:
        text = args.input.read_text(encoding="utf-8")
    else:
        result = subprocess.run(
            ["vulkaninfo", "--summary"], text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False
        )
        text = result.stdout
        if result.returncode != 0:
            print("Vulkan summary probe failed:\n" + text, file=sys.stderr)
            return 2
    if not parse(text):
        print(
            f"{LAYER} is not discoverable; install Arch package 'vulkan-validation-layers' "
            "and rerun M14 with SDL_GPU debug mode enabled.",
            file=sys.stderr,
        )
        return 1
    print(f"{LAYER}: discoverable")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
