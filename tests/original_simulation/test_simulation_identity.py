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
    dispatcher = (root / "GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp").read_text()
    failures = []
    for token in ("MSG_CREATE_SELECTED_GROUP", "MSG_DO_MOVETO", "MSG_DO_FORCE_ATTACK_OBJECT",
                  "MSG_SELF_DESTRUCT", "GameEngine::update()", "getLastCommandSource",
                  "hasAchievedVictory", "hasSinglePlayerBeenDefeated"):
        if token not in engine:
            failures.append(f"production simulation lacks {token}")
    for token in ("groupMoveToPosition", "groupForceAttackObject", "killPlayer"):
        if token not in dispatcher:
            failures.append(f"original dispatcher lacks {token}")
    linked = args.link_map.read_text(errors="replace").split("Discarded input sections", 1)[0]
    for member in ("GameLogic.cpp.o", "GameLogicDispatch.cpp.o", "AI.cpp.o", "AIUpdate.cpp.o",
                   "ScriptEngine.cpp.o", "VictoryConditions.cpp.o"):
        if member not in linked:
            failures.append(f"production link lacks {member}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M21 simulation identity: original dispatcher, AI, scripts, objects, and victory providers linked")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
