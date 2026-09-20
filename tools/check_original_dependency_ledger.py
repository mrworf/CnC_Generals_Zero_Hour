#!/usr/bin/env python3
import argparse
import csv
import hashlib
import pathlib
import sys


COLUMNS = [
    "source", "sha256", "symbol", "consumer", "lifecycle_or_config",
    "provider_target", "callback_or_vtable", "global_edges", "logical_assets",
    "writes", "owner", "evidence_grade", "tests",
]
GRADES = {"inspected", "compile", "link", "runtime"}
OWNERS = {"M26", "M27", "M28", "M20", "M21", "M22", "M23", "M24", "M25"}


def validate(root: pathlib.Path, ledger: pathlib.Path) -> list[str]:
    errors: list[str] = []
    with ledger.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream, delimiter="\t")
        if reader.fieldnames != COLUMNS:
            return [f"ledger columns differ: {reader.fieldnames!r}"]
        rows = list(reader)
    if not rows:
        return ["ledger has no dependency edges"]
    cmake_path = root / "CMakeLists.txt"
    if cmake_path.is_file():
        cmake = cmake_path.read_text(encoding="utf-8")
        if "target_compile_definitions(zh_original_process PRIVATE NDEBUG" in cmake:
            errors.append("zh_original_process has an unconditional NDEBUG workaround")
    for line, row in enumerate(rows, start=2):
        missing = [column for column in COLUMNS if not row[column].strip()]
        if missing:
            errors.append(f"line {line}: empty required fields: {','.join(missing)}")
            continue
        source = root / row["source"]
        if not source.is_file():
            errors.append(f"line {line}: missing source {row['source']}")
        else:
            digest = hashlib.sha256(source.read_bytes()).hexdigest()
            if digest != row["sha256"]:
                errors.append(f"line {line}: source drift {row['source']}")
        if row["owner"] not in OWNERS:
            errors.append(f"line {line}: ownerless/unknown owner {row['owner']}")
        if row["evidence_grade"] not in GRADES:
            errors.append(f"line {line}: invalid evidence grade {row['evidence_grade']}")
        if row["owner"] == "M26" and row["evidence_grade"] != "runtime":
            errors.append(f"line {line}: M26 provider lacks runtime evidence")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=pathlib.Path, required=True)
    parser.add_argument("--ledger", type=pathlib.Path, required=True)
    args = parser.parse_args()
    errors = validate(args.root.resolve(), args.ledger.resolve())
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("original dependency ledger: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
