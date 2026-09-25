#!/usr/bin/env python3
"""Exercise atomic native Bridge/tower/layer admission on generated inputs."""

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


SUCCESS = re.compile(r"original bridge owner: admitted=1 layer=(\d+) towers=(\d+) "
                     r"links=(\d+) objects=(\d+) bridge-id=(\d+) "
                     r"tower-first=(\d+) tower-last=(\d+)")
FAILURE = "original bridge owner: admitted=0 residual=0 drawables=0 list=0 links=0"
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")
MODELED = re.compile(r"original W3D borrowed file owner: active=(\d+) modeled=(\d+)")


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
    with TemporaryDirectory(prefix="zh-m22-bridge-owner-") as scratch:
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
        road_file = source / "Data/INI/Default/Roads.ini"
        fixture.write(road_file, "Bridge OwnedBridge\n"
                      " TowerObjectNameFromLeft = OwnedTower\n"
                      " TowerObjectNameFromRight = OwnedTower\n"
                      " TowerObjectNameToLeft = OwnedTower\n"
                      " TowerObjectNameToRight = OwnedTower\nEnd\n")
        object_file = source / "Data/INI/Default/Object.ini"
        fixture.write(object_file, object_file.read_text(encoding="ascii") +
                      "Object OwnedBridge\n KindOf = STRUCTURE BRIDGE LANDMARK_BRIDGE\n"
                      " IsBridge = Yes\n Geometry = BOX\n"
                      " GeometryMajorRadius = 8\n GeometryMinorRadius = 3\n"
                      " Draw = W3DModelDraw ModuleTag_BridgeDraw\n"
                      "  DefaultConditionState\n   Model = TEST.HLOD\n  End\n End\n"
                      " Body = ActiveBody ModuleTag_BridgeBody\n"
                      "  MaxHealth = 100\n  InitialHealth = 100\n End\n"
                      " Behavior = BridgeBehavior ModuleTag_BridgeBehavior\n End\nEnd\n"
                      "Object OwnedTower\n KindOf = STRUCTURE BRIDGE_TOWER\n"
                      " Geometry = BOX\n GeometryMajorRadius = 2\n GeometryMinorRadius = 2\n"
                      " Body = ActiveBody ModuleTag_TowerBody\n"
                      "  MaxHealth = 100\n  InitialHealth = 100\n End\n"
                      " Behavior = BridgeTowerBehavior ModuleTag_TowerBehavior\n End\nEnd\n")
        model_file = source / "Art/W3D/TEST.w3d"
        model_file.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(args.asset_producer.resolve()), "--emit", str(model_file)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        no_road = base / "no-road-input"
        shutil.copytree(source, no_road)
        fixture.write(no_road / "Data/INI/Default/Roads.ini", "")
        no_tower = base / "no-tower-input"
        shutil.copytree(source, no_tower)
        road = no_tower / "Data/INI/Default/Roads.ini"
        fixture.write(road, road.read_text(encoding="ascii").replace(
            "TowerObjectNameToRight = OwnedTower", "TowerObjectNameToRight = MissingTower"))
        optional_towers = base / "optional-towers-input"
        shutil.copytree(source, optional_towers)
        road = optional_towers / "Data/INI/Default/Roads.ini"
        fixture.write(road, road.read_text(encoding="ascii").replace(
            " TowerObjectNameToLeft = OwnedTower\n", "").replace(
            " TowerObjectNameToRight = OwnedTower\n", ""))
        bad_geometry = base / "bad-geometry-input"
        shutil.copytree(source, bad_geometry)
        obj = bad_geometry / "Data/INI/Default/Object.ini"
        fixture.write(obj, obj.read_text(encoding="ascii").replace(
            "Object OwnedBridge\n KindOf = STRUCTURE BRIDGE LANDMARK_BRIDGE\n"
            " IsBridge = Yes\n Geometry = BOX\n",
            "Object OwnedBridge\n KindOf = STRUCTURE BRIDGE LANDMARK_BRIDGE\n"
            " IsBridge = Yes\n Geometry = SPHERE\n", 1))
        no_bridge_interface = base / "no-bridge-interface-input"
        shutil.copytree(source, no_bridge_interface)
        obj = no_bridge_interface / "Data/INI/Default/Object.ini"
        fixture.write(obj, obj.read_text(encoding="ascii").replace(
            " Behavior = BridgeBehavior ModuleTag_BridgeBehavior\n End\n", "", 1))
        no_tower_interface = base / "no-tower-interface-input"
        shutil.copytree(source, no_tower_interface)
        obj = no_tower_interface / "Data/INI/Default/Object.ini"
        fixture.write(obj, obj.read_text(encoding="ascii").replace(
            " Behavior = BridgeTowerBehavior ModuleTag_TowerBehavior\n End\n", "", 1))
        before = snapshot(source)
        for root in (source, no_road, no_tower, optional_towers, bad_geometry,
                     no_bridge_interface, no_tower_interface):
            fixture.make_read_only(root)
        profile = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M22_W3D_FILE_OWNER_PROFILE": "1",
            "ZH_M22_BRIDGE_OWNER_PROBE": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
            "ZH_M21_SCENARIO": "mission",
        }

        stable_ids = []

        def check(label: str, root: Path, *, towers: int | None, extra: dict | None = None):
            result = run(str(args.executable.resolve()), base / label, root,
                         env_overrides={**profile, **(extra or {})})
            success = SUCCESS.search(result.stderr)
            rollback = ROLLBACK.search(result.stderr)
            modeled = MODELED.search(result.stderr)
            exhaust = bool(extra and "ZH_M22_BRIDGE_EXHAUST_LAYERS" in extra)
            valid = (result.returncode == 3 and
                     "original bridge owner boundary complete" in result.stderr and
                     rollback and rollback.group(3) == "0" and
                     modeled and modeled.groups() ==
                     ("1", str(14 if exhaust else 1 if towers is not None else 0)) and
                     "original recording factory teardown: resources=0" in result.stdout)
            if towers is None:
                valid = valid and FAILURE in result.stderr and not success
                if exhaust:
                    valid = valid and "original bridge owner exhaustion: preexisting=14 intact=14" in result.stderr
            else:
                valid = valid and success and success.groups()[:4] == (
                    "2", str(towers), str(towers), str(towers + 1))
                if valid and root == source:
                    ids = success.groups()[4:]
                    if stable_ids and ids != stable_ids[0]:
                        valid = False
                    stable_ids.append(ids)
            if not valid:
                raise SystemExit(f"bridge owner {label} failed: status={result.returncode} "
                                 f"success={success.groups() if success else 'absent'} "
                                 f"failure={int(FAILURE in result.stderr)} "
                                 f"modeled={modeled.groups() if modeled else 'absent'} "
                                 f"rollback={rollback.groups() if rollback else 'absent'} "
                                 f"boundary={int('original bridge owner boundary complete' in result.stderr)}")

        check("generation-1", source, towers=4)
        check("generation-2", source, towers=4)
        check("optional-towers", optional_towers, towers=2)
        for label, root in (("missing-road", no_road), ("missing-tower", no_tower),
                            ("malformed-geometry", bad_geometry),
                            ("missing-bridge-interface", no_bridge_interface),
                            ("missing-tower-interface", no_tower_interface)):
            check(label, root, towers=None)
            check(f"retry-{label}", source, towers=4)
        for tower in range(1, 5):
            check(f"fault-tower-{tower}", source, towers=None,
                  extra={"ZH_M22_BRIDGE_FAIL_AFTER_TOWER": str(tower)})
            check(f"retry-tower-{tower}", source, towers=4)
        check("fault-layer", source, towers=None,
              extra={"ZH_M22_BRIDGE_FAIL_AFTER_LAYER": "1"})
        check("retry-layer", source, towers=4)
        check("layer-exhaustion", source, towers=None,
              extra={"ZH_M22_BRIDGE_EXHAUST_LAYERS": "1"})
        check("retry-exhaustion", source, towers=4)
        if snapshot(source) != before:
            raise SystemExit("bridge owner changed read-only generated input")
    print("M22 08L3A bridge owner: modeled=1 towers=4 layer=2 failures=11 retry=11")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
