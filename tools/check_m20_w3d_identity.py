#!/usr/bin/env python3
import argparse
import csv
import json
import pathlib
import re
import subprocess
import sys


W3D_SOURCES = (
    "W3DModuleFactory.cpp", "W3DModelDraw.cpp", "W3DDependencyModelDraw.cpp",
    "W3DLaserDraw.cpp", "W3DOverlordAircraftDraw.cpp", "W3DOverlordTankDraw.cpp",
    "W3DOverlordTruckDraw.cpp", "W3DProjectileStreamDraw.cpp", "W3DPropDraw.cpp",
    "W3DScienceModelDraw.cpp", "W3DSupplyDraw.cpp", "W3DTankDraw.cpp",
    "W3DTankTruckDraw.cpp", "W3DTreeDraw.cpp", "W3DTruckDraw.cpp",
)

REGISTRATIONS = (
    "W3DDefaultDraw", "W3DDebrisDraw", "W3DModelDraw", "W3DLaserDraw",
    "W3DOverlordTankDraw", "W3DOverlordTruckDraw", "W3DOverlordAircraftDraw",
    "W3DProjectileStreamDraw", "W3DPoliceCarDraw", "W3DRopeDraw",
    "W3DScienceModelDraw", "W3DSupplyDraw", "W3DDependencyModelDraw",
    "W3DTankDraw", "W3DTruckDraw", "W3DTracerDraw", "W3DTankTruckDraw",
    "W3DTreeDraw", "W3DPropDraw",
)


def has_direct3d_dependency(ldd_output: str) -> bool:
    """Inspect DSO names, not ASLR mapping addresses or loader paths."""
    for line in ldd_output.splitlines():
        fields = line.split("=>", 1)[0].split()
        if not fields:
            continue
        name = pathlib.PurePosixPath(fields[0]).name.lower()
        if "d3d" in name or "direct3d" in name:
            return True
    return False


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--factory-source", type=pathlib.Path, required=True)
    parser.add_argument("--classification", type=pathlib.Path, required=True)
    args = parser.parse_args()

    failures: list[str] = []
    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    active = {}
    for entry in commands:
        if "CMakeFiles/zh_original_config_providers.dir/" not in entry.get("output", ""):
            continue
        name = pathlib.Path(entry["file"]).name
        if name in W3D_SOURCES:
            active[name] = entry["command"]
    if set(active) != set(W3D_SOURCES):
        failures.append(f"active W3D source set mismatch: {sorted(active)}")
    for name, command in active.items():
        if "-DZH_W3D_SCHEMA_ONLY" not in command or "-DBRUTAL_TIMING_HACK" not in command:
            failures.append(f"schema boundary definitions absent: {name}")
        if re.search(r"(?:^|[/\\])d3d8\.h(?:\s|$)|(?:^|\s)-l(?:d3d|dx)", command, re.I):
            failures.append(f"legacy D3D entered schema compile: {name}")

    source = args.factory_source.read_text(encoding="utf-8")
    registered = re.findall(r"addW3DSchema\((W3D\w+),", source)
    if tuple(registered) != REGISTRATIONS:
        failures.append(f"registration inventory/order mismatch: {registered}")

    included = args.link_map.read_text(encoding="utf-8", errors="replace").split(
        "Discarded input sections", 1
    )[0]
    for name in W3D_SOURCES:
        member = f"libzh_original_config_providers.a({name}.o)"
        if member not in included:
            failures.append(f"production link did not extract {name}")

    symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    for symbol in (
        "W3DModuleFactory::init()", "W3DModelDrawModuleData::buildFieldParse",
        "W3DLaserDrawModuleData::buildFieldParse", "W3DTruckDrawModuleData::buildFieldParse",
    ):
        if symbol not in symbols:
            failures.append(f"production executable lacks original symbol: {symbol}")
    dynamic = subprocess.run(
        ["ldd", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    if has_direct3d_dependency(dynamic):
        failures.append("production executable acquired a Direct3D dependency")

    with args.classification.open(encoding="utf-8", newline="") as stream:
        rows = {pathlib.Path(row["path"]).name: row for row in csv.DictReader(stream, delimiter="\t")}
    for name in W3D_SOURCES:
        row = rows.get(name)
        if row is None or row["disposition"] != "production-compiled" or not row["provider"].endswith(name):
            failures.append(f"classification is not backed by its production source: {name}")

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M20 W3D identity: ok registrations=19 sources=15 device-dependencies=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
