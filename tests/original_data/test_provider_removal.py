#!/usr/bin/env python3
import pathlib
import subprocess
import sys
import tempfile


def main() -> int:
    compiler = sys.argv[1]
    repository = pathlib.Path(sys.argv[2]).resolve()
    with tempfile.TemporaryDirectory(prefix="m27-provider-removal-") as directory:
        root = pathlib.Path(directory)
        source = root / "consumer.cpp"
        binary = root / "consumer"
        source.write_text(
            '#include "zh/original_data.h"\n'
            "int main(){ return zh::original_data::provider_xfer_identity()[0] + "
            "zh::original_data::provider_random_identity()[0] + "
            "zh::original_data::provider_map_identity()[0]; }\n",
            encoding="utf-8",
        )
        result = subprocess.run(
            [compiler, "-std=c++17", f"-I{repository / 'include'}", str(source), "-o", str(binary)],
            check=False, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if result.returncode == 0:
            print("consumer linked after required M27 providers were removed", file=sys.stderr)
            return 1
        if not all(name in result.stderr for name in
                   ("provider_xfer_identity", "provider_random_identity", "provider_map_identity")):
            print("negative M27 link failed for an unrelated reason", file=sys.stderr)
            return 1
    print("M27 provider-removal link control: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
