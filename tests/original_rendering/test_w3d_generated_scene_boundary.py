#!/usr/bin/env python3
"""Verify the generated-only source transition after the accepted water reset."""

import argparse
from pathlib import Path
import re
import shutil
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run as scenario_run

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import run


BOUNDARY = "original generated scene boundary: mapini=complete terrain=not-loaded"
RESET = "original retail water reset: complete=1"
STOP = "original active-water reset next boundary pending"
ROLLBACK = "original graphics rollback:"
ROLLBACK_COUNTS = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")
TEARDOWN = "original recording factory teardown: resources=0"


def snapshot(root: Path):
    return tuple(sorted((path.relative_to(root), path.read_bytes())
                        for path in root.rglob("*") if path.is_file()))


def check(result, *, boundary: bool, message: str, device_constructed: bool = True):
    output = result.stdout + result.stderr
    rollback = ROLLBACK_COUNTS.search(result.stderr)
    if result.returncode != 3 or (BOUNDARY in result.stderr) != boundary or \
            not rollback or rollback.group(3) != "0" or \
            (device_constructed and TEARDOWN not in result.stdout) or \
            "original scenario setup:" in result.stdout:
        raise SystemExit(f"generated scene {message} failed: status={result.returncode} "
                         f"boundary={int(BOUNDARY in result.stderr)} "
                         f"rollback={int(ROLLBACK in result.stderr)} "
                         f"teardown={int(TEARDOWN in result.stdout)} "
                         f"allocations={rollback.groups() if rollback else 'absent'}")
    if boundary and (RESET not in result.stderr or STOP in output or
                     "original retail Recording scene: logic=terrain" in result.stderr):
        raise SystemExit(f"generated scene {message} crossed wrong source boundary")
    return tuple(int(value) for value in rollback.groups())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--ordinary-executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    source_root = args.source_root.resolve()
    logic_source = (source_root / "GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp").read_text()
    start = logic_source.index("void GameLogic::startNewGame")
    parser_call = logic_source.index("loadMapINI( TheGlobalData->m_mapName )", start)
    boundary = logic_source.index(BOUNDARY, parser_call)
    terrain_load = logic_source.index("TheTerrainLogic->loadMap(", parser_call)
    if not parser_call < boundary < terrain_load:
        raise SystemExit("generated scene source parser stop moved across terrain load")
    fixture = load_m20_fixture(source_root)
    with TemporaryDirectory(prefix="zh-m22-generated-scene-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        ordinary = base / "ordinary-input"
        shutil.copytree(source, ordinary)
        game_data = source / "Data/INI/Default/GameData.ini"
        original = game_data.read_text(encoding="ascii")
        game_data.write_text(original.replace("END\n", " UseWaterPlane = Yes\n"
                                              " UseCloudPlane = Yes\n"
                                              " WaterExtentX = 8\n WaterExtentY = 8\n"
                                              " WaterType = 0\n UseShadowVolumes = Yes\n"
                                              " MaxTerrainTracks = 2\nEND\n", 1),
                             encoding="ascii")
        # The parser must execute a project-owned override before the boundary.
        fixture.write(source / "Maps/Owned/map.ini", "GameData\n MaxTerrainTracks = 2\nEnd\n")
        before = snapshot(source)
        missing = base / "missing-provider"
        shutil.copytree(source, missing)
        (missing / "Data/INI/Default/Water.ini").unlink()
        fixture.make_read_only(source)
        fixture.make_read_only(missing)
        fixture.make_read_only(ordinary)
        profile = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
        }
        executable = str(args.executable.resolve())
        diagnostic_deltas = []
        for mode in ("mission", "skirmish"):
            normal = scenario_run(args.ordinary_executable.resolve(),
                                  base / f"{mode}-ordinary", ordinary, mode)
            if normal.returncode or "original scenario setup:" not in normal.stdout or \
                    BOUNDARY in normal.stderr:
                raise SystemExit(f"generated scene changed ordinary {mode} completion: "
                                 f"status={normal.returncode} "
                                 f"setup={int('original scenario setup:' in normal.stdout)} "
                                 f"boundary={int(BOUNDARY in normal.stderr)}")
            scenario = {**profile, "ZH_M21_SCENARIO": mode,
                        "ZH_M21_MAP": r"Maps\Owned\Owned.map"}
            for generation in range(2):
                result = run(executable, base / f"{mode}-{generation}", source,
                             env_overrides=scenario)
                residual, baseline, _ = check(result, boundary=True,
                                              message=f"{mode} generation")
                diagnostic_deltas.append(("positive", residual - baseline))
                if result.stderr.count(BOUNDARY) != 1:
                    raise SystemExit("generated scene parser marker repeated")
            absent = run(executable, base / f"{mode}-absent-selector", source,
                         env_overrides={key: value for key, value in scenario.items()
                                        if key != "ZH_M22_GENERATED_SCENE_ROUTE"})
            residual, baseline, _ = check(absent, boundary=False,
                                          message=f"{mode} absent selector")
            diagnostic_deltas.append(("absent", residual - baseline))
            if STOP not in absent.stderr or RESET not in absent.stderr:
                raise SystemExit("generated scene changed accepted 08D early stop")
        for name, removed in (("no-config", "ZH_M22_RETAIL_CONFIG_ROUTE"),
                              ("no-reset", "ZH_M22_RETAIL_CONFIG_RESET_PROFILE")):
            selected = {**profile, "ZH_M21_SCENARIO": "mission",
                        "ZH_M21_MAP": r"Maps\Owned\Owned.map"}
            selected.pop(removed)
            result = run(executable, base / name, source, env_overrides=selected)
            check(result, boundary=False, message=name)
            if "generated scene selector requires bounded Recording reset owners" not in result.stderr:
                raise SystemExit(f"generated scene {name} did not fail at selector gate")
        selected = {**profile, "ZH_M21_SCENARIO": "mission",
                    "ZH_M21_MAP": r"Maps\Owned\Owned.map"}
        failed_device = run(executable, base / "device-failure", source,
                            env_overrides={**selected, "ZH_M22_FACTORY_FAIL_DEVICE": "1"})
        check(failed_device, boundary=False, message="device failure",
              device_constructed=False)
        provider = run(executable, base / "provider-removal", missing,
                       env_overrides=selected)
        check(provider, boundary=False, message="provider removal")
        retry = run(executable, base / "provider-retry", source,
                    env_overrides=selected)
        residual, baseline, _ = check(retry, boundary=True, message="provider retry")
        diagnostic_deltas.append(("retry", residual - baseline))
        absent_deltas = {delta for kind, delta in diagnostic_deltas if kind == "absent"}
        if len(absent_deltas) != 1 or any(delta not in absent_deltas
                                          for _, delta in diagnostic_deltas):
            raise SystemExit("generated selector changed accepted 08D process-pool rollback baseline")
        if before != snapshot(source):
            raise SystemExit("generated scene changed read-only input")
    print("M22 08F0 generated scene boundary: modes=2 generations=2 parser=complete owners=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
