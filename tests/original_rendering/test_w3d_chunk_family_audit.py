#!/usr/bin/env python3
"""Owned BIG/W3D positive and malformed-range controls for the retail audit."""

from collections import Counter
from pathlib import Path
import subprocess
import struct
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
from audit_w3d_chunk_families import InvalidArchive, audit_archives, wwshade_requirement


def chunk(kind: int, payload: bytes = b"", nested: bool = False) -> bytes:
    return struct.pack("<II", kind, len(payload) | (0x8000_0000 if nested else 0)) + payload


def archive(entries: list[tuple[bytes, bytes]]) -> bytes:
    table_size = sum(9 + len(name) for name, _ in entries)
    data_offset = 16 + table_size
    table = bytearray()
    payload = bytearray()
    for name, contents in entries:
        table += struct.pack(">II", data_offset + len(payload), len(contents)) + name + b"\0"
        payload += contents
    result = bytearray(b"BIGF" + b"\0" * 12)
    result += table + payload
    struct.pack_into("<I", result, 4, len(result))
    struct.pack_into(">II", result, 8, len(entries), 16 + len(table))
    return bytes(result)


def rejected(path: Path, contents: bytes) -> bool:
    path.write_bytes(contents)
    try:
        audit_archives([path])
    except InvalidArchive:
        return True
    return False


def main() -> int:
    with TemporaryDirectory(prefix="zh-w3d-family-audit-") as scratch:
        path = Path(scratch) / "owned.big"
        good = archive([
            (b"ART/ONE.W3D", chunk(0x0000, chunk(0x001F), True) + chunk(0x0700)),
            (b"Art/Two.w3d", chunk(0x0B00, chunk(0x0B01), True)),
            (b"Data/ignored.bin", b"not W3D"),
        ])
        path.write_bytes(good)
        entries, counts, opaque = audit_archives([path])
        if entries != 2 or opaque or counts != Counter({0x0000: 1, 0x0700: 1, 0x0B00: 1}):
            raise SystemExit("owned W3D family aggregate is incorrect")
        if wwshade_requirement(counts, opaque) != "yes" or wwshade_requirement(Counter(), 0) != "no":
            raise SystemExit("WWShade requirement signal is incorrect")
        empty = bytearray(archive([(b"valid.w3d", chunk(0x0000)), (b"ignored.bin", b"")]))
        struct.pack_into(">I", empty, 16 + 9 + len(b"valid.w3d"), 0)
        path.write_bytes(empty)
        entries, counts, opaque = audit_archives([path])
        if entries != 1 or counts != Counter({0x0000: 1}) or opaque:
            raise SystemExit("source-compatible zero-length BIG entry was rejected")
        if not rejected(path, good[:12]):
            raise SystemExit("truncated BIG header was accepted")
        bad_range = bytearray(good)
        struct.pack_into(">I", bad_range, 16, len(good) + 1)
        if not rejected(path, bytes(bad_range)):
            raise SystemExit("out-of-range BIG entry was accepted")
        bad_name = bytearray(good)
        struct.pack_into(">I", bad_name, 12, 25)
        if not rejected(path, bytes(bad_name)):
            raise SystemExit("unterminated BIG entry name was accepted")
        bad_chunk = archive([(b"bad.w3d", struct.pack("<II", 0x0B00, 0x8000_0010))])
        if not rejected(path, bad_chunk):
            raise SystemExit("out-of-range W3D top-level chunk was accepted")
        path.write_bytes(bad_chunk)
        entries, counts, opaque = audit_archives([path], report_opaque=True)
        if entries != 1 or counts or opaque != 1:
            raise SystemExit("explicit opaque W3D classification is incorrect")
        if wwshade_requirement(counts, opaque) != "unknown":
            raise SystemExit("opaque W3D requirement was silently reported absent")
        prefix = archive([(b"prefix.w3d", chunk(0x0B00) +
                           struct.pack("<II", 0x7FFF_FFFE, 16))])
        path.write_bytes(prefix)
        context: Counter[str] = Counter()
        entries, counts, opaque = audit_archives(
            [path], report_opaque=True, opaque_context=context)
        if entries != 1 or counts or opaque != 1 or context["shdmesh_candidates"] != 1:
            raise SystemExit("opaque W3D valid-prefix shader candidate was hidden")
        if not rejected(path, archive([(b"other.bin", b"ignored")])):
            raise SystemExit("archive without W3D entries was accepted")
        unavailable = Path(scratch) / "private-input-must-not-echo.big"
        process = subprocess.run(
            [sys.executable, str(Path(__file__).resolve().parents[2] /
                             "tools/audit_w3d_chunk_families.py"), "--archive", str(unavailable)],
            capture_output=True, text=True,
        )
        if process.returncode == 0 or unavailable.name in process.stderr:
            raise SystemExit("unavailable archive path leaked through diagnostic")
    print("owned W3D chunk-family aggregate and malformed bounds: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
