#!/usr/bin/env python3
"""Prove the guarded full-draw target compiles the canonical status-circle owner."""

import argparse
import json
from pathlib import Path
import shlex
import subprocess


def symbols(path: Path) -> str:
    return subprocess.run(["nm", "-C", str(path)], capture_output=True,
                          text=True, check=True).stdout


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--full-link-map", type=Path, required=True)
    parser.add_argument("--production", type=Path, required=True)
    args = parser.parse_args()

    commands = json.loads(args.compile_commands.read_text())
    selected = [entry for entry in commands
                if entry["file"].endswith("/W3DStatusCircle.cpp")]
    if len(selected) != 1 or "CMakeFiles/zh_original_w3d_draw_full.dir" not in selected[0]["command"]:
        raise RuntimeError("canonical full-draw status-circle source command is missing or duplicated")
    command = selected[0]["command"]
    if "-DZH_WW3D_CPU_ONLY=1" not in command or "-DNDEBUG" not in command:
        raise RuntimeError("status-circle source lost its guarded original CPU ABI")
    parts = shlex.split(command)
    object_path = Path(parts[parts.index("-o") + 1])
    if not object_path.is_absolute():
        object_path = Path(selected[0]["directory"]) / object_path
    if not object_path.is_file():
        raise RuntimeError("canonical status-circle object was not built")
    defined = symbols(object_path)
    for name in ("W3DStatusCircle::W3DStatusCircle()",
                 "W3DStatusCircle::Render(RenderInfoClass&)",
                 "W3DStatusCircle::initData()"):
        if name not in defined:
            raise RuntimeError(f"canonical status-circle owner omitted {name}")
    if str(object_path.relative_to(Path(selected[0]["directory"]))) not in args.full_link_map.read_text():
        raise RuntimeError("full-draw probe did not link the canonical status-circle object")
    if "W3DStatusCircle::" in symbols(args.production):
        raise RuntimeError("schema-only production unexpectedly links physical status-circle bodies")
    print("guarded canonical status-circle compile/link and schema-only negative: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
