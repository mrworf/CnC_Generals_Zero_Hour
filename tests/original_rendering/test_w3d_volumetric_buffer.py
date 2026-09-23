#!/usr/bin/env python3
"""Exercise the generated source buffer-slot prerequisite for volume shadows."""
import argparse
import os
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree

def main() -> int:
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--source-root",type=Path,required=True)
    args=parser.parse_args(); fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-volume-buffer-") as scratch:
        root=Path(scratch); source=source_tree(root/"source",fixture,"valid")
        os.environ["ZH_M22_VOLUME_BUFFER_PROFILE"]="1"
        result=run(args.executable.resolve(),root/"run",source,"mission")
        marker="original volume buffer: source-slots=1 retry=2 negatives=1 removal=1 generations=2 resources=0"
        if result.returncode or marker not in result.stdout:
            raise SystemExit(f"original volume buffer failed ({result.returncode}): {result.stdout[-2000:]} {result.stderr[-2000:]}")
    os.environ.pop("ZH_M22_VOLUME_BUFFER_PROFILE",None)
    print("original volume buffer: source slots/failures/reentry ok")
    return 0

if __name__=="__main__": raise SystemExit(main())
