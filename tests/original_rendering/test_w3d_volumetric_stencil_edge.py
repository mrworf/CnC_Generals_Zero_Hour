#!/usr/bin/env python3
"""Exercise the generated original XYZ volume public-edge dispatch boundary."""
import argparse
import os
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument("--executable",type=Path,required=True); parser.add_argument("--source-root",type=Path,required=True)
parser.add_argument("--physical",action="store_true")
args=parser.parse_args()
with TemporaryDirectory(prefix="zh-m22-volume-edge-") as scratch:
    root=Path(scratch); os.environ["ZH_M22_VOLUME_STENCIL_EDGE_PROFILE"]="1"
    if args.physical: os.environ["ZH_M22_VOLUME_STENCIL_EDGE_PHYSICAL"]="1"
    result=run(args.executable.resolve(),root/"run",source_tree(root/"source",load_m20_fixture(args.source_root.resolve()),"valid"),"mission")
    marker="original volume stencil edge: public-dispatch=1 retries=5 foreign=1 generations=2 resources=0"
    physical_marker="original volume stencil edge: physical-dispatch=1 generations=2 resources=0"
    if result.returncode or marker not in result.stdout or (args.physical and physical_marker not in result.stdout):
        raise SystemExit(result.stdout[-2000:]+result.stderr[-2000:])
os.environ.pop("ZH_M22_VOLUME_STENCIL_EDGE_PROFILE",None)
os.environ.pop("ZH_M22_VOLUME_STENCIL_EDGE_PHYSICAL",None)
