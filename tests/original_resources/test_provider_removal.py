#!/usr/bin/env python3
import argparse
import pathlib
import subprocess
import sys
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=pathlib.Path)
    parser.add_argument("--build-dir", required=True, type=pathlib.Path)
    args = parser.parse_args()
    archive = args.build_dir / "libzh_original_resources.a"
    if not archive.is_file():
        print("M28 provider archive is absent", file=sys.stderr)
        return 1
    with tempfile.TemporaryDirectory(prefix="zh-m28-provider-removal-") as temporary:
        source = pathlib.Path(temporary) / "consumer.cpp"
        binary = pathlib.Path(temporary) / "consumer"
        source.write_text(
            '#include "zh/original_resources.h"\n'
            "int main(){ return zh::original_resources::provider_cpu_identity()[0] + "
            "zh::original_resources::provider_ui_identity()[0] + "
            "zh::original_resources::provider_audio_identity()[0]; }\n",
            encoding="utf-8")
        command = ["c++", "-std=c++17", "-I", str(args.root / "include"), str(source), "-o", str(binary)]
        result = subprocess.run(command, text=True, capture_output=True)
        if result.returncode == 0:
            print("consumer linked after all required M28 providers were removed", file=sys.stderr)
            return 1
        diagnostic = result.stdout + result.stderr
        for symbol in ("provider_cpu_identity", "provider_ui_identity", "provider_audio_identity"):
            if symbol not in diagnostic:
                print(f"negative M28 link failed without {symbol}", file=sys.stderr)
                return 1
    print("M28 provider-removal link control: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
