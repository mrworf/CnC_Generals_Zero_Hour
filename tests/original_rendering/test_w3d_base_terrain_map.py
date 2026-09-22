#!/usr/bin/env python3
"""Exercise original BaseHeightMap logical map/shroud ownership."""

import argparse
import os
from pathlib import Path
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_simulation"))
from test_scenario_setup import load_m20_fixture, prepare_owned_source, run


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    with TemporaryDirectory(prefix="zh-m22-base-terrain-map-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        fixture.make_read_only(source)
        os.environ["ZH_M22_BASE_TERRAIN_PROFILE"] = "1"
        os.environ["ZH_M22_BASE_TERRAIN_MAP"] = r"Maps\Owned\Owned.map"
        marker = "original base terrain map owner: map=8x8 shroud=7x7 resources=0"
        for generation in range(2):
            result = run(args.executable.resolve(), base / f"generation-{generation}",
                         source, "mission")
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original base terrain generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original base terrain map owner: ownership/reentry/rejection ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
