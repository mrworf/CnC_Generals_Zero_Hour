#!/usr/bin/env python3
"""Check opt-in original factory startup against an equivalent device-only lifecycle."""

import argparse
from pathlib import Path
import re
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import SUCCESS, fixture, make_read_only, run

sys.path.insert(0, str(Path(__file__).resolve().parent))
from test_w3d_status_scene import has_validation_diagnostic


MARKER = re.compile(
    r"original graphics bootstrap: mode=(original|device-only) "
    r"display=(\d+) view=(\d+) terrain=(\d+) empty=(\d+) changed=(\d+) aliases=(\d+) "
    r"ready=(\d+) engine=(\d+) residual=(\d+) baseline=(\d+)"
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    args = parser.parse_args()
    executable = args.executable.resolve()
    with TemporaryDirectory(prefix="zh-m22-factory-bootstrap-") as scratch:
        base = Path(scratch)
        source = base / "readonly-input"
        fixture(source)
        make_read_only(source)

        def invoke(profile: str | None, failure: str | None = None):
            overrides = {}
            if profile == "original":
                overrides["ZH_M22_ORIGINAL_FACTORY_PROFILE"] = "1"
            elif profile == "device-only":
                overrides["ZH_M22_GPU_DEVICE_ONLY_PROFILE"] = "1"
            if failure:
                overrides[failure] = "1"
            result = run(executable, base, source, env_overrides=overrides)
            output = result.stdout + result.stderr
            if has_validation_diagnostic(output):
                raise SystemExit(f"Vulkan validation rejected {profile}:\n{output}")
            return result, output

        default, output = invoke(None)
        if default.returncode or SUCCESS not in default.stdout or MARKER.search(output):
            raise SystemExit(f"default headless factory changed ({default.returncode}):\n{output}")

        device, output = invoke("device-only")
        device_marker = MARKER.search(device.stdout)
        if device.returncode or not device_marker or device_marker.group(1) != "device-only" or \
                tuple(map(int, device_marker.groups()[1:7])) != (0, 0, 0, 0, 0, 0):
            raise SystemExit(f"device-only control failed ({device.returncode}):\n{output}")
        device_counts = tuple(map(int, device_marker.groups()[7:]))

        for generation in range(2):
            original, output = invoke("original")
            original_marker = MARKER.search(original.stdout)
            if original.returncode or not original_marker or original_marker.group(1) != "original" or \
                    tuple(map(int, original_marker.groups()[1:7])) != (1, 1, 1, 1, 0, 0):
                raise SystemExit(f"original factory generation {generation} failed "
                                 f"({original.returncode}):\n{output}")
            original_counts = tuple(map(int, original_marker.groups()[7:]))
            if original_counts[2:] != device_counts[2:] or \
                    original_counts[1] - original_counts[0] != \
                    device_counts[1] - device_counts[0] or \
                    original_counts[2] - original_counts[3] != 4:
                raise SystemExit("original factory retained source allocations or device residual "
                                 f"changed: original={original_counts} device={device_counts}")

        device_failure, output = invoke("original", "ZH_M22_FACTORY_FAIL_DEVICE")
        if device_failure.returncode != 3 or "forced original graphics device failure" not in output or \
                f"original graphics rollback: residual={device_counts[3]} " \
                f"baseline={device_counts[3]} owners=0" not in output:
            raise SystemExit(f"failed-device rollback changed ({device_failure.returncode}):\n{output}")

        conflicting, output = invoke("original", "ZH_M22_GPU_DEVICE_ONLY_PROFILE")
        if conflicting.returncode != 3 or "requires one normal startup mode" not in output or \
                f"original graphics rollback: residual={device_counts[3]} " \
                f"baseline={device_counts[3]} owners=0" not in output:
            raise SystemExit(f"conflicting-profile rollback changed ({conflicting.returncode}):\n{output}")

        terrain_failure, output = invoke("original", "ZH_M22_FACTORY_FAIL_TERRAIN")
        if terrain_failure.returncode != 3 or "forced original terrain factory failure" not in output or \
                f"original graphics rollback: residual={device_counts[2]} " \
                f"baseline={device_counts[3]} owners=0" not in output:
            raise SystemExit(f"partial-factory rollback changed ({terrain_failure.returncode}):\n{output}")

    print("original graphics factory bootstrap: paired device residual and rollback ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
