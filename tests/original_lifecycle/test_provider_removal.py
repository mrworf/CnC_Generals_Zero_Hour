#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import tempfile


def main() -> int:
    compiler = sys.argv[1]
    include_runtime = pathlib.Path(sys.argv[2])
    include_engine = pathlib.Path(sys.argv[3])
    include_libraries = pathlib.Path(sys.argv[4])
    with tempfile.TemporaryDirectory(prefix="m20-lifecycle-provider-") as directory:
        root = pathlib.Path(directory)
        source = root / "consumer.cpp"
        binary = root / "consumer"
        source.write_text(
            '#include "PreRTS.h"\n#include "Common/SubsystemInterface.h"\n'
            'int main(){SubsystemInterfaceList list; installSubsystemINIDataLoader(nullptr);}\n',
            encoding="utf-8",
        )
        result = subprocess.run([
            compiler, "-std=c++17", f"-I{include_runtime}", f"-I{include_engine}",
            f"-I{include_libraries}", str(source), "-o", str(binary),
        ], check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode == 0:
            print("consumer linked after original lifecycle provider removal", file=sys.stderr)
            return 1
        if "SubsystemInterfaceList" not in result.stderr and "installSubsystemINIDataLoader" not in result.stderr:
            print("negative link failed for an unrelated reason", file=sys.stderr)
            return 1
    print("M20 lifecycle provider-removal link control: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
