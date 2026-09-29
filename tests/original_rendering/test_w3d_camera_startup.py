#!/usr/bin/env python3
"""Exercise the bounded native camera-startup owner on generated map data."""

import argparse
import os
import subprocess
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, run
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import run as bootstrap_run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import source_tree
from test_w3d_visual_height_map import visual_map


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--gpu", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-camera-startup-") as scratch:
        root = Path(scratch)
        map_path = root / "map.map"
        map_path.write_bytes(visual_map(texture_name="Flat"))
        map_path.chmod(0o444)
        packet = root / "prop.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-prop-frame", str(packet)], check=True)
        packet.chmod(0o444)
        os.environ["ZH_M22_CAMERA_STARTUP_PROFILE"] = "1"
        os.environ["ZH_M22_CAMERA_STARTUP_MAP"] = str(map_path)
        os.environ["ZH_M22_CAMERA_STARTUP_ASSET"] = str(packet)
        if args.gpu:
            os.environ["ZH_M22_CAMERA_STARTUP_PHYSICAL"] = "1"
        marker = "original camera startup: ground-default=1 stationary=1 retry=1 generations=2 resources=0"
        for generation in range(2):
            source = source_tree(root / f"source-{generation}", fixture, "valid", tree_textures=True, tree_decals=True)
            for path in source.rglob("*"):
                path.chmod(0o755 if path.is_dir() else 0o644)
            fixture.write(source / "Art/Textures/MYTEX.TGA", (source / "Art/Terrain/Tree0.tga").read_bytes())
            objects = source / "Data/INI/Default/Object.ini"
            fixture.write(objects, objects.read_text()+
                          "Object CameraPropFixture\n Scale = 8\n KindOf = PROP\n"
                          " Draw = W3DModelDraw ModuleTag_Prop\n DefaultConditionState\n"
                          " Model = TEST.LITONE01\n End\n End\nEnd\n")
            game_data = source / "Data/INI/Default/GameData.ini"
            text = game_data.read_text()
            assert text.count("END\n") == 1
            fixture.write(game_data, text.replace("END\n", " CameraHeight = 180\n CameraPitch = 37.5\n"
                                                 " CameraYaw = 0\n PartitionCellSize = 10\nEND\n"))
            fixture.make_read_only(source)
            bootstrap = bootstrap_run(str(args.executable.resolve()), root / f"bootstrap-{generation}", source,
                                      env_overrides={"ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
                                                     "ZH_M22_RECORDING_FACTORY_PROFILE": "1"})
            bootstrap_output = bootstrap.stdout + bootstrap.stderr
            if (bootstrap.returncode or "original camera deferred bootstrap:" not in bootstrap_output
                    or "runtime error:" in bootstrap_output or "ERROR: AddressSanitizer" in bootstrap_output
                    or "Validation Error" in bootstrap_output or "VUID-" in bootstrap_output):
                raise SystemExit(f"original camera deferred bootstrap generation {generation} failed "
                                 f"({bootstrap.returncode}); private output redacted")
            result = run(args.executable.resolve(), root / f"generation-{generation}", source, "mission")
            output = result.stdout + result.stderr
            if (result.returncode or marker not in output or "runtime error:" in output
                    or "ERROR: AddressSanitizer" in output or "Validation Error" in output or "VUID-" in output
                    or (args.gpu and "original camera startup physical:" not in output)):
                raise SystemExit(f"original camera startup generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original camera startup: ground/default/stationary rollback/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
