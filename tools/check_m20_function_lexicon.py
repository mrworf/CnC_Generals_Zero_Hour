#!/usr/bin/env python3
import argparse
import pathlib
import re
import subprocess
import sys
import tempfile


ENTRY = re.compile(r'\{\s*NAMEKEY_INVALID,\s*"([^"]+)"\s*,\s*([A-Za-z_][A-Za-z0-9_]*)\s*\}')
BOUNDARY = re.compile(r'ZH_UNSUPPORTED_(?:WINDOW|LAYOUT)_CALLBACK\(([^)]+)\)')


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--archive", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    args = parser.parse_args()

    lexicon_path = args.source_root / "GeneralsMD/Code/GameEngine/Source/Common/System/FunctionLexicon.cpp"
    boundary_path = args.source_root / "src/original_runtime/linux_online_callback_boundary.cpp"
    inventory_path = args.source_root / "cmake/LegacySourceInventory.cmake"
    entry_rows = [name for name, provider in ENTRY.findall(lexicon_path.read_text(encoding="utf-8"))
                  if name == provider]
    boundary_rows = [name for name in BOUNDARY.findall(boundary_path.read_text(encoding="utf-8"))
                     if name != "name"]
    entries = set(entry_rows)
    boundary = set(boundary_rows)
    online_entries = entries & boundary
    offline_entries = entries - boundary
    boundary_only = boundary - entries
    failures = []
    if len(entries) != 242:
        failures.append(f"complete FunctionLexicon changed: expected 242 named entries, found {len(entries)}")
    if len(entry_rows) != len(entries):
        failures.append(f"FunctionLexicon contains {len(entry_rows) - len(entries)} duplicate named rows")
    if len(boundary) != 96:
        failures.append(f"online callback boundary changed: expected 96 exact names, found {len(boundary)}")
    if len(boundary_rows) != len(boundary):
        failures.append(f"online callback boundary contains {len(boundary_rows) - len(boundary)} duplicate names")
    if boundary_only != {"BuddyControlSystem"}:
        failures.append(f"unexpected non-lexicon online callback closure: {sorted(boundary_only)}")
    if online_entries & offline_entries or online_entries | offline_entries != entries:
        failures.append("online/offline FunctionLexicon partitions overlap or have a gap")

    with tempfile.TemporaryDirectory(prefix="m20-online-boundary-") as directory:
        boundary_object = pathlib.Path(directory) / "linux_online_callback_boundary.cpp.o"
        boundary_object.write_bytes(subprocess.run(
            ["ar", "p", str(args.archive), boundary_object.name], check=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout)
        boundary_symbols = subprocess.run(
            ["nm", "-C", "--defined-only", str(boundary_object)], check=True, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout
    executable_symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout
    for name in sorted(boundary):
        if not re.search(rf"\b{re.escape(name)}\(", boundary_symbols):
            failures.append(f"online callback lacks its exact fail-closed provider: {name}")
        if not re.search(rf"\b{re.escape(name)}\(", executable_symbols):
            failures.append(f"online callback is not live in the production executable: {name}")
    for name in sorted(entries):
        from_boundary = bool(re.search(rf"\b{re.escape(name)}\(", boundary_symbols))
        if name not in boundary and from_boundary:
            failures.append(f"offline callback was replaced by the online boundary: {name}")
        if not re.search(rf"\b{re.escape(name)}\(", executable_symbols):
            failures.append(f"complete-table callback is not live in the production executable: {name}")

    included = args.link_map.read_text(encoding="utf-8", errors="replace").split(
        "Discarded input sections", 1)[0]
    if "linux_online_callback_boundary.cpp.o" not in included:
        failures.append("online callback boundary object was not extracted live")
    inventory = inventory_path.read_text(encoding="utf-8")
    absent_sdk = [line for line in inventory.splitlines()
                  if "Libraries/Source/GameSpy" in line and
                  "absent from the repository or outside its source boundary" in line]
    if len(absent_sdk) < 10:
        failures.append("authoritative legacy inventory no longer proves the excluded GameSpy SDK boundary")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"M20 production FunctionLexicon: ok unique-entries={len(entries)} duplicate-rows=0 "
          f"online-lexicon={len(online_entries)} offline-lexicon={len(offline_entries)} "
          f"online-closure={len(boundary)} boundary-only=BuddyControlSystem "
          f"excluded-sdk-records={len(absent_sdk)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
