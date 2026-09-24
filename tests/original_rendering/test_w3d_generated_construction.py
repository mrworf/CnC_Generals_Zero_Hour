#!/usr/bin/env python3
"""Exercise the generated-only post-map source construction boundary."""

import argparse
from pathlib import Path
import re
import shutil
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, owned_map, prepare_owned_source

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import run

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import tga


COMPLETE = "original generated construction boundary complete"
STAGES = ("terrain", "radar", "shroud", "terrain-logic", "radar-terrain",
          "pathfinder", "observer", "objects")
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")
TEARDOWN = "original recording factory teardown: resources=0"


def snapshot(root: Path):
    return tuple(sorted((path.relative_to(root), path.read_bytes())
                        for path in root.rglob("*") if path.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-generated-construction-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.write(source / "Maps/Owned/Owned.map", owned_map(visual=True))
        fixture.write(source / "Maps/Owned/map.ini",
                      "GameData\n MaxTerrainTracks = 2\n PartitionCellSize = 12\nEnd\n")
        fixture.write(source / "Data/INI/Default/Terrain.ini",
                      "Terrain DefaultTerrain\n Texture = Flat.tga\n Class = NONE\nEnd\n"
                      "Terrain Flat\n Texture = Flat.tga\n Class = NONE\nEnd\n")
        fixture.write(source / "Art/Terrain/Flat.tga", tga("valid"))
        game_data = source / "Data/INI/Default/GameData.ini"
        current = game_data.read_text(encoding="ascii")
        game_data.write_text(current.replace(
            "END\n", " UseWaterPlane = Yes\n UseCloudPlane = Yes\n"
            " WaterExtentX = 8\n WaterExtentY = 8\n WaterType = 0\n"
            " UseShadowVolumes = Yes\n MaxTerrainTracks = 2\nEND\n", 1),
            encoding="ascii")
        missing = base / "missing-provider"
        shutil.copytree(source, missing)
        (missing / "Art/Terrain/Flat.tga").unlink()
        before = snapshot(source)
        fixture.make_read_only(source)
        fixture.make_read_only(missing)
        common = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        }
        for mode in ("mission", "skirmish"):
            selected = {**common, "ZH_M21_SCENARIO": mode}
            for generation in range(2):
                result = run(str(args.executable.resolve()),
                             base / f"{mode}-{generation}", source,
                             env_overrides=selected)
                output = result.stdout + result.stderr
                rollback = ROLLBACK.search(result.stderr)
                if result.returncode != 3 or COMPLETE not in result.stderr or \
                        not rollback or rollback.group(3) != "0" or \
                        TEARDOWN not in result.stdout:
                    raise SystemExit(f"generated construction {mode} generation failed: "
                                     f"status={result.returncode} complete={int(COMPLETE in output)} "
                                     f"rollback={rollback.groups() if rollback else 'absent'}")
                offsets = [result.stderr.find(
                    f"original generated construction: stage={stage}") for stage in STAGES]
                if any(offset < 0 for offset in offsets) or offsets != sorted(offsets):
                    raise SystemExit("generated construction source stages changed")
        absent = {key: value for key, value in {**common, "ZH_M21_SCENARIO": "mission"}.items()
                  if key != "ZH_M22_GENERATED_CONSTRUCTION_ROUTE"}
        stopped = run(str(args.executable.resolve()), base / "absent", source,
                      env_overrides=absent)
        if stopped.returncode != 3 or "mapini=complete terrain=not-loaded" not in stopped.stderr or \
                COMPLETE in stopped.stderr:
            raise SystemExit("generated construction changed accepted parser stop")
        invalid = {key: value for key, value in common.items()
                   if key != "ZH_M22_GENERATED_SCENE_ROUTE"}
        invalid["ZH_M21_SCENARIO"] = "mission"
        rejected = run(str(args.executable.resolve()), base / "invalid", source,
                       env_overrides=invalid)
        if rejected.returncode != 3 or \
                "construction selector requires generated scene route" not in rejected.stderr:
            raise SystemExit("standalone construction selector was accepted")
        removed = run(str(args.executable.resolve()), base / "missing", missing,
                      env_overrides={**common, "ZH_M21_SCENARIO": "mission"})
        if removed.returncode != 3 or COMPLETE in removed.stderr or \
                not ROLLBACK.search(removed.stderr) or TEARDOWN not in removed.stdout:
            raise SystemExit("generated construction provider removal did not fail closed")
        retry = run(str(args.executable.resolve()), base / "retry", source,
                    env_overrides={**common, "ZH_M21_SCENARIO": "mission"})
        if retry.returncode != 3 or COMPLETE not in retry.stderr or \
                not ROLLBACK.search(retry.stderr) or TEARDOWN not in retry.stdout:
            raise SystemExit("generated construction provider retry failed")
        if before != snapshot(source):
            raise SystemExit("generated construction changed read-only input")
    print("M22 08F generated construction: modes=2 generations=2 stages=8 owners=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
