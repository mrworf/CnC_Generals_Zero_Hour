"""Fail-closed control for the original WW3D texture producer identity."""

import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--checker", type=Path, required=True)
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--link-map", type=Path, required=True)
    parser.add_argument("--executable", type=Path, required=True)
    args = parser.parse_args()
    source = "GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/texture.cpp"
    original = json.loads(args.compile_commands.read_text())
    assert any(item["file"].endswith(source) for item in original)
    with tempfile.TemporaryDirectory(prefix="m22-identity-") as tmp:
        removed_compile = Path(tmp) / "compile_commands.json"
        removed_compile.write_text(json.dumps([item for item in original if not item["file"].endswith(source)]))
        command = [sys.executable, str(args.checker), "--compile-commands", str(removed_compile),
                   "--link-map", str(args.link_map), "--executable", str(args.executable),
                   "--required", source]
        result = subprocess.run(command, capture_output=True, text=True, check=False)
        assert result.returncode != 0 and "was not compiled" in result.stderr, result.stderr
        removed_map = Path(tmp) / "link.map"
        removed_map.write_text(args.link_map.read_text().replace("texture.cpp.o", "removed-texture-provider.o"))
        command[command.index(str(removed_compile))] = str(args.compile_commands)
        command[command.index(str(args.link_map))] = str(removed_map)
        result = subprocess.run(command, capture_output=True, text=True, check=False)
        assert result.returncode != 0 and "was not linked" in result.stderr, result.stderr
    print("original WW3D texture provider removal rejected at compile and link boundaries")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
