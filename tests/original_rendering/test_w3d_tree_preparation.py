#!/usr/bin/env python3
"""Generated public display-route immutable tree preparation and retry."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture,run
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--asset-producer",type=Path,required=True)
    parser.add_argument("--source-root",type=Path,required=True)
    args=parser.parse_args()
    fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-tree-preparation-") as scratch:
        root=Path(scratch)
        map_path=root/"map.map";map_path.write_bytes(visual_map(texture_name="Flat"));map_path.chmod(0o444)
        packet=root/"tree.w3d"
        subprocess.run([str(args.asset_producer.resolve()),"--emit-rigid-tree",str(packet)],check=True)
        os.environ["ZH_M22_TREE_PREPARATION_PROFILE"]="1"
        os.environ["ZH_M22_TREE_PREPARATION_MAP"]=str(map_path)
        os.environ["ZH_M22_TREE_PREPARATION_ASSET"]=str(packet)
        for generation in range(2):
            source=source_tree(root/f"source-{generation}",fixture,"valid",tree_textures=True,
                               immobile_enemy=True,crusher_logic=True,fx_lists=True)
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission")
            output=result.stdout+result.stderr
            if (result.returncode or "original tree preparation: immutable=1 retry=1 generations=2 resources=0" not in output
                    or "runtime error:" in output or "ERROR: AddressSanitizer" in output):
                raise SystemExit(f"original tree preparation generation {generation} failed ({result.returncode}); private output redacted")
    print("original tree preparation: immutable/public-route/retry ok")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
