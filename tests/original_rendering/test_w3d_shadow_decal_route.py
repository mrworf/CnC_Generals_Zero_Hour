#!/usr/bin/env python3
"""Exercise the generated-map real-owner bounded original decal route."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import tga
from test_w3d_visual_height_map import visual_map

FULL_MARKER = "original full feature map: terrain-tracks-shadow-water-particle-smudge=1 failures=2 generations=2 resources=0"
VOLUME_MARKER = "original volume shadow: source=1 ordering=1 retry=2 tracks-water=1 negatives=1 removal=1 generations=2 resources=0"

def fixture(root: Path, producer: Path) -> None:
    model = root / "Art/W3D/TEST.w3d"; model.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(producer), "--emit", str(model)], check=True)
    obj = root / "Data/INI/Default/Object.ini"
    draw = (" Shadow = SHADOW_DECAL\n ShadowTexture = M22SourceShadow\n ShadowSizeX = 16\n ShadowSizeY = 8\n ShadowOffsetX = 1\n ShadowOffsetY = -2\n Draw = W3DModelDraw ModuleTag_M22Shadow\n  DefaultConditionState\n   Model = TEST.HLOD\n  End\n End\n")
    obj.write_text(obj.read_text().replace("Object LogicFixture\n", "Object LogicFixture\n" + draw, 1))

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True); parser.add_argument("--source-root", type=Path, required=True); parser.add_argument("--asset-producer", type=Path, required=True); parser.add_argument("--volume", action="store_true")
    args = parser.parse_args(); package = load_m20_fixture(args.source_root.resolve()); marker = VOLUME_MARKER if args.volume else FULL_MARKER
    with TemporaryDirectory(prefix="zh-m22-shadow-decal-") as scratch:
        root = Path(scratch); source = root / "source"
        prepare_owned_source(source, package)
        package.write(source / "Data/INI/Default/Terrain.ini", "Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\nTerrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
        package.write(source / "Art/Terrain/Flat.tga", tga("valid")); package.write(source / "Art/Terrain/M22SourceShadow.tga", tga("valid")); fixture(source, args.asset_producer.resolve())
        if args.volume:
            obj=source / "Data/INI/Default/Object.ini"
            obj.write_text(obj.read_text().replace("Shadow = SHADOW_DECAL", "Shadow = SHADOW_VOLUME"))
        packet = root / "shadow.map"; packet.write_bytes(visual_map(texture_name="Flat")); packet.chmod(0o444); package.make_read_only(source)
        if args.volume: os.environ["ZH_M22_VOLUME_SHADOW_PROFILE"] = "1"
        else: os.environ["ZH_M22_FULL_FEATURE_PROFILE"] = "1"
        os.environ["ZH_M22_SHADOW_DECAL_MAP"] = str(packet)
        for generation in range(2):
            result = run(args.executable.resolve(), root / f"run-{generation}", source, "mission")
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original decal route failed ({result.returncode}):\n{result.stdout}{result.stderr}")
        malformed = root / "malformed.map"; malformed.write_bytes(b"not a Generals map"); malformed.chmod(0o444)
        os.environ["ZH_M22_SHADOW_DECAL_MAP"] = str(malformed)
        rejected = run(args.executable.resolve(), root / "malformed", source, "mission")
        if rejected.returncode == 0:
            raise SystemExit("original full-feature malformed map did not fail before frame publication")
    os.environ.pop("ZH_M22_FULL_FEATURE_PROFILE", None); os.environ.pop("ZH_M22_SHADOW_DECAL_MAP", None)
    os.environ.pop("ZH_M22_VOLUME_SHADOW_PROFILE", None)
    print("original full feature map: source ordering/rollback/reentry ok")
    return 0

if __name__ == "__main__": raise SystemExit(main())
