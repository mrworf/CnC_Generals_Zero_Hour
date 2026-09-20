#!/usr/bin/env python3
import os
import pathlib
import subprocess
import sys
import tempfile


def run(executable: str, expected: str, contents: bytes | None, arbitrary_cwd: pathlib.Path) -> None:
    env = os.environ.copy()
    if contents is None and expected == "defaults":
        env.pop("ZH_MEMORY_POOLS_INI", None)
    else:
        fixture = arbitrary_cwd / f"{expected}.ini"
        if contents is not None:
            fixture.write_bytes(contents)
        env["ZH_MEMORY_POOLS_INI"] = str(fixture)
    result = subprocess.run(
        [executable, expected], cwd=arbitrary_cwd, env=env,
        text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False,
    )
    if result.returncode:
        raise AssertionError(f"{expected}: {result.stdout}\n{result.stderr}")


def main() -> int:
    executable = sys.argv[1]
    with tempfile.TemporaryDirectory(prefix="m26-pools-") as directory:
        cwd = pathlib.Path(directory)
        run(executable, "defaults", None, cwd)
        run(executable, "applied", b"UnknownPool 1 1\nPartitionContactListNode 5 3\n", cwd)
        run(executable, "malformed", b"PartitionContactListNode nope 4\n", cwd)
        run(executable, "invalid_count", b"PartitionContactListNode -1 4\n", cwd)
        run(executable, "duplicate", b"PartitionContactListNode 4 4\nPartitionContactListNode 8 8\n", cwd)
        run(executable, "oversized", b"x" * 5000, cwd)
        run(executable, "io_error", None, cwd)
    print("original-process pool configuration matrix: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
