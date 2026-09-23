#!/usr/bin/env python3
"""Exercise a generated original W3DModelDraw decal request before FD manager delivery."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


MARKER = "original source shadow: owner=1 decal-request=2 typed-negatives=3 removal=1 generations=2 resources=0"


def write_fixture(root: Path, fixture, producer: Path, shadow: str) -> None:
    prepare_owned_source(root, fixture)
    model = root / "Art/W3D/TEST.w3d"
    model.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run([str(producer), "--emit", str(model)], check=True)
    object_ini = root / "Data/INI/Default/Object.ini"
    text = object_ini.read_text()
    draw = (" Shadow = " + shadow + "\n"
            " ShadowTexture = M22SourceShadow\n"
            " ShadowSizeX = 16\n"
            " ShadowSizeY = 8\n"
            " ShadowOffsetX = 1\n"
            " ShadowOffsetY = -2\n"
            " Draw = W3DModelDraw ModuleTag_M22Shadow\n"
            "  DefaultConditionState\n"
            "   Model = TEST.HLOD\n"
            "  End\n"
            " End\n")
    fixture.write(object_ini, text.replace("Object LogicFixture\n", "Object LogicFixture\n" + draw, 1))


def execute(executable: Path, root: Path, source: Path):
    os.environ["ZH_M22_SHADOW_SOURCE_PROFILE"] = "1"
    return run(executable, root, source, "mission")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-shadow-source-") as scratch:
        root = Path(scratch)
        source = root / "decal-source"
        write_fixture(source, fixture, args.asset_producer.resolve(), "SHADOW_DECAL")
        invalid = root / "volume-source"
        shutil.copytree(source, invalid)
        object_ini = invalid / "Data/INI/Default/Object.ini"
        fixture.write(object_ini, object_ini.read_text().replace("Shadow = SHADOW_DECAL", "Shadow = SHADOW_VOLUME", 1))
        fixture.make_read_only(source)
        fixture.make_read_only(invalid)
        for generation in range(2):
            result = execute(args.executable.resolve(), root / f"decal-run-{generation}", source)
            if result.returncode or MARKER not in result.stdout:
                raise SystemExit(f"generated original decal owner failed ({result.returncode}):\n{result.stdout}{result.stderr}")
        rejected = execute(args.executable.resolve(), root / "volume-run", invalid)
        if rejected.returncode == 0 or "did not preserve exact decal metadata" not in (rejected.stdout + rejected.stderr):
            raise SystemExit("generated volume fixture did not fail before the decal source-owner witness")
    os.environ.pop("ZH_M22_SHADOW_SOURCE_PROFILE", None)
    print("original source shadow: generated decal owner/rejection/removal/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
