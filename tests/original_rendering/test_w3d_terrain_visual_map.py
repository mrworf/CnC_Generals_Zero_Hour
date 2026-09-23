#!/usr/bin/env python3
"""Exercise the original W3D terrain-visual generated-map transaction."""

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
    with TemporaryDirectory(prefix="zh-m22-terrain-visual-map-") as scratch:
        root = Path(scratch)
        map_path = root / "map.map"
        bad_map_path = root / "bad.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        bad_map_path.write_bytes(b"not a Generals map")
        map_path.chmod(0o444)
        bad_map_path.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_VISUAL_MAP_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_VISUAL_MAP"] = str(map_path)
        os.environ["ZH_M22_TERRAIN_VISUAL_BAD_MAP"] = str(bad_map_path)
        marker = "original terrain visual map: attach=1 rollback=3 draws=2 generations=2 resources=0"
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"generation-{generation}", source, "mission")
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original terrain visual map generation {generation} failed "
                                 f"({result.returncode}):\n{result.stdout}{result.stderr}")
    print("original terrain visual map: transaction/rollback/attachment/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
