#!/usr/bin/env python3
"""Exercise bounded original flat-terrain Recording submission."""

import argparse
import os
from pathlib import Path
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import authored_source_tree
from test_w3d_visual_height_map import authored_visual_map, query_visual_map, visual_map


def sized_visual_map(width: int, height: int) -> bytes:
    names = ["HeightMapData", "BlendTileData"]
    toc = bytearray(b"CkMp" + struct.pack("<I", len(names)))
    for index, name in enumerate(names, 1):
        encoded = name.encode("ascii")
        toc.extend(struct.pack("<B", len(encoded)) + encoded + struct.pack("<I", index))
    count = width * height
    heights = bytes(index % 256 for index in range(count))
    height_data = struct.pack("<7i", width, height, 0, 1, width, height, count) + heights
    blend = bytearray(struct.pack("<i", count))
    for _ in range(4):
        blend.extend(bytes(count * 2))
    blend.extend(bytes(((width + 7) // 8) * height))
    blend.extend(struct.pack("<4i", 1, 1, 1, 1))
    blend.extend(struct.pack("<4i", 0, 1, 1, 0))
    blend.extend(struct.pack("<H", 4) + b"Flat")
    blend.extend(struct.pack("<2i", 0, 0))
    toc.extend(struct.pack("<IHI", 1, 4, len(height_data)) + height_data)
    toc.extend(struct.pack("<IHI", 2, 8, len(blend)) + blend)
    return bytes(toc)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-flat-terrain-") as scratch:
        root = Path(scratch)
        map_path = root / "flat.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        authored_path = root / "authored.map"
        authored_path.write_bytes(authored_visual_map())
        authored_path.chmod(0o444)
        inventory_path = root / "inventory.map"
        inventory_path.write_bytes(query_visual_map(cliff_extra=True))
        inventory_path.chmod(0o444)
        for env_name, dimensions in {
                "ZH_M22_MULTI_TILE_X_MAP": (35, 8),
                "ZH_M22_MULTI_TILE_Y_MAP": (8, 35),
                "ZH_M22_MULTI_TILE_EXACT_MAP": (65, 65),
                "ZH_M22_MULTI_TILE_TERRAIN_MAP": (35, 34)}.items():
            path = root / f"{env_name.lower()}.map"
            path.write_bytes(sized_visual_map(*dimensions))
            path.chmod(0o444)
            os.environ[env_name] = str(path)
        os.environ["ZH_M22_FLAT_TERRAIN_PROFILE"] = "1"
        os.environ["ZH_M22_FLAT_TERRAIN_MAP"] = str(map_path)
        os.environ["ZH_M22_AUTHORED_TERRAIN_MAP"] = str(authored_path)
        os.environ["ZH_M22_EXTRA_BLEND_TERRAIN_MAP"] = str(inventory_path)
        for generation in range(2):
            source = authored_source_tree(root / f"source-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"generation-{generation}", source, "mission")
            marker = "original flat terrain geometry: cells=7x7 vb=4096 ib=6144 draws=2 multitile=4"
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original flat terrain generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original flat terrain geometry: ownership/submission/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
