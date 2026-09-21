#!/usr/bin/env python3
import argparse
import pathlib
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    args = parser.parse_args()
    root = args.source_root
    engine = (root / "src/original_runtime/linux_game_engine.cpp").read_text()
    game_logic = (root / "GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp").read_text()
    failures = []
    for call in ("TheGameLogic->startNewGame(FALSE)", "newInstance(Drawable)",
                 "getControlsInitCount", "g_propCount"):
        if call not in engine:
            failures.append(f"production scenario lacks {call}")
    for call in ("TheTerrainVisual->addProp", "TheGameClient->preloadAssets", "TheRecorder->initControls"):
        if call not in game_logic:
            failures.append(f"original start path lacks {call}")
    linked = args.link_map.read_text(errors="replace").split("Discarded input sections", 1)[0]
    for member in ("GameLogic.cpp.o", "Drawable.cpp.o", "Recorder.cpp.o", "TerrainVisual.cpp.o"):
        if member not in linked:
            failures.append(f"production link lacks {member}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M21 setup identity: original GameLogic, Drawable, preload, prop, and Recorder controls linked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
