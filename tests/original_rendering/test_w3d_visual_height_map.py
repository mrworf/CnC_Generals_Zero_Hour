#!/usr/bin/env python3
"""Exercise bounded visual WorldHeightMap metadata without texture resources."""

import argparse
import os
from pathlib import Path
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


def ascii_string(value: str) -> bytes:
    encoded = value.encode("ascii")
    return struct.pack("<H", len(encoded)) + encoded


def chunk(chunk_id: int, version: int, payload: bytes) -> bytes:
    return struct.pack("<IHI", chunk_id, version, len(payload)) + payload


def visual_map(kind: str = "valid", texture_name: str = "Void") -> bytes:
    names = ["HeightMapData", "BlendTileData"]
    toc = bytearray(b"CkMp" + struct.pack("<I", len(names)))
    for index, name in enumerate(names, 1):
        encoded = name.encode("ascii")
        toc.extend(struct.pack("<B", len(encoded)) + encoded + struct.pack("<I", index))
    height = struct.pack("<7i", 8, 8, 0, 1, 8, 8, 64) + bytes(range(64))
    arrays = [bytearray(128) for _ in range(4)]
    cliffs = bytearray(8)
    if kind == "active_blend":
        arrays[1][0:2] = struct.pack("<h", 1)
    if kind == "active_cliff":
        cliffs[0] = 1
    length = 63 if kind == "shape" else 64
    counts = (1, 1, 1, 201 if kind == "oversize" else 1)
    blend = bytearray(struct.pack("<i", length))
    blend.extend(b"".join(arrays) + cliffs + struct.pack("<4i", *counts))
    blend.extend(struct.pack("<4i", 0, 1, 1, 0) + ascii_string(texture_name))
    blend.extend(struct.pack("<2i", 0, 0))
    toc.extend(chunk(1, 4, height))
    if kind != "missing":
        toc.extend(chunk(2, 8, bytes(blend)))
    if kind == "duplicate":
        toc.extend(chunk(2, 8, bytes(blend)))
    result = bytes(toc)
    return result[:-12] if kind == "truncated" else result


def authored_visual_map(kind: str = "valid") -> bytes:
    names = ["HeightMapData", "BlendTileData"]
    toc = bytearray(b"CkMp" + struct.pack("<I", len(names)))
    for index, name in enumerate(names, 1):
        encoded = name.encode("ascii")
        toc.extend(struct.pack("<B", len(encoded)) + encoded + struct.pack("<I", index))
    height = struct.pack("<7i", 8, 8, 0, 1, 8, 8, 64) + bytes(range(64))
    tiles = [0] * 64
    tiles[1] = 16
    blends = [0] * 64
    blends[0] = 1
    extra = [0] * 64
    extra[2] = 1
    cliff_indices = [0] * 64
    cliff_indices[4] = 1
    if kind == "bad_tile":
        tiles[3] = 20
    if kind == "bad_blend":
        blends[3] = 2
    if kind == "bad_cliff":
        cliff_indices[3] = 2
    cliff_bits = bytearray(8)
    cliff_bits[0] = 1
    blend = bytearray(struct.pack("<i", 64))
    for values in (tiles, blends, extra, cliff_indices):
        blend.extend(struct.pack("<64h", *values))
    blend.extend(cliff_bits)
    blend.extend(struct.pack("<4i", 5, 2, 2, 2))
    blend.extend(struct.pack("<4i", 0, 4, 2, 0) + ascii_string("VoidA"))
    second_first = 3 if kind == "overlap" else (5 if kind == "bad_span" else 4)
    blend.extend(struct.pack("<4i", second_first, 1, 1, 0) + ascii_string("VoidB"))
    blend.extend(struct.pack("<2i", 1, 1))
    blend.extend(struct.pack("<3i", 0, 1, 1) + ascii_string("VoidEdge"))
    flag = 0 if kind == "bad_flag" else 0x7ADA0000
    blend.extend(struct.pack("<i6Bii", 4, 1, 0, 0, 0, 0, 0, 0, flag))
    cliff_u0 = float("nan") if kind == "bad_real" else 0.0
    blend.extend(struct.pack("<i8f2B", 16, cliff_u0, 0.0, 0.0, 1.0,
                             1.0, 1.0, 1.0, 0.0, 1, 0))
    if kind == "tail":
        blend.extend(b"x")
    toc.extend(chunk(1, 4, height))
    toc.extend(chunk(2, 8, bytes(blend)))
    return bytes(toc)


def query_visual_map(cliff_extra: bool = False) -> bytes:
    """Authored metadata covering every runtime blend/cliff query branch."""
    names = ["HeightMapData", "BlendTileData"]
    toc = bytearray(b"CkMp" + struct.pack("<I", len(names)))
    for index, name in enumerate(names, 1):
        encoded = name.encode("ascii")
        toc.extend(struct.pack("<B", len(encoded)) + encoded + struct.pack("<I", index))
    height = struct.pack("<7i", 8, 8, 0, 1, 8, 8, 64) + bytes(range(64))
    tiles = [0] * 64
    blends = [0] * 64
    extras = [0] * 64
    cliffs = [0] * 64
    blend_cells = (0, 1, 2, 3, 4, 5, 6, 8, 9)
    for record, cell in enumerate(blend_cells, 1):
        blends[cell] = record
    for record, cell in enumerate((16, 17, 18, 19, 20, 21, 22, 24), 1):
        extras[cell] = record
    for quadrant, cell in enumerate((40, 41, 42, 43)):
        tiles[cell] = quadrant
    tiles[32] = 16
    cliffs[32] = 1
    if cliff_extra:
        cliffs[24] = 1
    cliff_bits = bytearray(8)
    cliff_bits[4] = 1
    blend = bytearray(struct.pack("<i", 64))
    for values in (tiles, blends, extras, cliffs):
        blend.extend(struct.pack("<64h", *values))
    blend.extend(cliff_bits)
    blend.extend(struct.pack("<4i", 5, 10, 2, 2))
    blend.extend(struct.pack("<4i", 0, 4, 2, 0) + ascii_string("VoidA"))
    blend.extend(struct.pack("<4i", 4, 1, 1, 0) + ascii_string("VoidB"))
    blend.extend(struct.pack("<2i", 1, 1))
    blend.extend(struct.pack("<3i", 0, 1, 1) + ascii_string("VoidEdge"))
    records = (
        (4, 1, 0, 0, 0, 0, 0, -1),
        (5, 1, 0, 0, 0, 3, 0, -1),
        (6, 0, 1, 0, 0, 0, 0, -1),
        (7, 0, 1, 0, 0, 1, 0, -1),
        (8, 0, 0, 1, 0, 0, 0, -1),
        (9, 0, 0, 1, 0, 1, 1, -1),
        (10, 0, 0, 0, 1, 0, 1, -1),
        (16 if cliff_extra else 11, 0, 0, 0, 1, 0 if cliff_extra else 1, 1, -1),
        (12, 1, 0, 0, 0, 2, 0, 0),
    )
    for record in records:
        blend.extend(struct.pack("<i6Bii", *record, 0x7ADA0000))
    blend.extend(struct.pack("<i8f2B", 16, 0.0, 0.0, 0.0, 1.0,
                             1.0, 1.0, 1.0, 0.0, 1, 0))
    toc.extend(chunk(1, 4, height))
    toc.extend(chunk(2, 8, bytes(blend)))
    return bytes(toc)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-visual-height-map-") as scratch:
        root = Path(scratch)
        source = root / "readonly-input"
        prepare_owned_source(source, fixture)
        variants = {
            "INPUT": "valid", "MISSING": "missing", "DUPLICATE": "duplicate",
            "TRUNCATED": "truncated", "SHAPE": "shape", "ACTIVE_BLEND": "active_blend",
            "ACTIVE_CLIFF": "active_cliff", "OVERSIZE": "oversize",
        }
        for env_name, kind in variants.items():
            path = root / f"{kind}.map"
            path.write_bytes(visual_map(kind))
            path.chmod(0o444)
            os.environ[f"ZH_M22_VISUAL_MAP_{env_name}"] = str(path)
        for env_name, kind in {
                "AUTHORED": "valid", "AUTHORED_BAD_SPAN": "bad_span",
                "AUTHORED_OVERLAP": "overlap", "AUTHORED_BAD_REAL": "bad_real",
                "AUTHORED_BAD_TILE": "bad_tile", "AUTHORED_BAD_BLEND": "bad_blend",
                "AUTHORED_BAD_CLIFF": "bad_cliff", "AUTHORED_BAD_FLAG": "bad_flag",
                "AUTHORED_TAIL": "tail"}.items():
            path = root / f"authored-{kind}.map"
            path.write_bytes(authored_visual_map(kind))
            path.chmod(0o444)
            os.environ[f"ZH_M22_VISUAL_MAP_{env_name}"] = str(path)
        fixture.make_read_only(source)
        os.environ["ZH_M22_VISUAL_HEIGHT_MAP_PROFILE"] = "1"
        for generation in range(2):
            result = run(args.executable.resolve(), root / f"generation-{generation}",
                         source, "mission")
            marker = "original visual WorldHeightMap metadata: map=8x8 flat=1 generations=2 resources=0"
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original visual map generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original visual WorldHeightMap metadata: parse/reject/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
