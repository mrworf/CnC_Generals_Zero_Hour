#!/usr/bin/env python3
"""Generated source terrain-prop frame admission, rollback and physical retry."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--gpu", action="store_true")
    args=parser.parse_args()
    fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-prop-frame-") as scratch:
        root=Path(scratch)
        map_path=root/"map.map"
        map_path.write_bytes(visual_map(texture_name="Flat"));map_path.chmod(0o444)
        packet=root/"prop.w3d"
        subprocess.run([str(args.asset_producer.resolve()),"--emit-prop-frame",str(packet)],check=True)
        packet.chmod(0o444)
        os.environ["ZH_M22_PROP_FRAME_PROFILE"]="1"
        os.environ["ZH_M22_PROP_FRAME_MAP"]=str(map_path)
        os.environ["ZH_M22_PROP_FRAME_ASSET"]=str(packet)
        if args.gpu: os.environ["ZH_M22_PROP_FRAME_PHYSICAL"]="1"
        for generation in range(2):
            source=source_tree(root/f"source-{generation}",fixture,"valid",tree_textures=True,tree_decals=True)
            # This wrapper owns the generated input tree; finish authored
            # fixture additions before restoring its read-only input contract.
            for path in source.rglob("*"):
                path.chmod(0o755 if path.is_dir() else 0o644)
            fixture.write(source/"Art/Textures/MYTEX.TGA",(source/"Art/Terrain/Tree0.tga").read_bytes())
            objects=source/"Data/INI/Default/Object.ini"
            fixture.write(objects,objects.read_text()+
                "Object FramePropFixture\n Scale = 8\n KindOf = PROP\n"
                " Draw = W3DModelDraw ModuleTag_Prop\n DefaultConditionState\n Model = TEST.LITONE01\n End\n End\nEnd\n"+
                "".join(f"Object Frame{kind}Fixture\n Scale = 8\n KindOf = PROP\n"
                    f" Draw = W3DModelDraw ModuleTag_Prop\n DefaultConditionState\n Model = {name}\n End\n End\nEnd\n"
                    for kind,name in (("Static","TEST.STATIC01"),("Skin","TEST.SKINHLOD"),("Hlod","TEST.HLOD"),
                        ("Hmodel","PROP_HMODEL"),("Collection","PROP_COLLECTION"),("Legacy","PROP_LEGACY"))))
            fixture.make_read_only(source)
            # 1382 props-only + 1386 combined fresh-generation operation
            # ordinals measured 137.385/137.275 s on isolated native GCC;
            # corrected GCC/Clang sanitizer processes took 565.788/566.341
            # and 492.544/492.894 s. Keep TimeoutExpired and ~32% headroom
            # over the slower sanitizer, without reducing the workload.
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission",timeout_seconds=750)
            output=result.stdout+result.stderr
            if (result.returncode or "original prop frame: source=1 rollback=1 retry=1 generations=2 resources=0" not in output
                or "runtime error:" in output or "ERROR: AddressSanitizer" in output
                or "Validation Error" in output or "VUID-" in output
                or (args.gpu and "original prop frame physical:" not in output)):
                raise SystemExit(f"original prop frame generation {generation} failed ({result.returncode}); output redacted")
    print("original prop frame: source/rollback/identical-retry ok")
    return 0

if __name__=="__main__":
    raise SystemExit(main())
