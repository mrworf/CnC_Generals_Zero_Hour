#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import tempfile


def main() -> int:
    compiler = sys.argv[1]
    with tempfile.TemporaryDirectory(prefix="m26-provider-removal-") as directory:
        root = pathlib.Path(directory)
        source = root / "consumer.cpp"
        binary = root / "consumer"
        source.write_text("extern void initMemoryManager(); int main(){initMemoryManager();}\n", encoding="utf-8")
        result = subprocess.run([compiler, str(source), "-o", str(binary)], check=False,
                                text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if result.returncode == 0:
            print("consumer linked after required original provider removal", file=sys.stderr)
            return 1
        if "initMemoryManager" not in result.stderr:
            print("negative link failed for an unrelated reason", file=sys.stderr)
            return 1
    print("original provider-removal link control: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
