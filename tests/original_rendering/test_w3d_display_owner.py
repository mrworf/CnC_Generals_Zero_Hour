#!/usr/bin/env python3
"""Exercise original W3DDisplay owner bootstrap within an initialized source scenario."""

import argparse
import os
from pathlib import Path
import subprocess
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run
from test_w3d_status_scene import has_validation_diagnostic
from test_w3d_terrain_source_bitmap import tile_tga


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--physical", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-display-owner-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.write(source / "Maps/Owned/AssetUsage.txt", ";ignored\nTEST\n")
        fixture.write(source / "Maps/Owned/CommentsOnly.txt", ";TEST\n")
        fixture.write(source / "Art/Terrain/TreeA.tga", tile_tga(2,
                      ((3, 2, 1, 4), (7, 6, 5, 8),
                       (11, 10, 9, 12), (15, 14, 13, 16))))
        tree_b = tile_tga(1, ((23, 22, 21, 255),),
                          rle=True, depth=24, flags=0)
        fixture.write(source / "Art/Textures/TreeB.tga", tree_b)
        fixture.write(source / "Art/Terrain/TreeTrunc.tga", tree_b[:-7])
        fixture.write(source / "Art/Terrain/TreeBad.tga", tile_tga(1,
                      ((3, 2, 1, 4),), depth=16))
        half_header = struct.pack("<BBB5s4hBB", 0, 0, 2, b"\0" * 5,
                                  0, 0, 32, 32, 32, 0)
        fixture.write(source / "Art/Terrain/TreeHalf.tga",
                      half_header + bytes((31, 32, 33, 34)) * (32 * 32))
        wide = tile_tga(10, ((1, 2, 3, 4),) * 100, rle=True, flags=0)
        for index in range(6):
            fixture.write(source / f"Art/Terrain/TreeCap{index}.tga", wide)
        before = tuple(sorted((path.relative_to(source), path.read_bytes())
                              for path in source.rglob("*") if path.is_file()))
        fixture.make_read_only(source)
        packet = base / "rigid.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-rigid-tree", str(packet)],
                       check=True)
        os.environ["ZH_M22_DISPLAY_OWNER_PROFILE"] = "1"
        os.environ["ZH_M22_DISPLAY_OWNER_ASSET"] = str(packet)
        if args.physical:
            os.environ["ZH_M22_DISPLAY_OWNER_PHYSICAL"] = "1"
        result = run(args.executable.resolve(), base, source, "mission")
        marker = ("original display owner: bgfx generations=2 resources=0" if args.physical
                  else "original display owner: source generations=2 resources=0")
        output = result.stdout + result.stderr
        if result.returncode or marker not in result.stdout or has_validation_diagnostic(output):
            raise SystemExit(f"original display owner failed ({result.returncode}):\n{output}")
        after = tuple(sorted((path.relative_to(source), path.read_bytes())
                             for path in source.rglob("*") if path.is_file()))
        if before != after:
            raise SystemExit("original display owner changed read-only generated input")
    print("original W3DDisplay source owner lifecycle: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
