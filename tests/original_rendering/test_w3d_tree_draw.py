#!/usr/bin/env python3
"""Generated source-order immutable tree frame rollback and physical retry."""
import argparse
import os
from pathlib import Path
import subprocess
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture,run
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map

def selected_mesh(packet):
    """Select one exact root mesh chunk, retaining all nested payload bytes."""
    result=[]
    offset=0
    while offset<len(packet):
        if len(packet)-offset<8:
            raise ValueError("generated mesh root truncated")
        kind,size=struct.unpack_from("<II",packet,offset)
        end=offset+8+(size&0x7fffffff)
        if end>len(packet):
            raise ValueError("generated mesh root extent invalid")
        if kind==0:
            header_kind,header_size=struct.unpack_from("<II",packet,offset+8)
            header=packet[offset+16:offset+16+(header_size&0x7fffffff)]
            if (header_kind==0x1f and header[8:24].split(b"\0",1)[0]==b"LITONE01"
                    and header[24:40].split(b"\0",1)[0]==b"TEST"):
                result.append(packet[offset:end])
        offset=end
    if len(result)!=1:
        raise ValueError("generated exact selected tree mesh missing/duplicated")
    return result[0]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--asset-producer",type=Path,required=True)
    parser.add_argument("--source-root",type=Path,required=True)
    parser.add_argument("--gpu",action="store_true")
    args=parser.parse_args()
    fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-tree-draw-") as scratch:
        root=Path(scratch)
        map_path=root/"map.map";map_path.write_bytes(visual_map(texture_name="Flat"));map_path.chmod(0o444)
        packet=root/"tree.w3d"
        subprocess.run([str(args.asset_producer.resolve()),"--emit-rigid-tree",str(packet)],check=True)
        boundary_packet=root/"draw-only.w3d"
        # Produce the minimal packet by exact-byte extraction, not native
        # struct regeneration: unused authored record bytes are preserved too.
        selected=selected_mesh(packet.read_bytes())
        boundary_packet.write_bytes(selected)
        boundary_packet.chmod(0o444)
        if boundary_packet.read_bytes()!=selected:
            raise SystemExit("generated selected tree payload byte identity failed")
        os.environ["ZH_M22_TREE_PREPARATION_PROFILE"]="1"
        os.environ["ZH_M22_TREE_PREPARATION_MAP"]=str(map_path)
        os.environ["ZH_M22_TREE_PREPARATION_ASSET"]=str(packet)
        os.environ["ZH_M22_TREE_DRAW_BOUNDARIES"]="1"
        os.environ["ZH_M22_TREE_DRAW_BOUNDARY_ASSET"]=str(boundary_packet)
        if args.gpu:
            os.environ["ZH_M22_TREE_DRAW_PHYSICAL"]="1"
        for generation in range(2):
            source=source_tree(root/f"source-{generation}",fixture,"valid",tree_textures=True,
                               immobile_enemy=True,crusher_logic=True,fx_lists=True)
            # Final-source Clang sanitizer diagnosis measures 59.32--64.35s
            # for the unchanged 368-boundary sweep. Bound this process at 90s
            # (~40% margin); TimeoutExpired still reports a hung process.
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission",
                       timeout_seconds=90)
            output=result.stdout+result.stderr
            if (result.returncode or "original tree draw: source=1 rollback=1 retry=1 generations=2 resources=0" not in output
                    or "runtime error:" in output or "ERROR: AddressSanitizer" in output
                    or "Validation Error" in output or "VUID-" in output
                    or (args.gpu and "original tree draw physical: source=1 rollback=1 retry=1 generations=2 resources=0" not in output)):
                raise SystemExit(f"original tree draw generation {generation} failed ({result.returncode}); private output redacted")
    print("original tree draw: source-order/rollback/identical-retry ok")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
