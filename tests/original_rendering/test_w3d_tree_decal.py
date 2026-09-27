#!/usr/bin/env python3
"""Generated object-less source projected tree decal transaction."""
import argparse
import os
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture,run
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map, chunk, ascii_string
from test_w3d_tree_draw import selected_mesh

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--asset-producer",type=Path,required=True)
    parser.add_argument("--source-root",type=Path,required=True)
    parser.add_argument("--gpu",action="store_true")
    args=parser.parse_args()
    fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-tree-decal-") as scratch:
        root=Path(scratch);map_path=root/"map.map";map_path.write_bytes(visual_map(texture_name="Flat"));map_path.chmod(0o444)
        extent=112;cells=extent*extent
        large=bytearray(b"CkMp"+struct.pack("<I",2))
        for ordinal,name in enumerate(("HeightMapData","BlendTileData"),1):
            encoded=name.encode("ascii");large.extend(struct.pack("<B",len(encoded))+encoded+struct.pack("<I",ordinal))
        height=struct.pack("<7i",extent,extent,2,1,extent-4,extent-4,cells)+bytes(i%64 for i in range(cells))
        blend=struct.pack("<i",cells)+bytes(cells*8+cells//8)+struct.pack("<4i",1,1,1,1)
        blend+=struct.pack("<4i",0,1,1,0)+ascii_string("Flat")+struct.pack("<2i",0,0)
        large.extend(chunk(1,4,height));large.extend(chunk(2,8,blend))
        large_path=root/"large.map";large_path.write_bytes(large);large_path.chmod(0o444)
        packet=root/"tree.w3d"
        subprocess.run([str(args.asset_producer.resolve()),"--emit-rigid-tree",str(packet)],check=True)
        packet.write_bytes(selected_mesh(packet.read_bytes()));packet.chmod(0o444)
        os.environ["ZH_M22_TREE_PREPARATION_PROFILE"]="1"
        os.environ["ZH_M22_TREE_PREPARATION_MAP"]=str(map_path)
        os.environ["ZH_M22_TREE_PREPARATION_ASSET"]=str(packet)
        os.environ["ZH_M22_TREE_DECAL_PROFILE"]="1"
        os.environ["ZH_M22_TREE_DECAL_LARGE_MAP"]=str(large_path)
        if args.gpu:
            os.environ["ZH_M22_TREE_DECAL_PHYSICAL"]="1"
        for generation in range(2):
            source=source_tree(root/f"source-{generation}",fixture,"valid",tree_textures=True,
                               immobile_enemy=True,crusher_logic=True,fx_lists=True,tree_decals=True)
            # Isolated GCC sanitizer workload is 53.62s;90 retains bounded
            # TimeoutExpired failure while allowing host/validation variance.
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission",timeout_seconds=90)
            output=result.stdout+result.stderr
            if (result.returncode or "original tree decal: source=1 rollback=1 retry=1 generations=2 resources=0" not in output
                or "runtime error:" in output or "ERROR: AddressSanitizer" in output
                or "Validation Error" in output or "VUID-" in output
                or (args.gpu and "original tree decal physical:" not in output)):
                raise SystemExit(f"original tree decal generation {generation} failed ({result.returncode}); private output redacted")
        os.environ["ZH_M22_TREE_DECAL_PROVIDER_NEGATIVE"]="1"
        os.environ.pop("ZH_M22_TREE_DECAL_PHYSICAL",None)
        for category in ("missing","unsupported","truncated"):
            source=source_tree(root/f"source-{category}",fixture,"valid",tree_textures=True,
                               immobile_enemy=True,crusher_logic=True,fx_lists=True,tree_decals=category)
            result=run(args.executable.resolve(),root/f"generation-{category}",source,"mission",timeout_seconds=90)
            output=result.stdout+result.stderr
            if (result.returncode or "original tree decal: source=1 rollback=1 retry=1 generations=2 resources=0" not in output
                or "runtime error:" in output or "ERROR: AddressSanitizer" in output):
                raise SystemExit(f"original tree decal provider {category} failed ({result.returncode}); private output redacted")
    print("original tree decal: source/rollback/identical-retry ok")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
