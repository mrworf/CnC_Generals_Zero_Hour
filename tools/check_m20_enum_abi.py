#!/usr/bin/env python3
import argparse
import csv
import pathlib
import re
import subprocess
import sys
import tempfile


def git_text(root: pathlib.Path, revision: str, path: str) -> str:
    return subprocess.run(["git", "show", f"{revision}:{path}"], cwd=root, check=True,
                          text=True, stdout=subprocess.PIPE).stdout


def enum_body(text: str, name: str) -> str | None:
    match = re.search(rf"(?:typedef\s+)?\benum\s+{re.escape(name)}(?:\s*:\s*int)?\s*\{{", text)
    if not match:
        return None
    start = text.find("{", match.start())
    depth = 0
    for offset, char in enumerate(text[start:], start):
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return re.sub(r"\s+", "", text[start:offset + 1])
    return None


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=pathlib.Path, required=True)
    parser.add_argument("--manifest", type=pathlib.Path, required=True)
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--baseline", default="4c79b0cbfc4f346cf2ae7875dd6a865b0a1a31ed")
    args = parser.parse_args()
    root = args.root.resolve()
    failures: list[str] = []
    names: list[str] = []
    with args.manifest.open(encoding="utf-8", newline="") as stream:
        rows = list(csv.DictReader(stream, delimiter="\t"))
    for row in rows:
        name = row["enum"]
        names.append(name)
        if row["legacy_abi"] != "int32-values-preserved":
            failures.append(f"{name}: invalid ABI classification")
        for path in row["declarations"].split(";"):
            current = (root / path).read_text(encoding="utf-8", errors="replace")
            baseline = git_text(root, args.baseline, path)
            if not re.search(rf"\benum\s+{re.escape(name)}\s*:\s*int\s*;", current):
                failures.append(f"{name}: missing explicit current declaration in {path}")
            if not re.search(rf"\benum\s+{re.escape(name)}\s*;", baseline):
                # A few declarations are newly explicit because the original
                # MSVC PCH supplied their type indirectly.  Accept those only
                # when the legacy tree contains the matching enum definition;
                # the definition body/value check below remains mandatory.
                definition_paths = row["definitions"].split(";")
                if row["definitions"] == "none-original-forward-only" or not any(
                    enum_body(git_text(root, args.baseline, definition), name)
                    for definition in definition_paths
                ):
                    failures.append(f"{name}: declaration absent from baseline {path}")
        if row["definitions"] == "none-original-forward-only":
            continue
        for path in row["definitions"].split(";"):
            current_body = enum_body((root / path).read_text(encoding="utf-8", errors="replace"), name)
            baseline_body = enum_body(git_text(root, args.baseline, path), name)
            if current_body is None or baseline_body is None:
                failures.append(f"{name}: definition missing in {path}")
            elif current_body != baseline_body:
                failures.append(f"{name}: enumerator values changed in {path}")
    with tempfile.TemporaryDirectory(prefix="m20-enum-abi-") as directory:
        source = pathlib.Path(directory) / "probe.cpp"
        source.write_text(
            '#include "PreRTS.h"\n'
            f'#include "{(root / "GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp").as_posix()}"\n'
            f'#include "{(root / "GeneralsMD/Code/GameEngine/Include/Common/ThingSort.h").as_posix()}"\n'
            f'#include "{(root / "GeneralsMD/Code/GameEngine/Include/GameLogic/Module/ProductionUpdate.h").as_posix()}"\n'
            + "\n".join(f'static_assert(sizeof({name}) == sizeof(int), "{name} ABI");' for name in names)
            + "\n",
            encoding="utf-8",
        )
        command = [
            args.compiler, "-std=c++17", "-fsyntax-only",
            f"-I{root / 'src/original_runtime/include'}",
            f"-I{root / 'GeneralsMD/Code/GameEngine/Include'}",
            f"-I{root / 'GeneralsMD/Code/Libraries/Include'}",
            f"-I{root / 'GeneralsMD/Code/Libraries/Source/WWVegas'}",
            f"-I{root / 'GeneralsMD/Code/Libraries/Source/WWVegas/WWLib'}",
            str(source),
        ]
        result = subprocess.run(command, check=False, text=True,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode:
            failures.append("enum size probe failed:\n" + result.stderr[-4000:])
    if failures:
        print("\n".join(failures), file=sys.stderr)
        return 1
    print(f"M20 original enum ABI: ok enums={len(names)} size=int values=baseline")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
