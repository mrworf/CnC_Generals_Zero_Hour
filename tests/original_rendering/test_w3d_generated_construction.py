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
from test_production_entry import run as run_process

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_terrain_source_bitmap import tga


COMPLETE = "original generated construction boundary complete"
STAGES = ("terrain", "radar", "shroud", "terrain-logic", "radar-terrain",
          "pathfinder", "observer", "objects")
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")
TEARDOWN = "original recording factory teardown: resources=0"
TRANSACTION_ROLLBACK = "original construction rollback: residual=0"
CALLBACKS = re.compile(r"original binding callbacks: created=(\d+) draw=(\d+) behavior=(\d+)")
BINDING_CONTROLS = "original binding controls: null=1 foreign=1 prior-pair=1 registries=1"
FAILURE_STAGES = (
    "object-id", "team", "behavior-modules", "behavior-resolution", "radar", "logic",
    "object", "create", "partition", "drawable-registry", "draw-modules", "client-modules",
    "drawable-resolution", "drawable", "binding", "init",
    "object-created-before", "object-created-after",
    "binding-indicator-before", "binding-indicator-after",
    "binding-draw-before", "binding-draw-after",
    "binding-condition-before", "binding-condition-after",
    "binding-behavior-before", "binding-behavior-after",
)


def run(*args, **kwargs):
    result = run_process(*args, **kwargs)
    output = result.stdout + result.stderr
    # Successful subprocess exit does not establish sanitizer-clean callbacks.
    if any(marker in output for marker in ("AddressSanitizer", "LeakSanitizer",
                                           "UndefinedBehaviorSanitizer", "runtime error:")):
        raise SystemExit("generated construction sanitizer finding")
    return result


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
        object_ini = source / "Data/INI/Default/Object.ini"
        object_ini.write_text(object_ini.read_text(encoding="ascii").replace(
            "Object LogicFixture\n", "Object LogicFixture\n"
            " Draw = W3DDefaultDraw ModuleTag_BindingDraw\n End\n", 1),
            encoding="ascii")
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
        fake = base / "fake-structure-callback"
        shutil.copytree(source, fake)
        fake_ini = fake / "Data/INI/Default/Object.ini"
        fake_ini.write_text(fake_ini.read_text(encoding="ascii").replace(
            "KindOf = SELECTABLE VEHICLE", "KindOf = SELECTABLE VEHICLE FS_FAKE", 1),
            encoding="ascii")
        before = snapshot(source)
        fixture.make_read_only(source)
        fixture.make_read_only(missing)
        fixture.make_read_only(fake)
        common = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        }
        callback_counts = [0, 0, 0]
        for mode in ("mission", "skirmish"):
            selected = {**common, "ZH_M21_SCENARIO": mode}
            for generation in range(2):
                result = run(str(args.executable.resolve()),
                             base / f"{mode}-{generation}", source,
                             env_overrides=selected)
                output = result.stdout + result.stderr
                for callbacks in CALLBACKS.findall(result.stderr):
                    callback_counts = [max(previous, int(count))
                                       for previous, count in zip(callback_counts, callbacks)]
                rollback = ROLLBACK.search(result.stderr)
                if result.returncode != 3 or COMPLETE not in result.stderr or \
                        BINDING_CONTROLS not in result.stderr or \
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
        if any(count == 0 for count in callback_counts):
            raise SystemExit("generated binding did not reach every callback family")
        indexed_stages = tuple(
            f"{family}-{boundary}:{ordinal}"
            for family, count in zip(("object-created", "binding-draw", "binding-behavior"),
                                     callback_counts)
            for boundary in ("before", "after") for ordinal in range(count))
        failure_stages = (*FAILURE_STAGES, "binding-decal-before", "binding-decal-after",
                          *indexed_stages)
        for stage in failure_stages:
            stage_source = fake if stage.startswith("binding-decal-") else source
            failed = run(str(args.executable.resolve()), base / f"failure-{stage}", stage_source,
                         env_overrides={**common, "ZH_M21_SCENARIO": "mission",
                                        "ZH_M22_CONSTRUCTION_FAIL_STAGE": stage})
            failed_rollback = ROLLBACK.search(failed.stderr)
            if failed.returncode != 3 or COMPLETE in failed.stderr or \
                    not failed_rollback or failed_rollback.group(3) != "0" or \
                    TEARDOWN not in failed.stdout or TRANSACTION_ROLLBACK not in failed.stderr:
                raise SystemExit(f"generated construction rollback failed at {stage}: "
                                 f"status={failed.returncode} "
                                 f"rollback={failed_rollback.groups() if failed_rollback else 'absent'}")
            if stage.startswith("binding-") and \
                    "original binding rollback: restored=1" not in failed.stderr:
                raise SystemExit(f"generated binding did not restore pointers at {stage}")
            retried = run(str(args.executable.resolve()), base / f"retry-{stage}", source,
                          env_overrides={**common, "ZH_M21_SCENARIO": "mission"})
            retried_rollback = ROLLBACK.search(retried.stderr)
            if retried.returncode != 3 or COMPLETE not in retried.stderr or \
                    BINDING_CONTROLS not in retried.stderr or \
                    not retried_rollback or retried_rollback.group(3) != "0" or \
                    TEARDOWN not in retried.stdout:
                raise SystemExit(f"generated construction retry failed after {stage}")
        if before != snapshot(source):
            raise SystemExit("generated construction changed read-only input")
    print(f"M22 construction: modes=2 generations=2 stages=8 "
          f"rollback-stages={len(failure_stages)} owners=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
