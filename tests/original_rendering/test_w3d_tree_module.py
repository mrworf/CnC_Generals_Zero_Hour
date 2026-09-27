#!/usr/bin/env python3
"""Generated exact tree-module factory/transform/removal boundary."""
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
from test_w3d_tree_draw import selected_mesh

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--asset-producer",type=Path,required=True)
    parser.add_argument("--source-root",type=Path,required=True)
    parser.add_argument("--gpu",action="store_true")
    args=parser.parse_args();fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-tree-module-") as scratch:
        root=Path(scratch);map_path=root/"map.map";map_path.write_bytes(visual_map(texture_name="Flat"));map_path.chmod(0o444)
        packet=root/"tree.w3d"
        subprocess.run([str(args.asset_producer.resolve()),"--emit-rigid-tree",str(packet)],check=True)
        packet.write_bytes(selected_mesh(packet.read_bytes()));packet.chmod(0o444)
        os.environ["ZH_M22_TREE_MODULE_PROFILE"]="1"
        os.environ["ZH_M22_TREE_PREPARATION_MAP"]=str(map_path)
        os.environ["ZH_M22_TREE_PREPARATION_ASSET"]=str(packet)
        if args.gpu:os.environ["ZH_M22_TREE_MODULE_PHYSICAL"]="1"
        for generation in range(2):
            source=source_tree(root/f"source-{generation}",fixture,"valid",tree_textures=True,
                               immobile_enemy=True,tree_modules=True,tree_decals=args.gpu)
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission",timeout_seconds=90)
            output=result.stdout+result.stderr
            if (result.returncode or "original tree module: layout=1 transform=1 preflight=1 retry=1 generations=2 resources=0" not in output
                or "runtime error:" in output or "ERROR: AddressSanitizer" in output
                or "Validation Error" in output or "VUID-" in output
                or (args.gpu and "original tree module physical: factory=1 tree=1 decal=1 rollback=1 retry=1 resources=0" not in output)):
                raise SystemExit(f"original tree module generation {generation} failed ({result.returncode}); generated output redacted")
    print("original tree module: source/rollback/retry ok")
    return 0

if __name__=="__main__":raise SystemExit(main())
