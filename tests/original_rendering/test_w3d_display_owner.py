#!/usr/bin/env python3
"""Exercise original W3DDisplay owner bootstrap within an initialized source scenario."""

import argparse
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run
from test_w3d_status_scene import has_validation_diagnostic


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--physical", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-display-owner-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.make_read_only(source)
        packet = base / "rigid.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-rigid", str(packet)],
                       check=True)
        os.environ["ZH_M22_DISPLAY_OWNER_PROFILE"] = "1"
        os.environ["ZH_M22_DISPLAY_OWNER_ASSET"] = str(packet)
        if args.physical:
            os.environ["ZH_M22_DISPLAY_OWNER_PHYSICAL"] = "1"
        result = run(args.executable.resolve(), base, source, "mission")
        marker = ("original display owner: bgfx generations=2 resources=0" if args.physical
                  else "original display owner: source generations=2 resources=0")
        output = result.stdout + result.stderr
        if result.returncode or marker not in result.stdout or has_validation_diagnostic(output):
            raise SystemExit(f"original display owner failed ({result.returncode}):\n{output}")
    print("original W3DDisplay source owner lifecycle: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
