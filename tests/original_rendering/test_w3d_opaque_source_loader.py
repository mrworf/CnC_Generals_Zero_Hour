#!/usr/bin/env python3
"""Owned positive/negative original-source loader control for opaque W3D audit."""

import argparse
from collections import Counter
from pathlib import Path
import struct
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from audit_w3d_chunk_families import audit_archives, classify_opaque, source_wwshade_requirement
from test_w3d_chunk_family_audit import archive


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-loader", type=Path, required=True)
    args = parser.parse_args()
    with TemporaryDirectory(prefix="zh-w3d-opaque-source-") as scratch:
        owned = Path(scratch) / "owned.w3d"
        subprocess.run([str(args.source_loader), "--emit", str(owned)],
                       check=True, capture_output=True, timeout=15)
        valid = owned.read_bytes()
        malformed = struct.pack("<II", 0x7FFF_FFFE, 0x8000_0010)
        if classify_opaque(valid, args.source_loader) != "accepted":
            raise SystemExit("original W3D loader rejected the owned valid packet")
        if classify_opaque(malformed, args.source_loader) != "rejected":
            raise SystemExit("original W3D loader accepted malformed opaque input")
        big = Path(scratch) / "owned.big"
        big.write_bytes(archive([(b"Art/valid.w3d", valid),
                                 (b"Art/opaque.w3d", malformed)]))
        source_results: Counter[str] = Counter()
        opaque_context: Counter[str] = Counter()
        entries, chunks, opaque = audit_archives(
            [big], report_opaque=True, loader=args.source_loader,
            loader_counts=source_results, opaque_context=opaque_context,
        )
        if (entries != 2 or not chunks or opaque != 1 or
                source_results != Counter({"rejected": 1}) or
                opaque_context["shdmesh_candidates"]):
            raise SystemExit("opaque BIG classification did not isolate the original source rejection")
        if source_wwshade_requirement(chunks, opaque, opaque_context, source_results) != "no":
            raise SystemExit("rejected opaque payload did not close selected-archive WWShade absence")
        if source_wwshade_requirement(Counter(), 1, Counter({"shdmesh_candidates": 1}),
                                      Counter({"rejected": 1})) != "unknown":
            raise SystemExit("opaque SHDMESH candidate was silently treated as absent")
        if source_wwshade_requirement(Counter({0x0B00: 1}), 0, Counter(), Counter()) != "yes":
            raise SystemExit("reached SHDMESH was not required")
    print("owned W3D source-loader positive, opaque negative and aggregate isolation: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
