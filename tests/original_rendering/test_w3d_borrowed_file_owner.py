#!/usr/bin/env python3
"""Verify the generated modeled map object borrows the display file factory."""

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


OWNER = re.compile(r"original W3D borrowed file owner: active=(\d+) modeled=(\d+)")
ROLLBACK = re.compile(r"original graphics rollback: residual=(\d+) baseline=(\d+) owners=(\d+)")
COMPLETE = "original generated construction boundary complete"


def snapshot(root: Path):
    return tuple(sorted((path.relative_to(root), path.read_bytes())
                        for path in root.rglob("*") if path.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-borrowed-file-owner-") as scratch:
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
        fixture.write(game_data, game_data.read_text(encoding="ascii").replace(
            "END\n", " UseWaterPlane = Yes\n UseCloudPlane = Yes\n"
            " WaterExtentX = 8\n WaterExtentY = 8\n WaterType = 0\n"
            " UseShadowVolumes = Yes\n MaxTerrainTracks = 2\nEND\n", 1))
        object_ini = source / "Data/INI/Default/Object.ini"
        fixture.write(object_ini, object_ini.read_text(encoding="ascii").replace(
            "Object LogicFixture\n",
            "Object LogicFixture\n Draw = W3DModelDraw ModuleTag_Model\n"
            "  DefaultConditionState\n   Model = TEST.HLOD\n  End\n End\n", 1))
        model_file = source / "Art/W3D/TEST.w3d"
        model_file.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(args.asset_producer.resolve()), "--emit", str(model_file)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        missing = base / "missing-model-input"
        shutil.copytree(source, missing)
        (missing / "Art/W3D/TEST.w3d").unlink()
        before = snapshot(source)
        fixture.make_read_only(source)
        fixture.make_read_only(missing)
        profile = {
            "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
            "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
            "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
            "ZH_M22_RETAIL_CONFIG_RESET_PROFILE": "1",
            "ZH_M22_GENERATED_SCENE_ROUTE": "1",
            "ZH_M22_GENERATED_CONSTRUCTION_ROUTE": "1",
            "ZH_M22_W3D_FILE_OWNER_PROFILE": "1",
            "ZH_M21_MAP": r"Maps\Owned\Owned.map",
            "ZH_M21_SCENARIO": "mission",
        }

        def check(label: str, root: Path, *, modeled: int | None,
                  failure: str | None = None, extra: dict | None = None):
            result = run(str(args.executable.resolve()), base / label, root,
                         env_overrides={**profile, **(extra or {})})
            owner = OWNER.search(result.stderr)
            rollback = ROLLBACK.search(result.stderr)
            if result.returncode != 3 or not rollback or rollback.group(3) != "0" or \
                    "original recording factory teardown: resources=0" not in result.stdout or \
                    (failure is None and COMPLETE not in result.stderr) or \
                    (failure is not None and (failure not in result.stderr or
                                              COMPLETE in result.stderr)) or \
                    (modeled is not None and (not owner or owner.groups() != ("1", str(modeled)))):
                raise SystemExit(f"borrowed file owner {label} failed: status={result.returncode} "
                                 f"owner={owner.groups() if owner else 'absent'} "
                                 f"rollback={rollback.groups() if rollback else 'absent'} "
                                 f"complete={int(COMPLETE in result.stderr)} "
                                 f"failure={int(failure is not None and failure in result.stderr)}")

        check("generation-1", source, modeled=1)
        check("generation-2", source, modeled=1)
        check("missing-model", missing, modeled=0)
        check("retry", source, modeled=1)
        for edge in ("BEFORE", "AFTER"):
            check(f"factory-fail-{edge.lower()}", source, modeled=None,
                  failure=f"forced original display file factory {'pre-publication' if edge == 'BEFORE' else 'post-publication'} failure",
                  extra={f"ZH_M22_FACTORY_FAIL_FILE_{edge}": "1"})
            check(f"factory-retry-{edge.lower()}", source, modeled=1)
        check("foreign-factory", source, modeled=None,
              failure="original generated scene display owners missing",
              extra={"ZH_M22_FACTORY_FOREIGN_FILE": "1"})
        check("foreign-retry", source, modeled=1)
        check("missing-factory", source, modeled=None,
              failure="original generated scene display owners missing",
              extra={"ZH_M22_FACTORY_MISSING_FILE": "1"})
        check("missing-factory-retry", source, modeled=1)
        if snapshot(source) != before:
            raise SystemExit("borrowed file owner changed read-only generated input")
    print("M22 08L3A0 borrowed W3D file factory: modeled=1 provider-removal=0 generations=2 retry=1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
