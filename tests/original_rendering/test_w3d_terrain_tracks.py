#!/usr/bin/env python3
"""Exercise the bounded active original terrain-track source route."""

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
    with TemporaryDirectory(prefix="zh-m22-terrain-tracks-") as scratch:
        root = Path(scratch)
        packet = root / "tracks.map"
        packet.write_bytes(visual_map(texture_name="Flat"))
        packet.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_TRACKS_PROFILE"] = "1"
        os.environ["ZH_M22_TERRAIN_TRACKS_MAP"] = str(packet)
        source = source_tree(root / "source", fixture, "valid")
        result = run(args.executable.resolve(), root / "run", source, "mission")
        marker = "original terrain tracks: queue=1 retry=2 disabled=1 expired=1 generations=2 resources=0"
        if result.returncode or marker not in result.stdout:
            raise SystemExit(f"original terrain tracks failed ({result.returncode}); private output redacted")
    print("original terrain tracks: active/rollback/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
