#!/usr/bin/env python3
"""Render one generated rigid object through the production-order original factory."""

import argparse
from pathlib import Path
import re
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import fixture, make_read_only, run

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_status_scene import has_validation_diagnostic

MARKER = re.compile(r"mode=original display=1 view=1 terrain=1 empty=(\d+) changed=(\d+) "
                    r"aliases=0 .* residual=(\d+) baseline=(\d+)")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--asset-producer", type=Path, required=True)
    args = parser.parse_args()
    with TemporaryDirectory(prefix="zh-m22-factory-rigid-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        fixture(source)
        make_read_only(source)
        packet = base / "rigid.w3d"
        subprocess.run([str(args.asset_producer.resolve()), "--emit-rigid", str(packet)], check=True)

        def invoke(asset: Path | None):
            overrides = {"ZH_M22_ORIGINAL_FACTORY_PROFILE": "1"}
            if asset is not None:
                overrides["ZH_M22_FACTORY_RIGID_ASSET"] = str(asset)
            result = run(args.executable.resolve(), base, source, env_overrides=overrides)
            output = result.stdout + result.stderr
            if has_validation_diagnostic(output):
                raise SystemExit(f"Vulkan validation rejected factory rigid scene:\n{output}")
            return result, output

        empty, output = invoke(None)
        empty_marker = MARKER.search(empty.stdout)
        if empty.returncode or not empty_marker or tuple(map(int, empty_marker.groups()[:2])) != (1, 0):
            raise SystemExit(f"factory rigid absence control failed ({empty.returncode}):\n{output}")
        control_residual = tuple(map(int, empty_marker.groups()[2:]))

        for generation in range(2):
            present, output = invoke(packet)
            present_marker = MARKER.search(present.stdout)
            if present.returncode or not present_marker:
                raise SystemExit(f"factory rigid generation {generation} failed "
                                 f"({present.returncode}):\n{output}")
            _center_black, changed, residual, baseline = map(int, present_marker.groups())
            if changed == 0 or (residual, baseline) != control_residual:
                raise SystemExit("factory rigid pixel/teardown contract changed: "
                                 f"{present_marker.group(0)}\n{output}")

        missing, output = invoke(base / "missing.w3d")
        if missing.returncode != 3 or "factory rigid packet unreadable" not in output or \
                f"residual={control_residual[0]} baseline={control_residual[1]} owners=0" not in output:
            raise SystemExit(f"factory rigid missing-packet rollback failed ({missing.returncode}):\n{output}")

        malformed = base / "malformed.w3d"
        malformed.write_bytes(b"not-a-w3d")
        rejected, output = invoke(malformed)
        if rejected.returncode != 3 or "factory rigid packet rejected" not in output or \
                f"residual={control_residual[0]} baseline={control_residual[1]} owners=0" not in output:
            raise SystemExit(f"factory rigid malformed rollback failed ({rejected.returncode}):\n{output}")

    print("original factory rigid source pixels: present/absent/rollback ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
