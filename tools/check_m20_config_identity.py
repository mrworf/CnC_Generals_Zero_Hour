#!/usr/bin/env python3
import argparse
import json
import pathlib
import re
import subprocess
import sys


def target_outputs(commands: list[dict], target: str) -> dict[str, list[pathlib.Path]]:
    marker = f"CMakeFiles/{target}.dir/"
    outputs: dict[str, list[pathlib.Path]] = {}
    for entry in commands:
        output = pathlib.Path(entry.get("output", ""))
        if marker not in output.as_posix():
            continue
        outputs.setdefault(output.name, []).append(pathlib.Path(entry["file"]).resolve())
    return outputs


def source_output(commands: list[dict], source: pathlib.Path, target: str) -> pathlib.Path | None:
    marker = f"CMakeFiles/{target}.dir/"
    for entry in commands:
        candidate = pathlib.Path(entry.get("file", ""))
        if not candidate.is_absolute():
            candidate = pathlib.Path(entry.get("directory", ".")) / candidate
        if candidate.resolve() == source.resolve() and marker in pathlib.Path(entry.get("output", "")).as_posix():
            output = pathlib.Path(entry["output"])
            return output if output.is_absolute() else pathlib.Path(entry["directory"]) / output
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--dispatch-source", type=pathlib.Path, required=True)
    parser.add_argument("--dispatch-target", required=True)
    parser.add_argument("--provider-target", required=True)
    parser.add_argument("--provider-archive", type=pathlib.Path, required=True)
    parser.add_argument("--fixture", type=pathlib.Path, required=True)
    args = parser.parse_args()

    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    dispatch_object = source_output(commands, args.dispatch_source, args.dispatch_target)
    failures: list[str] = []
    if dispatch_object is None or not dispatch_object.is_file():
        failures.append("missing exact active production INI.cpp dispatch object")
        dispatch_symbols = ""
    else:
        dispatch_symbols = subprocess.run(
            ["nm", "-C", "--defined-only", str(dispatch_object)], check=True, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        ).stdout

    source_text = args.dispatch_source.read_text(encoding="utf-8", errors="strict")
    table_match = re.search(
        r"static\s+const\s+BlockParse\s+theTypeTable\[\]\s*=\s*\{(.*?)\n\};",
        source_text,
        re.DOTALL,
    )
    if table_match is None:
        failures.append("production INI type table not found")
        callbacks: list[str] = []
    else:
        callbacks = re.findall(
            r'\{\s*"[^"]+"\s*,\s*([A-Za-z_]\w*(?:::[A-Za-z_]\w*)*)\s*\}',
            table_match.group(1),
        )
        if len(callbacks) < 60 or len(callbacks) != len(set(callbacks)):
            failures.append(f"unexpected production callback inventory: {len(callbacks)} entries")

    link_map = args.link_map.read_text(encoding="utf-8", errors="replace")
    included = link_map.split("Merging program properties", 1)[0]
    included = included.split("Discarded input sections", 1)[0]
    executable_symbols = subprocess.run(
        ["nm", "-C", "--defined-only", str(args.executable)], check=True, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout
    provider_outputs = target_outputs(commands, args.provider_target)
    archive_symbols = subprocess.run(
        ["nm", "-C", "--print-file-name", "--defined-only", str(args.provider_archive)],
        check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    ).stdout

    extracted_members: set[str] = set()
    for callback in callbacks:
        if f"{callback}(" not in executable_symbols:
            failures.append(f"missing live callback symbol: {callback}")
            continue
        if f"{callback}(" in dispatch_symbols:
            continue
        definitions = [line for line in archive_symbols.splitlines() if f"{callback}(" in line]
        members = {
            match.group(1)
            for line in definitions
            if (match := re.search(r":([^/:]+\.cpp\.o):", line)) is not None
        }
        if len(members) != 1:
            failures.append(f"callback provider identity is missing or ambiguous: {callback}")
            continue
        member = next(iter(members))
        if f"lib{args.provider_target}.a({member})" not in included:
            failures.append(f"callback provider was not extracted: {callback} from {member}")
            continue
        extracted_members.add(member)
        sources = provider_outputs.get(member, [])
        if len(sources) != 1:
            failures.append(f"provider member identity is ambiguous for {member}: {len(sources)} sources")

    for required in ("INI::load(", "theTypeTable"):
        if required not in executable_symbols:
            failures.append(f"missing live production dispatch symbol: {required}")
    if "Chat.cpp.o" not in extracted_members:
        failures.append("representative OnlineChatColors provider was not extracted")

    result = subprocess.run(
        [str(args.executable), "valid", str(args.fixture)], check=False, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    if result.returncode or "original-config dispatch: ok" not in result.stdout:
        failures.append(f"actual INI::load runtime witness absent (exit {result.returncode})")

    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"M20 complete config identity: ok callbacks={len(callbacks)} providers={len(extracted_members)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
