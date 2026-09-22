#!/usr/bin/env python3
"""Exercise source terrain TGA ownership before atlas creation."""

import argparse
import os
from pathlib import Path
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_visual_height_map import visual_map


def tga(kind: str) -> bytes:
    width = 128 if kind == "oversize" else 64
    height = 32 if kind == "dimensions" else 64
    image_type = 10 if kind == "compressed" else 2
    depth = 24 if kind == "depth" else 32
    header = struct.pack("<BBB5s4hBB", 0, 0, image_type, b"\0" * 5,
                         0, 0, width, height, depth, 0)
    pixels = bytes((3, 2, 1, 4)) * (width * height)
    result = header + pixels
    return result[:-17] if kind == "truncated" else result


def source_tree(root: Path, fixture, kind: str) -> Path:
    source = root / "readonly-input"
    prepare_owned_source(source, fixture)
    terrain_ini = ("Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\n"
                   "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
    if kind == "duplicate":
        terrain_ini += "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n"
    fixture.write(source / "Data/INI/Default/Terrain.ini", terrain_ini)
    if kind != "missing":
        fixture.write(source / "Art/Terrain/Flat.tga", tga(kind))
    fixture.make_read_only(source)
    return source


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-terrain-bitmap-") as scratch:
        root = Path(scratch)
        map_path = root / "flat.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_BITMAP_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_BITMAP_MAP"] = str(map_path)
        marker = "original terrain source bitmap: class=Flat tile=64 mips=7 resources=0"
        for generation in range(2):
            source = source_tree(root / f"valid-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"run-valid-{generation}", source, "mission")
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"terrain bitmap generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
        duplicate_source = source_tree(root / "duplicate", fixture, "duplicate")
        duplicate = run(args.executable.resolve(), root / "run-duplicate",
                        duplicate_source, "mission")
        if duplicate.returncode or marker not in duplicate.stdout:
            raise SystemExit("terrain bitmap duplicate authored name changed source replacement")
        for kind in ("missing", "compressed", "depth", "dimensions", "oversize", "truncated"):
            source = source_tree(root / kind, fixture, kind)
            result = run(args.executable.resolve(), root / f"run-{kind}", source, "mission")
            if result.returncode == 0 or marker in result.stdout:
                raise SystemExit(f"terrain bitmap {kind} input did not fail closed")
    print("original terrain source bitmap: decode/reject/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
