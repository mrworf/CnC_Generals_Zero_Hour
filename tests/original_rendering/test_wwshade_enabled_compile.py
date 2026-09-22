#!/usr/bin/env python3
"""Compile canonical W3D sources through the guarded WWShade branch."""

import argparse
import json
from pathlib import Path
import shlex
import subprocess
from tempfile import TemporaryDirectory


def source_command(commands: list[dict], filename: str) -> dict:
    matches = [entry for entry in commands if entry["file"].endswith("/" + filename)
               and "CMakeFiles/zh_w3d.dir" in entry["command"]]
    if len(matches) != 1:
        raise RuntimeError(f"canonical zh_w3d command for {filename} is not unique")
    if "ZH_WW3D_CPU_ONLY" not in matches[0]["command"] or "-DUSE_WWSHADE" in matches[0]["command"]:
        raise RuntimeError(f"canonical {filename} is not the disabled CPU-only branch")
    return matches[0]


def output_path(parts: list[str], directory: Path) -> Path:
    try:
        position = parts.index("-o")
        selected = Path(parts[position + 1])
    except (ValueError, IndexError):
        raise RuntimeError("canonical compile command lacks an output object") from None
    return selected if selected.is_absolute() else directory / selected


def undefined_symbols(path: Path) -> str:
    result = subprocess.run(["nm", "-C", "-u", str(path)],
                            capture_output=True, text=True, check=True)
    return result.stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compile-commands", type=Path, required=True)
    args = parser.parse_args()
    commands = json.loads(args.compile_commands.read_text())
    required = {"assetmgr.cpp": "SHD_Register_Loader",
                "ww3d.cpp": "SHD_Flush"}
    with TemporaryDirectory(prefix="zh-wwshade-enabled-") as scratch:
        for filename, symbol in required.items():
            entry = source_command(commands, filename)
            directory = Path(entry["directory"])
            parts = shlex.split(entry["command"])
            canonical = output_path(parts, directory)
            if not canonical.is_file() or symbol in undefined_symbols(canonical):
                raise RuntimeError(f"disabled {filename} unexpectedly needs enabled WWShade symbol")
            alternate = Path(scratch) / (filename + ".o")
            parts[parts.index("-o") + 1] = str(alternate)
            parts.insert(1, "-DUSE_WWSHADE=1")
            result = subprocess.run(parts, cwd=directory,
                                    capture_output=True, text=True, timeout=120)
            if result.returncode:
                raise RuntimeError(f"enabled {filename} compilation failed: {result.stderr[-1000:]}")
            if symbol not in undefined_symbols(alternate):
                raise RuntimeError(f"enabled {filename} omitted authored WWShade call")
    print("canonical disabled and guarded enabled WWShade source branches: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
