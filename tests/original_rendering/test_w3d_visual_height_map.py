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


def visual_map(kind: str = "valid") -> bytes:
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
    blend.extend(struct.pack("<4i", 0, 1, 1, 0) + ascii_string("Flat"))
    blend.extend(struct.pack("<2i", 0, 0))
    toc.extend(chunk(1, 4, height))
    if kind != "missing":
        toc.extend(chunk(2, 8, bytes(blend)))
    if kind == "duplicate":
        toc.extend(chunk(2, 8, bytes(blend)))
    result = bytes(toc)
    return result[:-7] if kind == "truncated" else result


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
