"""Reject native-layout WW3D material inlines in the GameClient host TU."""

import argparse
import json
import pathlib
import subprocess
import sys


def check(commands: list[dict], host: pathlib.Path, probe: pathlib.Path) -> None:
    source = next((entry for entry in commands if entry["file"].endswith(
        "tests/original_rendering/retail_material_probe.cpp")), None)
    if source is None or "-DZH_WW3D_CPU_ONLY=1" not in source["command"]:
        raise ValueError("retail material probe has incompatible WW3D CPU ABI")
    if "-DZH_WW3D_CPU_ONLY=1" in next(entry for entry in commands if entry["file"].endswith(
            "src/original_runtime/linux_game_engine.cpp") and
            "zh_original_w3d_full_probe" in entry["command"])["command"]:
        raise ValueError("GameClient host changed ABI without all dependent source owners")
    host_symbols = subprocess.run(["nm", "-C", "--defined-only", str(host)], capture_output=True,
                                  text=True, check=True).stdout
    if "MeshModelClass::Get_Pass_Count" in host_symbols or "VertexMaterialClass::Peek_Mapper" in host_symbols:
        raise ValueError("GameClient host emits original WW3D material inlines with incompatible ABI")
    probe_symbols = subprocess.run(["nm", "-C", "--defined-only", str(probe)], capture_output=True,
                                   text=True, check=True).stdout
    if "zh_probe_retail_material_families" not in probe_symbols:
        raise ValueError("WW3D CPU ABI material witness is absent")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--host-object", type=pathlib.Path, required=True)
    parser.add_argument("--probe-object", type=pathlib.Path, required=True)
    args = parser.parse_args()
    commands = json.loads(args.compile_commands.read_text())
    try:
        check(commands, args.host_object, args.probe_object)
        stripped = [dict(entry) for entry in commands]
        for entry in stripped:
            if entry["file"].endswith("tests/original_rendering/retail_material_probe.cpp"):
                entry["command"] = entry["command"].replace("-DZH_WW3D_CPU_ONLY=1", "")
        try:
            check(stripped, args.host_object, args.probe_object)
        except ValueError as error:
            if "incompatible WW3D CPU ABI" not in str(error):
                raise
        else:
            raise ValueError("provider-removal control accepted a missing WW3D CPU ABI definition")
    except (ValueError, StopIteration) as error:
        print(error, file=sys.stderr)
        return 1
    print("original WW3D material ABI isolation and missing-CPU-flag negative: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
