#!/usr/bin/env python3
"""Generated native display descriptor preload and temporary Drawable ownership."""
import argparse
from pathlib import Path
import os
import struct
import subprocess
import re
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture,prepare_owned_source,run
from test_w3d_tree_draw import selected_mesh
from test_w3d_terrain_source_bitmap import tile_tga

MARKER="original asset preload: source=1 lazy=1 temporary=1 retry=1 generations=2 resources=0"

def chunk(kind,payload):
    return struct.pack("<II",kind,len(payload))+payload

def box(name):
    # Public W3dBoxStruct: version/name/color/center/extent, exact fixed-width transport.
    payload=struct.pack("<II32sI6f",0x10000,0,name.encode("ascii"),0xffffffff,0,0,0,1,2,3)
    return chunk(0x740,payload)

def mip_enabled(packet):
    """Author only the public TextureInfo NO_LOD bit for the regular consumer."""
    result=bytearray(packet);changed=[]
    def visit(first,end,depth=0):
        assert depth<64
        while first<end:
            kind,size=struct.unpack_from("<II",result,first)
            stop=first+8+(size&0x7fffffff);assert stop<=end
            if kind==0x33:
                attributes,=struct.unpack_from("<H",result,first+8)
                assert attributes&4
                struct.pack_into("<H",result,first+8,attributes&~4);changed.append(first+8)
            if size&0x80000000:visit(first+8,stop,depth+1)
            first=stop
        assert first==end
    visit(0,len(result));assert len(changed)==1
    assert all(a==b or (i==changed[0] and a^b==4) for i,(a,b) in enumerate(zip(packet,result)))
    return bytes(result)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root",type=Path,required=True)
    parser.add_argument("--executable",type=Path,required=True)
    parser.add_argument("--asset-producer",type=Path,required=True)
    parser.add_argument("--gpu",action="store_true")
    args=parser.parse_args();fixture=load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-asset-preload-") as scratch:
        root=Path(scratch);source=root/"source";prepare_owned_source(source,fixture)
        obj=source/"Data/INI/Default/Object.ini"
        fixture.write(obj,obj.read_text(encoding="ascii")+
            "Object PreloadTemporaryFixture\n Scale = 1\n KindOf = DRAWABLE_ONLY PRELOAD\n"
            " Draw = W3DModelDraw ModuleTag_Preload\n"
            "  DefaultConditionState\n   Model = NONE\n  End\n"
            "  ConditionState = DAMAGED\n   Model = PreloadTemporary\n  End\n End\nEnd\n"
            "Object PreloadLoadedFixture\n Scale = 1\n KindOf = DRAWABLE_ONLY\n"
            " Draw = W3DModelDraw ModuleTag_Loaded\n"
            "  DefaultConditionState\n   Model = NONE\n  End\n"
            "  ConditionState = DAMAGED\n   Model = PreloadLoaded\n  End\n End\nEnd\n")
        fixture.write(source/"Data/INI/ParticleSystem.ini",
            "ParticleSystem PreloadParticleFixture\n Type = PARTICLE\n ParticleName = PreloadParticle.tga\nEnd\n")
        fixture.write(source/"Art/W3D/PreloadDirect.w3d",box("PRELOAD.DIRECT"))
        fixture.write(source/"Art/W3D/PreloadQualified.w3d.w3d",box("PRELOAD.QUALIFIED"))
        fixture.write(source/"Art/W3D/PreloadTemporary.w3d",box("PRELOAD.TEMPORARY"))
        fixture.write(source/"Art/W3D/PreloadUnsupported.w3d",chunk(0x7ffffffe,b""))
        fixture.write(source/"Art/W3D/PreloadLoaded.w3d",box("PRELOAD.LOADED"))
        fixture.write(source/"Art/W3D/PreloadDebris.w3d",box("PRELOAD.DEBRIS"))
        for ordinal in range(256):
            fixture.write(source/f"Art/W3D/PreloadFault{ordinal}.w3d",box(f"PRELOAD.FAULT{ordinal}"))
        if args.gpu:
            packet=root/"physical.w3d"
            subprocess.run([str(args.asset_producer.resolve()),"--emit-rigid-tree",str(packet)],check=True)
            fixture.write(source/"Art/W3D/PreloadPhysical.w3d",mip_enabled(selected_mesh(packet.read_bytes())))
            fixture.write(source/"Art/Textures/mytex.tga",tile_tga(1,((32,128,224,255),)))
            os.environ["ZH_M22_ASSET_PRELOAD_PHYSICAL"]="1"
        fixture.make_read_only(source)
        os.environ["ZH_M22_ASSET_PRELOAD_PROFILE"]="1"
        counts=[]
        for generation in range(2):
            result=run(args.executable.resolve(),root/f"generation-{generation}",source,"mission",timeout_seconds=120)
            output=result.stdout+result.stderr
            boundary=re.findall(r"original asset preload boundaries: rejected=(\d+) retry=1",output)
            if (result.returncode or MARKER not in output or
                len(boundary)!=1 or not 0<int(boundary[0])<4096 or
                (args.gpu and "original asset preload physical: lazy=1 identity=1 pixels=1 retry=1 generations=2 resources=0" not in output) or
                any(category in output for category in ("runtime error:","AddressSanitizer","LeakSanitizer",
                                                       "UndefinedBehaviorSanitizer","Validation Error","VUID-"))):
                raise SystemExit(f"asset preload generation {generation} failed ({result.returncode}); generated output redacted")
            counts.append(int(boundary[0]))
        if counts[0]!=counts[1]:raise SystemExit("asset preload equivalent generation boundary counts differ")
        print(f"original asset preload boundaries: rejected={counts[0]} retry=1 generations=2")
    print("original asset preload: native descriptor/temporary/rollback/retry ok")
    return 0

if __name__=="__main__":raise SystemExit(main())
