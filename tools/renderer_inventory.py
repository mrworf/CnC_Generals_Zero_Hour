#!/usr/bin/env python3
"""Fail-closed inventory of legacy D3D/D3DX identifiers."""

from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RULES = ROOT / "docs/renderer/legacy-api-mapping.tsv"
SOURCE_ROOTS = (ROOT / "GeneralsMD/Code", ROOT / "Generals/Code")
SOURCE_SUFFIXES = frozenset({".c", ".cc", ".cpp", ".h", ".hpp"})
TOKEN_PATTERN = re.compile(r"\b(?:D3D[A-Za-z0-9_]*|D3DX[A-Za-z0-9_]*|IDirect3D[A-Za-z0-9_]*|Direct3D[A-Za-z0-9_]*)\b")
REQUIRED_CATEGORIES = frozenset(
    {
        "formats",
        "primitives-fvf",
        "fixed-function-state",
        "texture-stage-state",
        "resources",
        "render-targets",
        "shader-assembly",
        "math",
        "image-and-font-helpers",
        "device-and-lifecycle",
        "lighting-and-materials",
        "legacy-wrapper",
    }
)
REQUIRED_DISPOSITIONS = frozenset(
    {"public-api", "repository-shader", "cpu-fallback", "compatibility-adapter"}
)


class InventoryError(RuntimeError):
    pass


@dataclass(frozen=True)
class Rule:
    rule_id: str
    pattern_text: str
    pattern: re.Pattern[str]
    category: str
    disposition: str
    target: str
    rationale: str


def load_rules(path: Path) -> list[Rule]:
    required = {"rule_id", "pattern", "category", "disposition", "target", "rationale"}
    try:
        with path.open(encoding="utf-8", newline="") as stream:
            reader = csv.DictReader(stream, delimiter="\t")
            if set(reader.fieldnames or ()) != required:
                raise InventoryError(f"{path}: expected columns {sorted(required)}")
            rules: list[Rule] = []
            for line, row in enumerate(reader, start=2):
                missing = [name for name in required if not (row.get(name) or "").strip()]
                if missing:
                    raise InventoryError(f"{path}:{line}: empty fields: {', '.join(sorted(missing))}")
                try:
                    pattern = re.compile(row["pattern"])
                except re.error as error:
                    raise InventoryError(f"{path}:{line}: invalid pattern: {error}") from error
                rules.append(
                    Rule(
                        row["rule_id"], row["pattern"], pattern, row["category"],
                        row["disposition"], row["target"], row["rationale"],
                    )
                )
    except OSError as error:
        raise InventoryError(f"cannot read mapping rules {path}: {error}") from error
    duplicate_ids = [name for name, count in Counter(rule.rule_id for rule in rules).items() if count > 1]
    if duplicate_ids:
        raise InventoryError(f"duplicate rule ids: {', '.join(sorted(duplicate_ids))}")
    return rules


def scan_tokens(roots: tuple[Path, ...] = SOURCE_ROOTS) -> set[str]:
    tokens: set[str] = set()
    for root in roots:
        if not root.is_dir():
            raise InventoryError(f"missing source root: {root}")
        for path in sorted(root.rglob("*")):
            if path.is_file() and path.suffix.lower() in SOURCE_SUFFIXES:
                try:
                    tokens.update(TOKEN_PATTERN.findall(path.read_text(encoding="utf-8", errors="ignore")))
                except OSError as error:
                    raise InventoryError(f"cannot read source {path}: {error}") from error
    if not tokens:
        raise InventoryError("renderer inventory found no D3D/D3DX identifiers")
    return tokens


def classify(tokens: set[str], rules: list[Rule]) -> dict[str, Rule]:
    result: dict[str, Rule] = {}
    failures: list[str] = []
    for token in sorted(tokens):
        matches = [rule for rule in rules if rule.pattern.fullmatch(token)]
        if len(matches) != 1:
            label = "unmapped" if not matches else "ambiguous: " + ", ".join(rule.rule_id for rule in matches)
            failures.append(f"{token}: {label}")
        else:
            result[token] = matches[0]
    if failures:
        raise InventoryError("inventory classification failed:\n  " + "\n  ".join(failures))
    return result


def validate_coverage(classification: dict[str, Rule]) -> None:
    categories = {rule.category for rule in classification.values()}
    dispositions = {rule.disposition for rule in classification.values()}
    missing_categories = REQUIRED_CATEGORIES - categories
    missing_dispositions = REQUIRED_DISPOSITIONS - dispositions
    if missing_categories or missing_dispositions:
        details = []
        if missing_categories:
            details.append("missing categories: " + ", ".join(sorted(missing_categories)))
        if missing_dispositions:
            details.append("missing dispositions: " + ", ".join(sorted(missing_dispositions)))
        raise InventoryError("; ".join(details))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--check", action="store_true", help="validate the checked-in source inventory")
    parser.add_argument("--rules", type=Path, default=DEFAULT_RULES)
    parser.add_argument("--source-root", type=Path, action="append")
    args = parser.parse_args()
    try:
        roots = tuple(args.source_root) if args.source_root else SOURCE_ROOTS
        rules = load_rules(args.rules)
        classification = classify(scan_tokens(roots), rules)
        validate_coverage(classification)
    except InventoryError as error:
        print(f"renderer inventory error: {error}", file=sys.stderr)
        return 1
    counts = Counter(rule.category for rule in classification.values())
    print(f"renderer inventory: {len(classification)} identifiers, {len(rules)} mapping rules")
    for category in sorted(counts):
        print(f"  {category}: {counts[category]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
