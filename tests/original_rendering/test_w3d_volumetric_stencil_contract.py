#!/usr/bin/env python3
import argparse, os, sys
from pathlib import Path
from tempfile import TemporaryDirectory
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"original_simulation"))
from test_scenario_setup import load_m20_fixture,run
sys.path.insert(0,str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
p=argparse.ArgumentParser(); p.add_argument("--executable",type=Path,required=True); p.add_argument("--source-root",type=Path,required=True); a=p.parse_args()
with TemporaryDirectory(prefix="zh-m22-volume-stencil-") as temp:
    root=Path(temp); os.environ["ZH_M22_VOLUME_STENCIL_PROFILE"]="1"
    r=run(a.executable.resolve(),root/"run",source_tree(root/"source",load_m20_fixture(a.source_root.resolve()),"valid"),"mission")
    if r.returncode or "original volume stencil: public-state=1 retries=1 generations=2 resources=0" not in r.stdout: raise SystemExit(r.stdout[-2000:]+r.stderr[-2000:])
os.environ.pop("ZH_M22_VOLUME_STENCIL_PROFILE",None)
