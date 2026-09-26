#!/usr/bin/env python3
"""Exercise original terrain bounds and scene update registration."""

import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map


# The retained 64-type/4000-instance workload plus two topple generations
# takes about 58 seconds natively and 258 seconds total under isolated GCC
# ASan+UBSan. Bound each generated process, including all its assertions.
GENERATION_TIMEOUT_SECONDS = 240


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-terrain-scene-attachment-") as scratch:
        root = Path(scratch)
        map_path = root / "map.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        packet = root / "tree.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-rigid-tree", str(packet)],
                       check=True)
        os.environ["ZH_M22_TERRAIN_SCENE_ATTACHMENT_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_SCENE_ATTACHMENT_MAP"] = str(map_path)
        os.environ["ZH_M22_TERRAIN_SCENE_ATTACHMENT_ASSET"] = str(packet)
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid",
                                 tree_textures=True, immobile_enemy=True,
                                 crusher_logic=True, fx_lists=True)
            try:
                result = run(args.executable.resolve(), root / f"generation-{generation}", source,
                             "mission", timeout_seconds=GENERATION_TIMEOUT_SECONDS)
            except subprocess.TimeoutExpired:
                raise SystemExit(f"original terrain scene attachment generation {generation} "
                                 f"timed out after {GENERATION_TIMEOUT_SECONDS} seconds; "
                                 "private output redacted") from None
            marker = "original terrain scene attachment: bounds=8x8 registrations=2 shroud=2 display-cells=3 generations=2 resources=0"
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original terrain scene attachment generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original terrain scene attachment: bounds/registration/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
