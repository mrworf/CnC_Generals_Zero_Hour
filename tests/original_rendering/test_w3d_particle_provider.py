#!/usr/bin/env python3
"""Exercise the original particle-provider closure with a generated map."""
import argparse, os, sys
from pathlib import Path
from tempfile import TemporaryDirectory
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map
def main():
    p=argparse.ArgumentParser(); p.add_argument("--source-root",type=Path,required=True); p.add_argument("--executable",type=Path,required=True); a=p.parse_args(); fixture=load_m20_fixture(a.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-particle-provider-") as scratch:
        root=Path(scratch); packet=root/"particle.map"; packet.write_bytes(visual_map(texture_name="Flat")); packet.chmod(0o444)
        os.environ["ZH_M22_PARTICLE_PROFILE"]="1"; os.environ["ZH_M22_PARTICLE_MAP"]=str(packet)
        result=run(a.executable.resolve(),root/"run",source_tree(root/"source",fixture,"valid"),"mission")
        if result.returncode or "original particle provider: owner=1 queue=1 smudge=1 retry=1 generations=2 resources=0" not in result.stdout:
            raise SystemExit(f"original particle provider failed ({result.returncode}): {result.stdout[-2000:]} {result.stderr[-2000:]}")
        os.environ.pop("ZH_M22_PARTICLE_PROFILE")
        os.environ.pop("ZH_M22_PARTICLE_MAP")
        os.environ["ZH_M22_PARTICLE_DEFAULT_PROFILE"]="1"
        default=run(a.executable.resolve(),root/"default",source_tree(root/"default-source",fixture,"valid"),"mission")
        os.environ.pop("ZH_M22_PARTICLE_DEFAULT_PROFILE")
        if default.returncode or "original particle default factory: linux=1" not in default.stdout:
            raise SystemExit(f"original particle default factory failed ({default.returncode}): {default.stdout[-2000:]} {default.stderr[-2000:]}")
    print("original particle provider: source queue/retry/reentry ok")
if __name__ == "__main__": main()
