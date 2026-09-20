#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys


def target_object(commands: list[dict], source: pathlib.Path, target: str) -> str | None:
    source = source.resolve()
    for entry in commands:
        candidate = pathlib.Path(entry.get("file", ""))
        if not candidate.is_absolute():
            candidate = pathlib.Path(entry.get("directory", ".")) / candidate
        if candidate.resolve() == source:
            output = pathlib.Path(entry.get("output", ""))
            marker = f"CMakeFiles/{target}.dir/"
            normalized = output.as_posix()
            if marker in normalized:
                return output.name
    return None


def archive_member_included(link_map: str, target: str, object_name: str) -> bool:
    # GNU and LLVM maps put members that were actually pulled from the archive
    # in this leading section. Mentions confined to discarded/debug sections do
    # not establish a contribution.
    included = link_map.split("Merging program properties", 1)[0]
    included = included.split("Discarded input sections", 1)[0]
    return f"lib{target}.a({object_name})" in included


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--source", action="append", default=[])
    parser.add_argument("--symbol", action="append", default=[])
    parser.add_argument("--witness", required=True)
    parser.add_argument("--argument", action="append", default=[])
    args = parser.parse_args()

    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    link_map = args.link_map.read_text(encoding="utf-8", errors="replace")
    failures: list[str] = []
    for value in args.source:
        source = pathlib.Path(value).resolve()
        object_name = target_object(commands, source, args.target)
        if object_name is None:
            failures.append(f"missing exact active {args.target} compile command: {source}")
        elif not archive_member_included(link_map, args.target, object_name):
            failures.append(f"source object was not pulled from {args.target}: {object_name}")

    symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    for symbol in args.symbol:
        if not any(symbol in line for line in symbols.splitlines()):
            failures.append(f"missing live defined symbol: {symbol}")

    result = subprocess.run(
        [str(args.executable), *args.argument], check=False, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode or args.witness not in result.stdout:
        failures.append(f"runtime witness absent (exit {result.returncode}): {args.witness}")
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print("M26 original identity: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
