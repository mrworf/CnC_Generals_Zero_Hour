#!/usr/bin/env python3
"""Exercise original draw construction in an owned GameClient scenario."""

import argparse
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--asset-producer", type=pathlib.Path, required=True)
    parser.add_argument("--retail-archive", type=pathlib.Path)
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    context = tempfile.TemporaryDirectory(prefix="zh-m22-draw-")
    try:
        scratch = context.name
        base = pathlib.Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        model_file = source / "Art/W3D/TEST.w3d"
        model_file.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([str(args.asset_producer.resolve()), "--emit", str(model_file)], check=True)
        ini = source / "Data/INI/Default/Object.ini"
        content = ini.read_text()
        draw_types = ("W3DDefaultDraw", "W3DModelDraw", "W3DTankDraw",
                      "W3DTankTruckDraw", "W3DTruckDraw", "W3DSupplyDraw",
                      "W3DOverlordTankDraw", "W3DDependencyModelDraw",
                      "W3DOverlordAircraftDraw", "W3DOverlordTruckDraw")
        draw_modules = []
        for index, draw_type in enumerate(draw_types):
            draw_modules.append(f" Draw = {draw_type} ModuleTag_Draw{index}\n")
            if draw_type != "W3DDefaultDraw":
                if draw_type in ("W3DTankDraw", "W3DOverlordTankDraw"):
                    draw_modules.append("  TreadAnimationRate = 0.1\n")
                if draw_type in ("W3DTankTruckDraw", "W3DTruckDraw", "W3DOverlordTruckDraw"):
                    draw_modules.append("  LeftFrontTireBone = TIRE_FL\n"
                                        "  RightFrontTireBone = TIRE_FR\n"
                                        "  LeftRearTireBone = TIRE_RL\n"
                                        "  RightRearTireBone = TIRE_RR\n"
                                        "  TireRotationMultiplier = 0.1\n")
                if draw_type == "W3DSupplyDraw":
                    draw_modules.append("  SupplyBonePrefix = SUPPLY\n")
                    draw_modules.append("  ExtraPublicBone = SUPPLY\n")
                model = "TEST.HLOD"
                draw_modules.append("  DefaultConditionState\n"
                                    f"   Model = {model}\n")
                if draw_type == "W3DModelDraw":
                    draw_modules.append("   Animation = TESTTREE.IDLE\n")
                draw_modules.append("  End\n")
            draw_modules.append(" End\n")
        content = content.replace("Object LogicFixture\n", "Object LogicFixture\n"
                                  + "".join(draw_modules), 1)
        content = content.replace(" Behavior = AIUpdateInterface ModuleTag_AI\n",
                                  " Behavior = PhysicsBehavior ModuleTag_Physics\n End\n"
                                  " Behavior = OverlordContain ModuleTag_Overlord\n"
                                  "  PayloadTemplateName = RiderFixture\n"
                                  "  AllowInsideKindOf = PORTABLE_STRUCTURE\n"
                                  "  Slots = 1\n"
                                  " End\n"
                                  " Behavior = AIUpdateInterface ModuleTag_AI\n", 1)
        content += ("Object RiderFixture\n"
                    " KindOf = SELECTABLE PORTABLE_STRUCTURE STRUCTURE\n"
                    " TransportSlotCount = 1\n"
                    " Draw = W3DDependencyModelDraw ModuleTag_RiderDraw\n"
                    "  DefaultConditionState\n"
                    "   Model = TEST.HLOD\n"
                    "  End\n"
                    " End\n"
                    " Body = ActiveBody ModuleTag_Body\n"
                    "  MaxHealth = 100\n"
                    "  InitialHealth = 100\n"
                    " End\n"
                    "End\n")
        fixture.write(ini, content)
        locomotor = source / "Data/INI/Locomotor.ini"
        fixture.write(locomotor, locomotor.read_text().replace(
            "Appearance = TWO_LEGS", "Appearance = FOUR_WHEELS", 1))
        fixture.write(source / "Data/INI/ParticleSystem.ini",
                      "ParticleSystem TrackDebrisDirtLeft\nEnd\n"
                      "ParticleSystem TrackDebrisDirtRight\nEnd\n")
        missing_root = base / "missing-input"
        shutil.copytree(source, missing_root)
        missing_ini = missing_root / "Data/INI/Default/Object.ini"
        missing_text = missing_ini.read_text().replace("Model = TEST.HLOD", "Model = MISSING.HLOD", 1)
        fixture.write(missing_ini, missing_text)
        missing_rider_root = base / "missing-rider-input"
        shutil.copytree(source, missing_rider_root)
        missing_rider_ini = missing_rider_root / "Data/INI/Default/Object.ini"
        fixture.write(missing_rider_ini, missing_rider_ini.read_text().replace("Slots = 1", "Slots = 0", 1))
        fixture.make_read_only(source)
        fixture.make_read_only(missing_root)
        fixture.make_read_only(missing_rider_root)
        if args.retail_archive:
            archive = args.retail_archive.resolve(strict=True)
            # Only the owned input root is temporarily writable. The retail
            # archive target is never chmod'ed or otherwise modified.
            source.chmod(0o700)
            (source / "W3DZH.big").symlink_to(archive)
            source.chmod(0o500)
            os.environ["ZH_M22_RETAIL_MODEL"] = "ABBarracks_AC"
        os.environ["ZH_M22_DRAW_PROFILE"] = "1"
        result = run(args.executable.resolve(), base, source, "mission")
        if result.returncode or "original scenario setup:" not in result.stdout or "original full draw: drawables=4 hlods=10 animations=1 supply-transitions=2 logic-bones=1 client-before-logic-bones=0 dependency-blocks=2 dependency-releases=2 tread-scrolls=2 wheel-controls=3 rider-dependencies=3" not in result.stdout:
            raise SystemExit(f"full original GameClient draw failed ({result.returncode}):\n"
                             f"{result.stdout}{result.stderr}")
        observed = [line for line in result.stdout.splitlines()
                    if line.startswith("original full draw module:")]
        if len(observed) != len(draw_types) + 1 or any(
                sum(draw_type in line for line in observed) !=
                (2 if draw_type == "W3DDependencyModelDraw" else 1)
                for draw_type in draw_types):
            raise SystemExit(f"original GameClient omitted concrete full draw identity:\n"
                             f"{result.stdout}{result.stderr}")
        if args.retail_archive and "original retail W3D model: ABBarracks_AC class=" not in result.stdout:
            raise SystemExit(f"retail W3D model did not load through original provider:\n"
                             f"{result.stdout}{result.stderr}")
        if args.retail_archive:
            aggregate = re.search(r"^original retail material families: meshes=(\d+) materials=(\d+)(.*)$",
                                  result.stdout, re.MULTILINE)
            if not aggregate or int(aggregate[1]) < 1 or int(aggregate[2]) < 1 or not re.fullmatch(
                    r"(?: mapper-\d+=\d+)*", aggregate[3]):
                raise SystemExit("original retail material family aggregate is absent or malformed")
            shaders = re.search(r"^original retail shader families: variants=(\d+)(.*)$",
                                result.stdout, re.MULTILINE)
            if not shaders or int(shaders[1]) < 1 or not re.fullmatch(
                    r"(?: (?:primary|detail-color|detail-alpha|fog|blend)-\d+=\d+)+", shaders[2]):
                raise SystemExit("original retail shader family aggregate is absent or malformed")
        if args.keep:
            print(result.stdout)
        os.environ.pop("ZH_M22_RETAIL_MODEL", None)
        missing = run(args.executable.resolve(), base, missing_root, "mission")
        if missing.returncode or "original full draw: drawables=4 hlods=9 animations=0 supply-transitions=2 logic-bones=1 client-before-logic-bones=0 dependency-blocks=2 dependency-releases=2 tread-scrolls=2 wheel-controls=3 rider-dependencies=3" not in missing.stdout:
            raise SystemExit(f"missing original model was not distinguishable:\n"
                             f"{missing.stdout}{missing.stderr}")
        missing_rider = run(args.executable.resolve(), base, missing_rider_root, "mission")
        if (missing_rider.returncode == 0 or
                "original Overlord rider missing" not in missing_rider.stderr or
                "original scenario setup:" in missing_rider.stdout):
            raise SystemExit(f"missing original rider did not fail closed:\n"
                             f"{missing_rider.stdout}{missing_rider.stderr}")
        print("original GameClient full W3D draw modules constructed and called: 10")
    finally:
        if args.keep:
            print(f"kept fixture: {context.name}")
            context._finalizer.detach()
        else:
            context.cleanup()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
