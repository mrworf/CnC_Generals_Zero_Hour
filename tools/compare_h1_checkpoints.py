#!/usr/bin/env python3
"""Compare canonical H1 records from three original-runtime configurations."""

import hashlib
import subprocess
import sys

RECORD_BYTES = 224
RECORDS_PER_LIFETIME = 121
LIFETIMES = 3
PREFIX = b"H1_RECORD "


def validate(lines):
    if len(lines) != RECORDS_PER_LIFETIME * LIFETIMES + 1:
        raise ValueError("record count")
    if lines[-1] != b"NATIVE_ROOT h1-records PASS":
        raise ValueError("completion marker")
    records = []
    for ordinal, line in enumerate(lines[:-1]):
        if not line.startswith(PREFIX):
            raise ValueError("record prefix")
        try:
            record = bytes.fromhex(line[len(PREFIX):].decode("ascii"))
        except (UnicodeDecodeError, ValueError) as error:
            raise ValueError("record hex") from error
        if len(record) != RECORD_BYTES or record[:4] != b"H1R1":
            raise ValueError("record width/version")
        version = int.from_bytes(record[4:8], "little")
        tick = int.from_bytes(record[8:12], "little")
        frame = int.from_bytes(record[12:16], "little")
        if version != 1 or tick != ordinal % RECORDS_PER_LIFETIME or frame != tick + 1:
            raise ValueError("record order/frame")
        records.append(record)
    for lifetime in range(1, LIFETIMES):
        start = lifetime * RECORDS_PER_LIFETIME
        if records[start:start + RECORDS_PER_LIFETIME] != records[:RECORDS_PER_LIFETIME]:
            raise ValueError("same-process lifetime divergence")
    return b"".join(records)


def compare(outputs):
    canonical = [validate(output) for output in outputs]
    if any(value != canonical[0] for value in canonical[1:]):
        raise ValueError("cross-configuration byte divergence")
    return hashlib.sha256(canonical[0]).hexdigest()


def must_reject(lines):
    try:
        validate(lines)
    except ValueError:
        return
    raise ValueError("mutated records accepted")


def mutation_checks(lines):
    must_reject(lines[:-2] + lines[-1:])
    reordered = lines.copy()
    reordered[1], reordered[2] = reordered[2], reordered[1]
    must_reject(reordered)
    tampered = lines.copy()
    payload = bytearray.fromhex(tampered[0][len(PREFIX):].decode("ascii"))
    payload[20] ^= 1
    tampered[0] = PREFIX + payload.hex().encode("ascii")
    must_reject(tampered)
    divergent_seed = lines.copy()
    payload = bytearray.fromhex(divergent_seed[RECORDS_PER_LIFETIME][len(PREFIX):].decode("ascii"))
    payload[16] ^= 1
    divergent_seed[RECORDS_PER_LIFETIME] = PREFIX + payload.hex().encode("ascii")
    must_reject(divergent_seed)


def main(paths):
    if len(paths) != 3:
        raise ValueError("provide GCC, GCC sanitizer and Clang sanitizer fixtures")
    outputs = []
    for ordinal, path in enumerate(paths, 1):
        try:
            result = subprocess.run([path, "h1-records"], capture_output=True, timeout=120)
        except (OSError, subprocess.TimeoutExpired) as error:
            raise ValueError(f"configuration {ordinal} did not complete") from error
        if result.returncode:
            raise ValueError(f"configuration {ordinal} rejected H1")
        outputs.append(result.stdout.splitlines())
    mutation_checks(outputs[0])
    digest = compare(outputs)
    print(f"H1 checkpoints: 121 x 3 records, 224 bytes each, SHA256 {digest}")
    print("Same-process and all three supplied configurations match; four mutations rejected.")


if __name__ == "__main__":
    try:
        main(sys.argv[1:])
    except ValueError as error:
        print(f"H1 checkpoint comparison failed: {error}", file=sys.stderr)
        sys.exit(1)
