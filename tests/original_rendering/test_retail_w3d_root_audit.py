#!/usr/bin/env python3
"""Owned two-root coverage and loose-W3D negative for the retail family gate."""

import argparse
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from audit_retail_w3d_roots import audit_roots
from audit_w3d_chunk_families import InvalidArchive, source_wwshade_requirement
from test_w3d_chunk_family_audit import archive, chunk


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-loader", type=Path, required=True)
    args = parser.parse_args()
    with TemporaryDirectory(prefix="zh-retail-w3d-roots-") as scratch:
        base = Path(scratch) / "base"
        zh = Path(scratch) / "zh"
        base.mkdir()
        zh.mkdir()
        (zh / "nested").mkdir()
        (base / "primary.big").write_bytes(archive([(b"Art/rigid.w3d", chunk(0x0000))]))
        (zh / "primary.big").write_bytes(archive([(b"Art/hlod.w3d", chunk(0x0700))]))
        (zh / "nested/secondary.big").write_bytes(archive([(b"Art/shader.w3d", chunk(0x0B00))]))
        archives, entries, counts, opaque, loader, context = audit_roots(base, zh, args.source_loader)
        if (archives != 3 or entries != 3 or counts[0x0B00] != 1 or opaque or
                source_wwshade_requirement(counts, opaque, context, loader) != "yes"):
            raise SystemExit("nested archive SHDMESH was omitted from two-root aggregate")
        (zh / "loose.w3d").write_bytes(chunk(0x0B00))
        try:
            audit_roots(base, zh, args.source_loader)
        except InvalidArchive as error:
            if "loose W3D" not in str(error):
                raise
        else:
            raise SystemExit("loose W3D source silently bypassed archive aggregate")
        (zh / "loose.w3d").unlink()
        try:
            audit_roots(base, base, args.source_loader)
        except InvalidArchive:
            pass
        else:
            raise SystemExit("duplicate retail roots were accepted")
    print("owned two-root nested W3D family and loose-file rejection: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
