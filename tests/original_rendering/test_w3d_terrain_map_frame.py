#!/usr/bin/env python3
"""Exercise the bounded source shroud/view/scene map frame through Recording."""

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
    with TemporaryDirectory(prefix="zh-m22-terrain-map-frame-") as scratch:
        root = Path(scratch)
        map_path = root / "map.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_MAP_FRAME_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_MAP_FRAME_MAP"] = str(map_path)
        marker = ("original terrain map frame: shroud-before-terrain=1 retry=2 "
                  "siblings=0 generations=2 resources=0")
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"generation-{generation}",
                         source, "mission")
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original terrain map frame generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original terrain map frame: ordering/rollback/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
