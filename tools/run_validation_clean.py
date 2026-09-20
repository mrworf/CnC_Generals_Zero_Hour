#!/usr/bin/env python3
"""Run a GPU test and fail if its combined output contains Vulkan validation errors."""

from __future__ import annotations

import subprocess
import sys


def has_validation_error(output: str) -> bool:
    return "Validation Error:" in output or "VUID-" in output


def result_code(process_code: int, output: str) -> int:
    if process_code != 0:
        return process_code
    return 3 if has_validation_error(output) else 0


def main() -> int:
    command = sys.argv[1:]
    if not command:
        print("usage: run_validation_clean.py COMMAND [ARG ...]", file=sys.stderr)
        return 2
    result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    print(result.stdout, end="")
    code = result_code(result.returncode, result.stdout)
    if code == 3:
        print("GPU validation diagnostics were emitted; M14 acceptance fails.", file=sys.stderr)
    return code


if __name__ == "__main__":
    raise SystemExit(main())
