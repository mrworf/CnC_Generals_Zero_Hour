#!/usr/bin/env python3
"""Verify original sources were compiled, linked, and executed."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys


def verify(
    compile_commands: pathlib.Path,
    link_map: pathlib.Path,
    executable: pathlib.Path,
    required: list[str],
    witness: str = "original-support runtime provider=GeneralsMD EAC Refpack 1.01",
) -> None:
    commands = json.loads(compile_commands.read_text(encoding="utf-8"))
    compiled = {pathlib.Path(entry["file"]).as_posix() for entry in commands}
    map_text = link_map.read_text(encoding="utf-8", errors="replace")
    for source in required:
        if not any(path.endswith(source) for path in compiled):
            raise ValueError(f"required original provider was not compiled: {source}")
        object_stem = pathlib.Path(source).stem + ".cpp.o"
        if object_stem not in map_text:
            raise ValueError(f"required original provider was not linked: {source}")
    result = subprocess.run([str(executable)], text=True, capture_output=True, check=False)
    if result.returncode != 0:
        raise ValueError(f"original runtime witness failed: {result.stderr.strip()}")
    if witness not in result.stdout:
        raise ValueError("original runtime witness is missing")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--required", action="append", default=[])
    parser.add_argument("--witness", default="original-support runtime provider=GeneralsMD EAC Refpack 1.01")
    args = parser.parse_args(argv)
    try:
        verify(args.compile_commands, args.link_map, args.executable, args.required, args.witness)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"original identity error: {error}", file=sys.stderr)
        return 1
    print(f"original source identity: {len(args.required)} providers compiled, linked, and runtime witnessed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
