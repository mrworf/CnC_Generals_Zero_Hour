#!/usr/bin/env python3
"""Read-only original-data audit with bounded public output and byte integrity."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess


def snapshot(roots):
    records = {}
    for ordinal, supplied in enumerate(roots):
        link = os.lstat(supplied)
        records[(ordinal, "root")] = (link.st_dev, link.st_ino, os.path.realpath(supplied))
        for directory, subdirectories, files in os.walk(supplied, followlinks=False):
            subdirectories[:] = sorted(name for name in subdirectories
                                       if not os.path.islink(os.path.join(directory, name)))
            for name in sorted(files):
                path = Path(directory) / name
                if path.is_symlink() or not path.is_file():
                    continue
                before = path.stat()
                digest = hashlib.sha256()
                with path.open("rb") as stream:
                    for chunk in iter(lambda: stream.read(4 * 1024 * 1024), b""):
                        digest.update(chunk)
                after = path.stat()
                if (before.st_size, before.st_mtime_ns, before.st_ino) != (
                        after.st_size, after.st_mtime_ns, after.st_ino):
                    raise RuntimeError("input changed during snapshot")
                records[(ordinal, str(path.relative_to(supplied)))] = (
                    after.st_size, after.st_mtime_ns, digest.digest())
    return records


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--data-root", action="append", required=True,
                        help="ordered supplied roots, Zero Hour before Generals")
    options = parser.parse_args()
    try:
        before = snapshot(options.data_root)
        result = None
        try:
            result = subprocess.run([options.binary, *options.data_root],
                                    capture_output=True, timeout=120, check=False)
        except Exception:
            pass
        unchanged = before == snapshot(options.data_root)
        if result is None:
            print("READ_ONLY_DATA_AUDIT status=EXECUTION_REJECTED")
            print("INPUT_INTEGRITY=" + ("UNCHANGED" if unchanged else "CHANGED"))
            return 1
        # Never relay exception messages, stderr, file names, hashes or paths.
        public = re.fullmatch(rb"READ_ONLY_DATA_AUDIT stage=[0-4] mask=[0-9]{1,2} "
                              rb"status=(?:PASS|INCOMPLETE|REJECTED)\n", result.stdout)
        if not public or result.stderr:
            print("READ_ONLY_DATA_AUDIT status=OUTPUT_REJECTED")
        else:
            print(result.stdout.decode("ascii").strip())
        print("INPUT_INTEGRITY=" + ("UNCHANGED" if unchanged else "CHANGED"))
        return 0 if unchanged and public and not result.stderr and result.returncode == 0 else 1
    except Exception:
        print("READ_ONLY_DATA_AUDIT status=WRAPPER_REJECTED")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
