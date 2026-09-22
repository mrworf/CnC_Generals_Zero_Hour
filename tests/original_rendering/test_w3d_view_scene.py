#!/usr/bin/env python3
"""Run canonical W3DView camera/source-scene frames in an initialized GameClient."""

import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run
from test_w3d_status_scene import has_validation_diagnostic


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--physical", action="store_true")
    args = parser.parse_args()
    game_client = (args.source_root / "GeneralsMD/Code/GameEngine/Source/GameClient/GameClient.cpp").read_text()
    teardown = game_client.split("GameClient::~GameClient()", 1)[1].split("void GameClient::init", 1)[0]
    if not (teardown.index("delete TheInGameUI;") <
            teardown.index("delete TheTerrainVisual;") <
            teardown.index("delete TheDisplay;")):
        raise SystemExit("original GameClient UI/visual/display destructor order changed")
    ui_source = (args.source_root / "GeneralsMD/Code/GameEngine/Source/GameClient/InGameUI.cpp").read_text()
    ui_destructor = ui_source.split("InGameUI::~InGameUI()", 1)[1].split("void InGameUI::init", 1)[0]
    placement_cleanup = ui_source.split("void InGameUI::destroyPlacementIcons", 1)[1].split(
        "void InGameUI::", 1)[0]
    if "placeBuildAvailable( NULL, NULL );" not in ui_destructor or \
            "TheTerrainVisual->removeAllBibs();" not in placement_cleanup:
        raise SystemExit("original UI destructor no longer reaches terrain bib cleanup")
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-view-scene-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.make_read_only(source)
        packet = base / "rigid.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-rigid", str(packet)],
                       check=True)
        os.environ["ZH_M22_VIEW_SCENE_PROFILE"] = "1"
        os.environ["ZH_M22_VIEW_SCENE_ASSET"] = str(packet)
        if args.physical:
            os.environ["ZH_M22_VIEW_SCENE_PHYSICAL"] = "1"
        result = run(args.executable.resolve(), base, source, "mission")
        marker = ("original W3DView frame: physical generations=4 resources=0" if args.physical
                  else "original W3DView frame: source draw=1 resources=0")
        output = result.stdout + result.stderr
        if result.returncode or marker not in result.stdout or has_validation_diagnostic(output):
            raise SystemExit(f"original W3DView scene failed ({result.returncode}):\n{output}")
    print("original W3DView source scene frame: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
