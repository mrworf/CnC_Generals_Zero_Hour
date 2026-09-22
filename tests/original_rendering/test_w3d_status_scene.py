#!/usr/bin/env python3
"""Run a bounded original GameClient scenario through the 2D source draw probe."""

import argparse
import os
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


def has_validation_diagnostic(output: str) -> bool:
    return "Validation Error" in output or "VUID-" in output


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--physical", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-status-scene-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.make_read_only(source)
        os.environ["ZH_M22_STATUS_2D_PROFILE"] = "1"
        if args.physical:
            os.environ["ZH_M22_STATUS_PHYSICAL"] = "1"
        result = run(args.executable.resolve(), base, source, "mission")
        marker = ("original 2D status scene: physical generations=4 resources=0"
                  if args.physical else "original 2D status scene: source draw=1 resources=0")
        combined_output = result.stdout + result.stderr
        if result.returncode or marker not in result.stdout or has_validation_diagnostic(combined_output):
            raise SystemExit(f"original 2D status scene failed ({result.returncode}):\n"
                             f"{combined_output}")
    print("original GameClient 2D status source frame: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
