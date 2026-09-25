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
from test_w3d_visual_height_map import authored_visual_map, visual_map


def tga(kind: str) -> bytes:
    width = 128 if kind == "oversize" else 64
    height = 32 if kind == "dimensions" else 64
    image_type = 10 if kind == "compressed" else 2
    depth = 16 if kind == "depth" else 32
    header = struct.pack("<BBB5s4hBB", 0, 0, image_type, b"\0" * 5,
                         0, 0, width, height, depth, 0)
    pixels = bytes((3, 2, 1, 4)) * (width * height)
    result = header + pixels
    return result[:-17] if kind == "truncated" else result


def tile_tga(width: int, colors, *, rle=False, depth=32, flags=0x30,
             identifier=b"terrain") -> bytes:
    extent = width * 64
    header = struct.pack("<BBB5s4hBB", len(identifier), 0, 10 if rle else 2,
                         b"\0" * 5, 0, 0, extent, extent, depth, flags)
    pixels = bytearray()
    bytes_per_pixel = depth // 8
    for stream_y in range(extent):
        canonical_y = extent - stream_y - 1 if flags & 0x20 else stream_y
        if rle:
            for tile_x in range(width):
                canonical_x = extent - tile_x * 64 - 1 if flags & 0x10 else tile_x * 64
                color = colors[(canonical_y // 64) * width + canonical_x // 64]
                pixels.extend(bytes((0xBF,)) + bytes(color[:bytes_per_pixel]))
        else:
            for stream_x in range(extent):
                canonical_x = extent - stream_x - 1 if flags & 0x10 else stream_x
                color = colors[(canonical_y // 64) * width + canonical_x // 64]
                pixels.extend(bytes(color[:bytes_per_pixel]))
    return header + identifier + pixels


def authored_source_tree(root: Path, fixture, kind: str) -> Path:
    source = root / "readonly-input"
    prepare_owned_source(source, fixture)
    terrain_ini = (
        "Terrain DefaultTerrain\n Texture = A.tga\n Class = NONE\nEnd\n"
        "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n"
        "Terrain VoidA\n Texture = A.tga\n Class = NONE\nEnd\n"
        "Terrain VoidB\n Texture = B.tga\n Class = NONE\nEnd\n"
        "Terrain VoidEdge\n Texture = Edge.tga\n Class = NONE\nEnd\n")
    fixture.write(source / "Data/INI/Default/Terrain.ini", terrain_ini)
    colors = ((3, 2, 1, 4), (7, 6, 5, 8), (11, 10, 9, 12), (15, 14, 13, 16))
    a = tile_tga(2, colors)
    b = tile_tga(1, ((23, 22, 21, 24),), rle=True, flags=0)
    edge = tile_tga(1, ((33, 32, 31, 255),), depth=24, flags=0x20)
    if kind == "dimensions":
        a = tile_tga(1, (colors[0],))
    elif kind == "truncated":
        a = a[:-19]
    elif kind == "packet":
        b = b[:-2]
    if kind != "missing_base":
        fixture.write(source / "Art/Terrain/A.tga", a)
    fixture.write(source / "Art/Terrain/Flat.tga", tga("valid"))
    fixture.write(source / "Art/Terrain/B.tga", b)
    if kind != "missing_edge":
        fixture.write(source / "Art/Terrain/Edge.tga", edge)
    fixture.make_read_only(source)
    return source


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
        authored_map = root / "authored.map"
        authored_map.write_bytes(authored_visual_map())
        authored_map.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_BITMAP_AUTHORED_MAP"] = str(authored_map)
        authored_marker = "original terrain source tile sets: base=5 edge=1 mips=7 resources=0"
        for generation in range(2):
            source = authored_source_tree(root / f"authored-valid-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"run-authored-valid-{generation}",
                         source, "mission")
            if result.returncode or authored_marker not in result.stdout:
                raise SystemExit(f"authored terrain tile-set generation {generation} failed; "
                                 "private output redacted")
        for kind in ("dimensions", "truncated", "packet", "missing_base", "missing_edge"):
            source = authored_source_tree(root / f"authored-{kind}", fixture, kind)
            result = run(args.executable.resolve(), root / f"run-authored-{kind}", source, "mission")
            if result.returncode == 0 or authored_marker in result.stdout:
                raise SystemExit(f"authored terrain tile-set {kind} input did not fail closed")
    print("original terrain source bitmap: decode/reject/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
