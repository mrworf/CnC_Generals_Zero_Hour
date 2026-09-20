#!/usr/bin/env python3
import argparse
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile


def source_object(commands: list[dict], source: pathlib.Path, target: str) -> pathlib.Path:
    marker = f"CMakeFiles/{target}.dir/"
    for entry in commands:
        candidate = pathlib.Path(entry.get("file", ""))
        if not candidate.is_absolute():
            candidate = pathlib.Path(entry.get("directory", ".")) / candidate
        if candidate.resolve() != source.resolve() or marker not in pathlib.Path(entry.get("output", "")).as_posix():
            continue
        output = pathlib.Path(entry["output"])
        return output if output.is_absolute() else pathlib.Path(entry["directory"]) / output
    raise RuntimeError(f"missing active object for {source} in {target}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--archiver", required=True)
    parser.add_argument("--compile-commands", type=pathlib.Path, required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--dispatch-source", type=pathlib.Path, required=True)
    parser.add_argument("--dispatch-target", required=True)
    parser.add_argument("--probe-source", type=pathlib.Path, required=True)
    parser.add_argument("--probe-target", required=True)
    parser.add_argument("--provider-archive", type=pathlib.Path, required=True)
    parser.add_argument("--member", default="Chat.cpp.o")
    parser.add_argument("--required-symbol", action="append", default=[])
    parser.add_argument("--library", action="append", type=pathlib.Path, default=[])
    args = parser.parse_args()

    included = args.link_map.read_text(encoding="utf-8", errors="replace").split("Discarded input sections", 1)[0]
    if f"libzh_original_config_providers.a({args.member})" not in included:
        print("selected removal member was not extracted by the passing executable", file=sys.stderr)
        return 1

    commands = json.loads(args.compile_commands.read_text(encoding="utf-8"))
    dispatch_object = source_object(commands, args.dispatch_source, args.dispatch_target)
    probe_object = source_object(commands, args.probe_source, args.probe_target)
    with tempfile.TemporaryDirectory(prefix="m20-config-provider-removal-") as directory:
        temporary = pathlib.Path(directory)
        archive = temporary / args.provider_archive.name
        binary = temporary / "probe-with-provider-removed"
        shutil.copy2(args.provider_archive, archive)
        removed = subprocess.run(
            [args.archiver, "d", str(archive), args.member], check=False, text=True,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if removed.returncode:
            print(f"could not remove extracted provider: {removed.stderr}", file=sys.stderr)
            return 1
        result = subprocess.run(
            [args.compiler, "-Wl,--gc-sections", str(dispatch_object), str(probe_object),
             "-o", str(binary), str(archive), *(str(path) for path in args.library)],
            check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if result.returncode == 0:
            print(f"real probe linked after extracted {args.member} provider removal", file=sys.stderr)
            return 1
        required_symbols = args.required_symbol or ["parseOnlineChatColorDefinition", "GameSpyColor"]
        if not any(symbol in result.stderr for symbol in required_symbols):
            print("negative link failed for an unrelated reason", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1
    print(f"M20 provider-removal link control: ok member={args.member}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
