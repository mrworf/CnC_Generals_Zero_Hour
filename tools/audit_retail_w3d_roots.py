#!/usr/bin/env python3
"""Name-free W3D family audit over two explicitly supplied retail roots."""

import argparse
from collections import Counter
from pathlib import Path
import sys

from audit_w3d_chunk_families import (
    InvalidArchive, audit_archive, source_wwshade_requirement,
)


def audit_roots(base: Path, zh: Path, source_loader: Path):
    if not base.is_dir() or not zh.is_dir() or base.samefile(zh):
        raise InvalidArchive("two distinct retail root directories are required")
    archives = sorted(
        (entry for root in (base, zh) for entry in root.rglob("*")
         if entry.is_file() and entry.suffix.lower() == ".big"),
        key=lambda path: (str(path.parent), path.name.lower()),
    )
    if not archives:
        raise InvalidArchive("no BIG archives were found in the supplied roots")
    loose = sum(1 for root in (base, zh) for entry in root.rglob("*")
                if entry.is_file() and entry.suffix.lower() == ".w3d")
    if loose:
        raise InvalidArchive("loose W3D files require separate scope classification")
    loader_counts: Counter[str] = Counter()
    opaque_context: Counter[str] = Counter()
    entries = 0
    opaque = 0
    chunks: Counter[int] = Counter()
    for ordinal, path in enumerate(archives, 1):
        try:
            found, family, excluded = audit_archive(
                path, report_opaque=True, loader=source_loader,
                loader_counts=loader_counts, opaque_context=opaque_context,
            )
        except InvalidArchive as error:
            raise InvalidArchive(f"archive ordinal {ordinal}: {error}") from None
        entries += found
        opaque += excluded
        chunks.update(family)
    if not entries:
        raise InvalidArchive("no W3D entries were found in the supplied roots")
    return len(archives), entries, chunks, opaque, loader_counts, opaque_context


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-root", type=Path, required=True)
    parser.add_argument("--zh-root", type=Path, required=True)
    parser.add_argument("--source-loader", type=Path, required=True)
    args = parser.parse_args()
    try:
        archives, entries, chunks, opaque, loader_counts, context = audit_roots(
            args.base_root, args.zh_root, args.source_loader)
    except InvalidArchive as error:
        print(f"retail W3D root audit failed: {error}", file=sys.stderr)
        return 1
    except OSError:
        print("retail W3D root audit failed: input root or archive is unavailable", file=sys.stderr)
        return 1
    requirement = source_wwshade_requirement(chunks, opaque, context, loader_counts)
    print(f"roots=2 archives={archives} loose_w3d=0 w3d_entries={entries} "
          f"opaque_w3d_entries={opaque} top_level_chunks={sum(chunks.values())}")
    print(f"shdmesh={chunks[0x0B00]} opaque_shdmesh_candidates={context['shdmesh_candidates']} "
          f"opaque_loader_accepted={loader_counts['accepted']} "
          f"opaque_loader_rejected={loader_counts['rejected']} "
          f"opaque_loader_unresolved={loader_counts['unresolved']}")
    print(f"source_wwshade_required={requirement}")
    return 2 if requirement == "unknown" else 0


if __name__ == "__main__":
    raise SystemExit(main())
