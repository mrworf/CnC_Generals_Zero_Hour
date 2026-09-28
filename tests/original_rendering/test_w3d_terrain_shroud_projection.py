#!/usr/bin/env python3
"""Exercise source-owned terrain shroud projection and material binding."""

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
    parser.add_argument("--gpu", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-terrain-shroud-projection-") as scratch:
        root = Path(scratch)
        map_path = root / "map.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_SHROUD_PROJECTION_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_SHROUD_PROJECTION_MAP"] = str(map_path)
        if args.gpu:
            os.environ["ZH_M22_TERRAIN_SHROUD_PHYSICAL"] = "1"
        marker = ("original terrain shroud projection: material=1 rollback=2 "
                  "generations=2 resources=0 draws=0")
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid")
            result = run(args.executable.resolve(), root / f"generation-{generation}",
                         source, "mission")
            output=result.stdout+result.stderr
            if (result.returncode or marker not in result.stdout or "Validation Error" in output
                    or "VUID-" in output or "runtime error:" in output or "ERROR: AddressSanitizer" in output
                    or (args.gpu and "original terrain shroud physical: nonuniform=1 rollback=1 generations=2 resources=0 committed-draws=10" not in result.stdout)):
                raise SystemExit(f"original terrain shroud projection generation {generation} "
                                 f"failed ({result.returncode}); private output redacted")
    print("original terrain shroud projection: ownership/rollback/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
