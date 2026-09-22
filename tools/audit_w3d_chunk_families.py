#!/usr/bin/env python3
"""Read-only, name-free aggregate of top-level W3D chunks in BIG archives."""

import argparse
from collections import Counter
import mmap
import os
from pathlib import Path
import struct
import sys


MAX_ENTRIES = 1_000_000
MAX_TABLE_BYTES = 128 * 1024 * 1024
MAX_NAME_BYTES = 4096


class InvalidArchive(ValueError):
    pass


def audit_archive(path: Path, report_opaque: bool = False) -> tuple[int, Counter[int], int]:
    with path.open("rb") as stream:
        size = os.fstat(stream.fileno()).st_size
        if size < 16:
            raise InvalidArchive("BIG header is truncated")
        with mmap.mmap(stream.fileno(), 0, access=mmap.ACCESS_READ) as data:
            if data[:4] not in (b"BIGF", b"BIG4"):
                raise InvalidArchive("BIG identifier is unsupported")
            declared_size = struct.unpack_from("<I", data, 4)[0]
            count, table_end = struct.unpack_from(">II", data, 8)
            if declared_size != size or count > MAX_ENTRIES or not 16 <= table_end <= size:
                raise InvalidArchive("BIG header range is invalid")
            if table_end - 16 > MAX_TABLE_BYTES:
                raise InvalidArchive("BIG table exceeds limit")
            cursor = 16
            w3d_entries = 0
            opaque_entries = 0
            chunks: Counter[int] = Counter()
            for _ in range(count):
                if cursor + 9 > table_end:
                    raise InvalidArchive("BIG entry table is truncated")
                offset, length = struct.unpack_from(">II", data, cursor)
                cursor += 8
                end_name = data.find(b"\0", cursor, table_end)
                if end_name < 0 or not 0 < end_name - cursor <= MAX_NAME_BYTES:
                    raise InvalidArchive("BIG entry name range is invalid")
                is_w3d = data[cursor:end_name].lower().endswith(b".w3d")
                cursor = end_name + 1
                if offset < table_end or offset > size or length > size - offset:
                    raise InvalidArchive("BIG entry payload range is invalid")
                if not is_w3d:
                    continue
                w3d_entries += 1
                position = offset
                payload_end = offset + length
                found: Counter[int] = Counter()
                while position < payload_end:
                    if payload_end - position < 8:
                        if not report_opaque:
                            raise InvalidArchive("W3D top-level chunk header is truncated")
                        opaque_entries += 1
                        break
                    kind, encoded_length = struct.unpack_from("<II", data, position)
                    chunk_length = encoded_length & 0x7FFF_FFFF
                    position += 8
                    if chunk_length > payload_end - position:
                        if not report_opaque:
                            raise InvalidArchive("W3D top-level chunk range is invalid")
                        opaque_entries += 1
                        break
                    found[kind] += 1
                    position += chunk_length
                else:
                    chunks.update(found)
            return w3d_entries, chunks, opaque_entries


def audit_archives(paths: list[Path], report_opaque: bool = False) -> tuple[int, Counter[int], int]:
    entries = 0
    opaque = 0
    chunks: Counter[int] = Counter()
    for path in paths:
        found, family, excluded = audit_archive(path, report_opaque)
        entries += found
        opaque += excluded
        chunks.update(family)
    if not entries:
        raise InvalidArchive("no W3D entries were found")
    return entries, chunks, opaque


def wwshade_requirement(chunks: Counter[int], opaque: int) -> str:
    return "yes" if chunks[0x0B00] else ("unknown" if opaque else "no")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, action="append", required=True)
    parser.add_argument("--report-opaque", action="store_true",
                        help="count malformed W3D payloads explicitly instead of failing the aggregate")
    args = parser.parse_args()
    try:
        entries, chunks, opaque = audit_archives(args.archive, args.report_opaque)
    except InvalidArchive as error:
        # No caller-supplied path or private BIG entry name is echoed.
        print(f"W3D family audit failed: {error}", file=sys.stderr)
        return 1
    except OSError:
        print("W3D family audit failed: input archive is unavailable", file=sys.stderr)
        return 1
    families = " ".join(f"type-0x{kind:04x}={count}" for kind, count in sorted(chunks.items()))
    print(f"archives={len(args.archive)} w3d_entries={entries} opaque_w3d_entries={opaque} "
          f"top_level_chunks={sum(chunks.values())} {families}")
    print(f"wwshade_required={wwshade_requirement(chunks, opaque)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
