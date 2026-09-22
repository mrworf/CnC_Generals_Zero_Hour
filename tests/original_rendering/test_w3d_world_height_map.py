#!/usr/bin/env python3
"""Exercise canonical logical WorldHeightMap ownership in the full-draw target."""

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
    with TemporaryDirectory(prefix="zh-m22-world-height-map-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        map_path = source / "Maps/Owned/Owned.map"
        fixture.make_read_only(source)
        os.environ["ZH_M22_HEIGHT_MAP_PROFILE"] = "1"
        os.environ["ZH_M22_HEIGHT_MAP_INPUT"] = str(map_path)
        for generation in range(2):
            result = run(args.executable.resolve(), base / f"generation-{generation}",
                         source, "mission")
            marker = "original WorldHeightMap logical owner: dimensions=8x8 generations=2 resources=0"
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"original WorldHeightMap generation {generation} failed "
                                 f"({result.returncode}); private output redacted")
    print("original WorldHeightMap logical owner: parse/reject/reentry ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
