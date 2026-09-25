#!/usr/bin/env python3
"""Exercise fresh pathfinder map rollback with generated bridge/wall owners."""

import argparse
from pathlib import Path
import re
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, owned_map, prepare_owned_source
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import run
sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import tga


OWNER = re.compile(r"original pathfinder owner: layers=(\d+),(\d+) walls=(\d+) "
                   r"null=(\d+) duplicate=(\d+) nonmember=(\d+) "
                   r"removal=(\d+) capacity=(\d+) "
                   r"first=(\d+) residual=(\d+) intact=(\d+) retry=(\d+)")
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")


def snapshot(root: Path):
    return tuple(sorted((p.relative_to(root), p.read_bytes())
                        for p in root.rglob("*") if p.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-pathfinder-owner-") as scratch:
        base = Path(scratch)
        source = base / "owned-input"
        prepare_owned_source(source, fixture)
        fixture.write(source / "Maps/Owned/Owned.map", owned_map(visual=True))
        fixture.write(source / "Maps/Owned/map.ini",
                      "GameData\n MaxTerrainTracks = 2\n PartitionCellSize = 12\nEnd\n")
        fixture.write(source / "Data/INI/Default/Terrain.ini",
                      "Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\n"
                      "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
        fixture.write(source / "Art/Terrain/Flat.tga", tga("valid"))
        game_data = source / "Data/INI/Default/GameData.ini"
        fixture.write(game_data, game_data.read_text(encoding="ascii").replace(
            "END\n", " UseWaterPlane = Yes\n UseCloudPlane = Yes\n"
            " WaterExtentX = 8\n WaterExtentY = 8\n WaterType = 0\n"
            " UseShadowVolumes = Yes\n MaxTerrainTracks = 2\nEND\n", 1))
        fixture.write(source / "Data/INI/Default/Roads.ini", "Bridge OwnedBridge\nEnd\n")
        obj = source / "Data/INI/Default/Object.ini"
        fixture.write(obj, obj.read_text(encoding="ascii") +
                      "Object OwnedBridge\n KindOf = STRUCTURE BRIDGE LANDMARK_BRIDGE\n"
                      " IsBridge = Yes\n Geometry = BOX\n"
                      " GeometryMajorRadius = 8\n GeometryMinorRadius = 3\n"
                      " Draw = W3DModelDraw ModuleTag_BridgeDraw\n"
                      "  DefaultConditionState\n   Model = TEST.HLOD\n  End\n End\n"
                      " Body = ActiveBody ModuleTag_BridgeBody\n"
                      "  MaxHealth = 100\n  InitialHealth = 100\n End\n"
                      " Behavior = BridgeBehavior ModuleTag_BridgeBehavior\n End\nEnd\n"
                      "Object OwnedWall\n KindOf = STRUCTURE WALK_ON_TOP_OF_WALL\n"
                      " Geometry = BOX\n GeometryMajorRadius = 3\n"
                      " GeometryMinorRadius = 3\n"
                      " Body = ActiveBody ModuleTag_WallBody\n"
                      "  MaxHealth = 100\n  InitialHealth = 100\n End\nEnd\n")
        model = source / "Art/W3D/TEST.w3d"
        model.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(args.asset_producer.resolve()), "--emit", str(model)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        before = snapshot(source)
        fixture.make_read_only(source)
        missing = base / "missing-wall-input"
        shutil.copytree(source, missing)
        wall_ini = missing / "Data/INI/Default/Object.ini"
        wall_ini.chmod(0o644)
        wall_ini.write_text(wall_ini.read_text(encoding="ascii").split("Object OwnedWall\n")[0],
                            encoding="ascii")
        fixture.make_read_only(missing)
        profile = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M22_W3D_FILE_OWNER_PROFILE": "1",
            "ZH_M22_PATHFINDER_OWNER_PROBE": "1",
            "ZH_M22_PATHFINDER_FAIL_ONCE": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
            "ZH_M21_SCENARIO": "mission",
        }
        for index, fault in enumerate((None, "zone", "ground", "rows", "layer:2",
                                       "layer:3", "wall", "classify", "ready", None)):
            extra = {"ZH_M22_PATHFINDER_FAIL_AT": fault} if fault else {}
            result = run(str(args.executable.resolve()), base / f"attempt-{index}",
                         source, env_overrides={**profile, **extra})
            owner = OWNER.search(result.stderr)
            rollback = ROLLBACK.search(result.stderr)
            expected_first = "1" if fault is None else "0"
            if not (result.returncode == 3 and owner and owner.groups() ==
                    ("2", "3", "127", "1", "1", "1", "1", "1",
                     expected_first, "0", "1", "1") and
                    "original pathfinder owner boundary complete" in result.stderr and
                    rollback and rollback.group(3) == "0" and
                    "original recording factory teardown: resources=0" in result.stdout):
                raise SystemExit(f"pathfinder owner fault={fault or 'none'} "
                                 f"status={result.returncode} "
                                 f"owner={owner.groups() if owner else 'absent'} "
                                 f"rollback={rollback.groups() if rollback else 'absent'}")
        stale = run(str(args.executable.resolve()), base / "stale-wall", source,
                    env_overrides={**profile, "ZH_M22_PATHFINDER_STALE_WALL": "1"})
        stale_marker = "original pathfinder stale wall: rejected=1 residual=0 intact=1"
        if not (stale.returncode == 3 and stale_marker in stale.stderr and
                "original pathfinder stale wall boundary complete" in stale.stderr and
                "original recording factory teardown: resources=0" in stale.stdout):
            raise SystemExit("stale registered wall was not rejected before newMap")
        missing_result = run(str(args.executable.resolve()), base / "missing-wall",
                             missing, env_overrides=profile)
        if not (missing_result.returncode == 3 and not OWNER.search(missing_result.stderr) and
                "generated pathfinder owner template missing" in missing_result.stderr and
                "original recording factory teardown: resources=0" in missing_result.stdout):
            raise SystemExit("missing pathfinder wall provider was not rejected")
        if snapshot(source) != before:
            raise SystemExit("pathfinder owner changed read-only generated input")
    print("M22 08L3B0 pathfinder owner: wall-capacity=127 faults=8 retry=8 generations=2")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
