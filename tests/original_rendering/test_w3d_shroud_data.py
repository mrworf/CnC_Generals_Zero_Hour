#!/usr/bin/env python3
"""Exercise original map-derived W3DShroud CPU ownership and rejection."""

import argparse
import os
from pathlib import Path
from tempfile import TemporaryDirectory

from test_w3d_status_scene import has_validation_diagnostic

import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-shroud-data-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.make_read_only(source)
        os.environ["ZH_M22_SHROUD_DATA_PROFILE"] = "1"
        os.environ["ZH_M22_SHROUD_DATA_MAP"] = r"Maps\Owned\Owned.map"
        for generation in range(2):
            result = run(args.executable.resolve(), base / f"generation-{generation}",
                         source, "mission")
            output = result.stdout + result.stderr
            marker = "original W3DShroud map data: grid=7x7 reentry=4x4 resources=0"
            if result.returncode or marker not in result.stdout or has_validation_diagnostic(output):
                raise SystemExit(f"original shroud data generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original W3DShroud map data: ownership/reentry/rejection ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
