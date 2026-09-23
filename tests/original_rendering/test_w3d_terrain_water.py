#!/usr/bin/env python3
"""Exercise the bounded generated-map original translucent-water route."""
import argparse, os, sys
from pathlib import Path
from tempfile import TemporaryDirectory
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map
def main():
    p=argparse.ArgumentParser(description=__doc__); p.add_argument("--executable",type=Path,required=True); p.add_argument("--source-root",type=Path,required=True); a=p.parse_args()
    with TemporaryDirectory(prefix="zh-m22-terrain-water-") as scratch:
        root=Path(scratch); packet=root/"water.map"; packet.write_bytes(visual_map(texture_name="Flat")); packet.chmod(0o444)
        os.environ["ZH_M22_TERRAIN_WATER_PROFILE"]="1"; os.environ["ZH_M22_TERRAIN_WATER_MAP"]=str(packet); os.environ["ZH_M22_SMUDGE_EFFECT_PROFILE"]="1"
        result=run(a.executable.resolve(),root/"run",source_tree(root/"source",load_m20_fixture(a.source_root.resolve()),"valid"),"mission")
        marker="original terrain water: plane=1 ordering=1 retry=2 tracks=1 siblings=0 generations=2 resources=0"
        if result.returncode or marker not in result.stdout: raise SystemExit(f"original terrain water failed ({result.returncode}): {result.stdout[-2000:]} {result.stderr[-2000:]}")
    print("original terrain water: active/rollback/reentry ok")
if __name__=="__main__": raise SystemExit(main())
