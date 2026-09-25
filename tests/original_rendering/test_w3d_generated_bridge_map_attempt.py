#!/usr/bin/env python3
"""Exercise the native authored bridge/wall map phase with generated sources."""

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


SUCCESS = re.compile(r"original bridge map attempt: admitted=1 bridges=(\d+) walls=(\d+) "
                     r"properties=(\d+) effects=(\d+) order=(\d+) objects=(\d+) "
                     r"prior=(\d+) first-id=(\d+) last-id=(\d+)")
FAILURE = re.compile(r"original bridge map attempt: admitted=0 objects=(-?\d+) "
                     r"drawables=(-?\d+) list=(\d+) bridge=(\d+) wall=(-?\d+) "
                     r"path=(-?\d+) radar=(\d+) prior=(\d+)")
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")


def snapshot(root: Path):
    return tuple(sorted((p.relative_to(root), p.read_bytes())
                        for p in root.rglob("*") if p.is_file()))


def write_variant(fixture, source: Path, destination: Path, relative: str,
                  replacement: str):
    shutil.copytree(source, destination)
    target = destination / relative
    target.chmod(0o644)
    fixture.write(target, replacement)
    fixture.make_read_only(destination)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-bridge-map-") as scratch:
        base = Path(scratch)
        source = base / "owned-input"
        prepare_owned_source(source, fixture)
        fixture.write(source / "Maps/Owned/Owned.map",
                      owned_map(visual=True, bridge_wall=True))
        fixture.write(source / "Maps/Owned/map.ini",
                      "GameData\n MaxTerrainTracks = 2\n PartitionCellSize = 12\nEnd\n")
        fixture.write(source / "Data/INI/Default/Terrain.ini",
                      "Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\n"
                      "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
        fixture.write(source / "Art/Terrain/Flat.tga", tga("valid"))
        data = source / "Data/INI/Default/GameData.ini"
        fixture.write(data, data.read_text(encoding="ascii").replace(
            "END\n", " UseWaterPlane = Yes\n UseCloudPlane = Yes\n"
            " WaterExtentX = 8\n WaterExtentY = 8\n WaterType = 0\n"
            " UseShadowVolumes = Yes\n MaxTerrainTracks = 2\nEND\n", 1))
        road_text = ("Bridge OwnedBridge\n TowerObjectNameFromLeft = OwnedTower\n"
                     " TowerObjectNameToRight = OwnedTower\nEnd\n")
        fixture.write(source / "Data/INI/Default/Roads.ini", road_text)
        object_file = source / "Data/INI/Default/Object.ini"
        object_text = (object_file.read_text(encoding="ascii") +
                       "Object OwnedBridge\n KindOf = STRUCTURE BRIDGE LANDMARK_BRIDGE\n"
                       " IsBridge = Yes\n Geometry = BOX\n"
                       " GeometryMajorRadius = 8\n GeometryMinorRadius = 3\n"
                       " Draw = W3DModelDraw ModuleTag_BridgeDraw\n"
                       "  DefaultConditionState\n   Model = TEST.HLOD\n  End\n End\n"
                       " Body = ActiveBody ModuleTag_BridgeBody\n"
                       "  MaxHealth = 100\n  InitialHealth = 100\n End\n"
                       " Behavior = BridgeBehavior ModuleTag_BridgeBehavior\n End\nEnd\n"
                       "Object OwnedTower\n KindOf = STRUCTURE BRIDGE_TOWER\n"
                       " Geometry = BOX\n GeometryMajorRadius = 2\n"
                       " GeometryMinorRadius = 2\n"
                       " Body = ActiveBody ModuleTag_TowerBody\n"
                       "  MaxHealth = 100\n  InitialHealth = 100\n End\n"
                       " Behavior = BridgeTowerBehavior ModuleTag_TowerBehavior\n End\nEnd\n"
                       "Object OwnedWall\n KindOf = STRUCTURE WALK_ON_TOP_OF_WALL\n"
                       " Geometry = BOX\n GeometryMajorRadius = 3\n"
                       " GeometryMinorRadius = 3\n"
                       " Body = ActiveBody ModuleTag_WallBody\n"
                       "  MaxHealth = 100\n  InitialHealth = 100\n End\nEnd\n")
        fixture.write(object_file, object_text)
        model = source / "Art/W3D/TEST.w3d"
        model.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(args.asset_producer.resolve()), "--emit", str(model)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        skirmish = base / "skirmish-input"
        shutil.copytree(source, skirmish)
        fixture.write(skirmish / "Maps/Owned/Owned.map",
                      owned_map(visual=True, skirmish=True, bridge_wall=True))
        unsupported = base / "unsupported-property-input"
        shutil.copytree(source, unsupported)
        fixture.write(unsupported / "Maps/Owned/Owned.map",
                      owned_map(visual=True, bridge_wall=True,
                                unsupported_bridge_property=True))
        fixture.make_read_only(unsupported)
        missing_road = base / "missing-road-input"
        write_variant(fixture, source, missing_road, "Data/INI/Default/Roads.ini", "")
        missing_tower = base / "missing-tower-input"
        write_variant(fixture, source, missing_tower, "Data/INI/Default/Roads.ini",
                      road_text.replace("TowerObjectNameToRight = OwnedTower",
                                        "TowerObjectNameToRight = MissingTower"))
        missing_bridge_behavior = base / "missing-bridge-behavior-input"
        write_variant(fixture, source, missing_bridge_behavior,
                      "Data/INI/Default/Object.ini",
                      object_text.replace(" Behavior = BridgeBehavior ModuleTag_BridgeBehavior\n End\n", "", 1))
        missing_wall_body = base / "missing-wall-body-input"
        write_variant(fixture, source, missing_wall_body,
                      "Data/INI/Default/Object.ini",
                      object_text.replace(" Body = ActiveBody ModuleTag_WallBody\n"
                                          "  MaxHealth = 100\n  InitialHealth = 100\n End\n", "", 1))
        missing_wall = base / "missing-wall-template-input"
        write_variant(fixture, source, missing_wall,
                      "Data/INI/Default/Object.ini",
                      object_text.split("Object OwnedWall\n")[0])
        before = snapshot(source)
        fixture.make_read_only(source)
        fixture.make_read_only(skirmish)
        profile = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M22_W3D_FILE_OWNER_PROFILE": "1",
            "ZH_M22_BRIDGE_MAP_PROBE": "1",
            "ZH_M22_BRIDGE_MAP_PREEXISTING": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
            "ZH_M21_SCENARIO": "mission",
        }
        stable_ids = []

        def check(label: str, root: Path, *, success: bool, extra: dict | None = None):
            environment = {**profile, **(extra or {})}
            if environment.get("ZH_M22_BRIDGE_MAP_PREEXISTING") == "":
                environment.pop("ZH_M22_BRIDGE_MAP_PREEXISTING")
            result = run(str(args.executable.resolve()), base / label, root,
                         env_overrides=environment)
            positive = SUCCESS.search(result.stderr)
            failure = FAILURE.search(result.stderr)
            rollback = ROLLBACK.search(result.stderr)
            valid = (result.returncode == 3 and
                     "original bridge map attempt boundary complete" in result.stderr and
                     rollback and rollback.group(3) == "0" and
                     "original recording factory teardown: resources=0" in result.stdout)
            if success:
                valid = valid and positive and positive.groups()[:7] == (
                    "1", "1", "2", "2", "1", "4", "1") and not failure
                if valid:
                    ids = positive.groups()[7:]
                    if stable_ids and ids != stable_ids[0]:
                        valid = False
                    stable_ids.append(ids)
            else:
                valid = valid and failure and failure.groups() == (
                    "0", "0", "0", "0", "0", "0", "0", "1") and not positive
            if not valid:
                raise SystemExit(f"bridge map {label} failed: status={result.returncode} "
                                 f"positive={positive.groups() if positive else 'absent'} "
                                 f"failure={failure.groups() if failure else 'absent'} "
                                 f"rollback={rollback.groups() if rollback else 'absent'}")

        check("mission-1", source, success=True)
        check("mission-2", source, success=True)
        check("skirmish", skirmish, success=True,
              extra={"ZH_M21_SCENARIO": "skirmish"})
        for fault in ("object:1", "position:1", "bridge:1", "property:1",
                      "object:2", "position:2", "wall:2", "property:2",
                      "radar", "pathfinder"):
            check("fault-" + fault.replace(":", "-"), source, success=False,
                  extra={"ZH_M22_BRIDGE_MAP_FAIL_AT": fault})
        check("duplicate", source, success=False,
              extra={"ZH_M22_BRIDGE_MAP_DUPLICATE": "1"})
        check("unsupported-property", unsupported, success=False)
        for label, root in (("missing-road", missing_road),
                            ("missing-tower", missing_tower),
                            ("missing-bridge-behavior", missing_bridge_behavior),
                            ("missing-wall-body", missing_wall_body),
                            ("missing-wall-template", missing_wall)):
            extra = ({"ZH_M22_BRIDGE_MAP_PREEXISTING": ""}
                     if label in ("missing-road", "missing-tower",
                                  "missing-bridge-behavior") else {})
            check(label, root, success=False, extra=extra)
        check("mission-retry", source, success=True)
        if snapshot(source) != before:
            raise SystemExit("bridge map attempt changed read-only generated input")
    print("M22 08L3B bridge map attempt: mission+skirmish+retry=4 failures=17 pre-teardown=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
