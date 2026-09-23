#!/usr/bin/env python3
"""Exercise bounded original flat-terrain Recording submission."""

import argparse
import os
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map


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
        os.environ["ZH_M22_FLAT_TERRAIN_PROFILE"] = "1"
        os.environ["ZH_M22_FLAT_TERRAIN_MAP"] = str(map_path)
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"generation-{generation}", source, "mission")
            marker = "original flat terrain geometry: cells=7x7 vb=4096 ib=6144 draws=2"
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original flat terrain generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original flat terrain geometry: ownership/submission/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
