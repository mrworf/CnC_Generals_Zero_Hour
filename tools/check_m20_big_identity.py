#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys


def source_object(commands: list[dict], source: pathlib.Path, target: str) -> pathlib.Path | None:
    marker = f"CMakeFiles/{target}.dir/"
    for entry in commands:
        candidate = pathlib.Path(entry["file"])
        if not candidate.is_absolute():
            candidate = pathlib.Path(entry["directory"]) / candidate
        if candidate.resolve() != source.resolve() or marker not in entry.get("output", ""):
            continue
        output = pathlib.Path(entry["output"])
        return output if output.is_absolute() else pathlib.Path(entry["directory"]) / output
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--source", type=pathlib.Path, required=True)
    parser.add_argument("--production-map", type=pathlib.Path, required=True)
    parser.add_argument("--test-map", type=pathlib.Path, required=True)
    parser.add_argument("--production", type=pathlib.Path, required=True)
    parser.add_argument("--test", type=pathlib.Path, required=True)
    args = parser.parse_args()

    failures: list[str] = []
    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    output = source_object(commands, args.source, "zh_original_config_providers")
    if output is None or not output.is_file():
        failures.append("production BIG adapter object is absent")
    for path, role in ((args.production_map, "production"), (args.test_map, "focused test")):
        included = path.read_text(encoding="utf-8", errors="replace").split("Discarded input sections", 1)[0]
        if "libzh_original_config_providers.a(linux_big_archive.cpp.o)" not in included:
            failures.append(f"BIG adapter was not extracted by {role} link")
    for executable, role in ((args.production, "production"), (args.test, "focused test")):
        symbols = subprocess.run(
            ["nm", "-C", "--defined-only", str(executable)], check=True, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        ).stdout
        if "zh::original_runtime::createLinuxBIGArchiveFileSystem()" not in symbols:
            failures.append(f"BIG adapter factory absent from {role} executable")

    source = args.source.read_text(encoding="utf-8")
    for required in (
        "invalid BIG archive entry bounds", "duplicate BIG archive logical path",
        "BIG archive table size mismatch", "rebuildDirectoryTree", "m_loadOrder",
    ):
        if required not in source:
            failures.append(f"BIG boundary check missing: {required}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M20 BIG identity: ok production-source/live-symbol/removal boundary")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
