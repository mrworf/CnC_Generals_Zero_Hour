#!/usr/bin/env python3
import argparse
import pathlib
import shlex
import shutil
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--build-dir", type=pathlib.Path, required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--link-map", type=pathlib.Path, required=True)
    parser.add_argument("--provider-archive", type=pathlib.Path, required=True)
    parser.add_argument("--member", required=True)
    parser.add_argument("--required-symbol", required=True)
    args = parser.parse_args()

    included = args.link_map.read_text(encoding="utf-8", errors="replace").split(
        "Discarded input sections", 1
    )[0]
    if f"libzh_original_config_providers.a({args.member})" not in included:
        print("selected provider was not extracted by the passing executable", file=sys.stderr)
        return 1
    commands = subprocess.run(
        ["ninja", "-t", "commands", args.target], cwd=args.build_dir, check=True,
        text=True, stdout=subprocess.PIPE,
    ).stdout.splitlines()
    archive_text = str(args.provider_archive)
    archive_token = args.provider_archive.name
    link = next((line for line in reversed(commands)
                 if archive_token in line and f" -o {args.target} " in line), None)
    if link is None:
        print("could not identify the active lifecycle link command", file=sys.stderr)
        return 1
    with tempfile.TemporaryDirectory(prefix="m20-headless-provider-removal-") as directory:
        temporary = pathlib.Path(directory)
        archive = temporary / args.provider_archive.name
        binary = temporary / "headless-with-provider-removed"
        shutil.copy2(args.provider_archive, archive)
        subprocess.run(["ar", "d", str(archive), args.member], check=True)
        segment = next(part.strip() for part in link.split("&&")
                       if archive_token in part and f" -o {args.target} " in part)
        argv = shlex.split(segment)
        argv = [str(archive) if value in (archive_text, archive_token) else value for value in argv]
        # The negative relink must not overwrite the passing executable's map;
        # later identity checks use that map as evidence of archive extraction.
        negative_map = temporary / "headless-with-provider-removed.link.map"
        argv = [
            (f"-Wl,-Map,{negative_map}" if value.startswith("-Wl,-Map,") else
             f"-Wl,-Map={negative_map}" if value.startswith("-Wl,-Map=") else value)
            for value in argv
        ]
        output_index = argv.index("-o") + 1
        argv[output_index] = str(binary)
        result = subprocess.run(argv, cwd=args.build_dir, check=False, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode == 0:
            print("lifecycle linked after removal of a live required provider", file=sys.stderr)
            return 1
        if args.required_symbol not in result.stderr:
            print("negative link failed for an unrelated reason", file=sys.stderr)
            print(result.stderr, file=sys.stderr)
            return 1
    print(f"M20 headless provider-removal: ok member={args.member}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
