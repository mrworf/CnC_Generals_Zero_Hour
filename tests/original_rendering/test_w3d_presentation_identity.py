"""M22 original GameClient CPU owner identity and mutually exclusive ABI gates."""

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys


DRAW_NAMES = (
    "W3DDefaultDraw", "W3DModelDraw", "W3DTankDraw", "W3DTankTruckDraw",
    "W3DTruckDraw", "W3DSupplyDraw", "W3DOverlordTankDraw",
    "W3DDependencyModelDraw", "W3DOverlordAircraftDraw", "W3DOverlordTruckDraw",
)
CLIENT_NAMES = (
    "W3DDisplay", "W3DView", "W3DAssetManager", "W3DScene", "W3DTerrainTracks", "W3DShroud",
    "BaseHeightMap", "HeightMap", "W3DWater",
)
WW3D_NAMES = ("scene", "light", "matpass")


def verify(commands, link_map, executable, removed=None, skip_runtime_witness=False):
    full_prefix = "src/original_runtime/full_w3d/CMakeFiles/zh_original_w3d_draw_full.dir/"
    for name in DRAW_NAMES + CLIENT_NAMES:
        original_source = f"/GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/"
        matching = [entry for entry in commands if original_source in entry["file"]
                    and entry["file"].endswith(f"/{name}.cpp")]
        full = [entry for entry in matching if "-DZH_WW3D_CPU_ONLY=1" in entry["command"]
                and "-DZH_W3D_SCHEMA_ONLY" not in entry["command"]
                and "-DZH_W3D_HEADLESS_INSTANCE" not in entry["command"]]
        if len(full) != 1:
            raise ValueError(f"full original GameClient provider not compiled once: {name}")
        if not any(full_prefix in line and f"/{name}.cpp.o" in line for line in link_map.splitlines()):
            raise ValueError(f"full original GameClient provider not linked: {name}")
        if name in DRAW_NAMES:
            if len(matching) < 2 or not any("-DZH_W3D_HEADLESS_INSTANCE" in e["command"] for e in matching):
                raise ValueError(f"canonical schema/full source configuration missing: {name}")
            if f"libzh_original_config_providers.a({name}.cpp.o)" in link_map:
                raise ValueError(f"mixed schema/full original draw objects: {name}")
    if not any(full_prefix in line and "/W3DShadow.cpp.o" in line
               for line in link_map.splitlines()) or not any(
        entry["file"].endswith("/Shadow/W3DShadow.cpp") for entry in commands
    ):
        raise ValueError("original shadow provider missing")
    for name in WW3D_NAMES:
        path = f"/GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/{name}.cpp"
        if not any(entry["file"].endswith(path) for entry in commands) or f"libzh_w3d.a({name}.cpp.o)" not in link_map:
            raise ValueError(f"original WW3D scene provider missing: {name}")
    ease_path = "/GeneralsMD/Code/GameEngine/Source/GameClient/ParabolicEase.cpp"
    if not any(entry["file"].endswith(ease_path) for entry in commands) or not any(
        full_prefix in line and "/ParabolicEase.cpp.o" in line for line in link_map.splitlines()
    ):
        raise ValueError("original W3DView ease provider missing")
    if removed or skip_runtime_witness:
        return
    result = subprocess.run([str(executable)], capture_output=True, text=True, check=False)
    if result.returncode or "original-rendering runtime provider=GeneralsMD GameClient CPU presentation" not in result.stdout:
        raise ValueError(f"original runtime witness failed: {result.stderr}")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=Path, required=True)
    parser.add_argument("--link-map", type=Path, required=True)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--provider-removal", action="store_true")
    parser.add_argument("--skip-runtime-witness", action="store_true")
    args = parser.parse_args()
    commands = json.loads(args.compile_commands.read_text())
    raw_map = args.link_map.read_text(errors="replace")
    # GNU link maps repeat every object's path thousands of times by section.
    # Preserve the exact linked object provenance but perform negative controls
    # against one canonical occurrence per object, not a 60 MiB section dump.
    linked = "\n".join(sorted(set(re.findall(
        r"src/original_runtime/full_w3d/CMakeFiles/zh_original_w3d_draw_full\.dir/[^\s()]+?\.cpp\.o"
        r"|libzh_w3d\.a\([A-Za-z0-9_]+\.cpp\.o\)"
        r"|libzh_original_config_providers\.a\([A-Za-z0-9_]+\.cpp\.o\)",
        raw_map,
    ))))
    try:
        verify(commands, linked, args.executable, skip_runtime_witness=args.skip_runtime_witness)
        if args.provider_removal:
            for name in DRAW_NAMES + CLIENT_NAMES:
                trimmed = [entry for entry in commands if not (
                    entry["file"].endswith(f"/{name}.cpp")
                    and "-DZH_WW3D_CPU_ONLY=1" in entry["command"])]
                try:
                    verify(trimmed, linked, args.executable, removed=name)
                except ValueError:
                    pass
                else:
                    raise ValueError(f"provider removal was accepted: {name}")
            missing_shadow = [entry for entry in commands
                              if not entry["file"].endswith("/Shadow/W3DShadow.cpp")]
            try:
                verify(missing_shadow, linked, args.executable, removed="W3DShadow")
            except ValueError:
                pass
            else:
                raise ValueError("shadow provider removal was accepted")
            for name in CLIENT_NAMES + DRAW_NAMES:
                truncated_map = linked.replace(f"/{name}.cpp.o", f"/removed-{name}.o")
                try:
                    verify(commands, truncated_map, args.executable, removed=name)
                except ValueError:
                    pass
                else:
                    raise ValueError(f"link provider removal was accepted: {name}")
            truncated_shadow_map = linked.replace("/W3DShadow.cpp.o", "/removed-W3DShadow.o")
            try:
                verify(commands, truncated_shadow_map, args.executable, removed="W3DShadow")
            except ValueError:
                pass
            else:
                raise ValueError("shadow link provider removal was accepted")
            for name in WW3D_NAMES:
                removed_source = f"/WW3D2/{name}.cpp"
                without_source = [entry for entry in commands
                                  if not entry["file"].endswith(removed_source)]
                try:
                    verify(without_source, linked, args.executable, removed=name)
                except ValueError:
                    pass
                else:
                    raise ValueError(f"WW3D scene provider removal was accepted: {name}")
                truncated_ww3d_map = linked.replace(f"libzh_w3d.a({name}.cpp.o)", "removed-ww3d.o")
                try:
                    verify(commands, truncated_ww3d_map, args.executable, removed=name)
                except ValueError:
                    pass
                else:
                    raise ValueError(f"WW3D scene link provider removal was accepted: {name}")
            without_ease = [entry for entry in commands if not entry["file"].endswith(
                "/GameClient/ParabolicEase.cpp")]
            try:
                verify(without_ease, linked, args.executable, removed="ParabolicEase")
            except ValueError:
                pass
            else:
                raise ValueError("W3DView ease source removal was accepted")
            try:
                verify(commands, linked.replace("/ParabolicEase.cpp.o", "/removed-ease.o"),
                       args.executable, removed="ParabolicEase")
            except ValueError:
                pass
            else:
                raise ValueError("W3DView ease link removal was accepted")
    except ValueError as error:
        print(f"original GameClient identity error: {error}", file=sys.stderr)
        return 1
    print("M22 original GameClient CPU owners and ten unmixed full draw providers witnessed"
          + ("; provider removal rejected" if args.provider_removal else ""))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
