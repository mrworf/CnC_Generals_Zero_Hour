#!/usr/bin/env python3
import argparse
import pathlib
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    args = parser.parse_args()
    loader = (args.source_root / "GeneralsMD/Code/GameEngine/Source/Common/System/OriginalMapLoader.cpp").read_text()
    world = (args.source_root / "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/WorldHeightMap.cpp").read_text()
    linux = (args.source_root / "src/original_runtime/linux_game_engine.cpp").read_text()
    failures = []
    for token in ("HeightMapData", "ObjectsList", "PolygonTriggers", "SidesList"):
        if token not in loader:
            failures.append(f"canonical loader lacks {token}")
    for duplicate in ("ParseSizeOnly", "ParseObjectsDataChunk", "ParseObjectData"):
        if duplicate in world:
            failures.append(f"W3D retains duplicate logical callback {duplicate}")
    if "OriginalMapLoader loader" not in world or "OriginalMapLoader loader" not in linux:
        failures.append("Linux and W3D do not both consume the shared original loader")
    included = args.link_map.read_text(errors="replace").split("Discarded input sections", 1)[0]
    if "libzh_original_config_providers.a(OriginalMapLoader.cpp.o)" not in included:
        failures.append("production executable did not extract original map provider")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M21 map identity: one shared original CPU parser used by Linux and W3D")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
