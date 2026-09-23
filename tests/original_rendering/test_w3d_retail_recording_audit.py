#!/usr/bin/env python3
"""Exercise the private-safe Recording factory seam and retail guard audit."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import sys
from tempfile import TemporaryDirectory

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "original_lifecycle"))
from test_production_entry import fixture, make_read_only, run


RECORDING = re.compile(
    r"original recording factory: commands=(\d+) creates=(\d+) uploads=(\d+) "
    r"passes=(\d+) draws=(\d+) presents=(\d+) failures=(\d+) resources=(\d+)"
)
AUDIT = re.compile(r"original retail terrain configuration: mask=(\d+)")
REACHABILITY = re.compile(r"original retail terrain reachability: mask=(\d+) families=(\d+) owners=(\d+)")
SETUP = "original retail cloud setup: scalar-transaction=1"


def snapshot(root: Path):
    return tuple(sorted((entry.relative_to(root), entry.stat().st_mode,
                         entry.stat().st_size, entry.stat().st_mtime_ns)
                        for entry in root.rglob("*") if entry.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--zh-data-root", type=Path, required=True)
    parser.add_argument("--generals-data-root", type=Path, required=True)
    parser.add_argument("--reachability", action="store_true",
                        help="also require the source-owned mask-30 consumer boundary")
    args = parser.parse_args()
    executable = args.executable.resolve()
    roots = (args.zh_data_root.resolve(), args.generals_data_root.resolve())
    if not all(root.is_dir() for root in roots):
        raise SystemExit("M22 08A retail audit requires readable supplied roots")

    with TemporaryDirectory(prefix="zh-m22-08a-") as scratch:
        base = Path(scratch)
        source = base / "owned-readonly"
        fixture(source)
        make_read_only(source)
        profile = {"ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
                   "ZH_M22_RECORDING_FACTORY_PROFILE": "1"}
        for generation in range(2):
            result = run(str(executable), base / f"generation-{generation}", source,
                         env_overrides=profile)
            marker = RECORDING.search(result.stdout)
            if result.returncode or not marker or int(marker.group(1)) < 2 or \
                    int(marker.group(2)) < 2 or int(marker.group(8)) != 2 or \
                    "original recording factory teardown: resources=0" not in result.stdout:
                raise SystemExit("M22 08A recording factory generation failed")
        invalid = run(str(executable), base / "invalid-selector", source,
                      env_overrides={"ZH_M22_RECORDING_FACTORY_PROFILE": "1"})
        if invalid.returncode != 3 or "recording factory selector requires original factory profile only" not in invalid.stderr:
            raise SystemExit("M22 08A recording selector did not fail closed")
        failure = run(str(executable), base / "device-failure", source,
                      env_overrides={**profile, "ZH_M22_FACTORY_FAIL_DEVICE": "1"})
        if failure.returncode != 3 or "original graphics rollback:" not in failure.stderr or \
                "owners=0" not in failure.stderr:
            raise SystemExit("M22 08A recording factory rollback changed")

        before = tuple(snapshot(root) for root in roots)
        masks = []
        for mode, logical_map in (("mission", r"Maps\MD_USA01\MD_USA01.map"),
                                  ("skirmish", r"Maps\BarrenBadlands\BarrenBadlands.map")):
            state = base / f"retail-{mode}"
            state.mkdir()
            environment = os.environ.copy()
            environment.update({
                "ZH_DATA_ROOT": str(roots[0]),
                "ZH_GENERALS_DATA_ROOT": str(roots[1]),
                "ZH_M20_HEADLESS_PROFILE": "1",
                "ZH_M21_SCENARIO": mode,
                "ZH_M21_MAP": logical_map,
                "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
                "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
                "ZH_M22_RETAIL_CONFIG_AUDIT": "1",
                "XDG_CONFIG_HOME": str(state / "xdg/config"),
                "XDG_CACHE_HOME": str(state / "xdg/cache"),
                "XDG_DATA_HOME": str(state / "xdg/data"),
                "XDG_STATE_HOME": str(state / "xdg/state"),
            })
            result = subprocess.run(
                [str(executable)], cwd=state, env=environment, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                timeout=120)
            match = AUDIT.search(result.stdout)
            if result.returncode != 3 or not match or int(match.group(1)) == 0 or \
                    "original scenario setup:" in result.stdout or \
                    "original graphics rollback:" not in result.stderr or \
                    "owners=0" not in result.stderr or \
                    "original recording factory teardown: resources=0" not in result.stdout:
                raise SystemExit("M22 08A retail configuration audit contract failed; private output redacted")
            masks.append(int(match.group(1)))
        if masks[0] != masks[1] or before != tuple(snapshot(root) for root in roots):
            raise SystemExit("M22 08A retail configuration audit changed input or diverged")

        if args.reachability:
            reached = []
            for mode, logical_map in (("mission", r"Maps\\MD_USA01\\MD_USA01.map"),
                                      ("skirmish", r"Maps\\BarrenBadlands\\BarrenBadlands.map")):
                for generation in range(2):
                    state = base / f"route-{mode}-{generation}"
                    state.mkdir()
                    environment = os.environ.copy()
                    environment.update({
                        "ZH_DATA_ROOT": str(roots[0]),
                        "ZH_GENERALS_DATA_ROOT": str(roots[1]),
                        "ZH_M20_HEADLESS_PROFILE": "1",
                        "ZH_M21_SCENARIO": mode,
                        "ZH_M21_MAP": logical_map,
                        "ZH_M22_ORIGINAL_FACTORY_PROFILE": "1",
                        "ZH_M22_RECORDING_FACTORY_PROFILE": "1",
                        "ZH_M22_RETAIL_CONFIG_AUDIT": "1",
                        "ZH_M22_RETAIL_CONFIG_ROUTE": "1",
                        "XDG_CONFIG_HOME": str(state / "xdg/config"),
                        "XDG_CACHE_HOME": str(state / "xdg/cache"),
                        "XDG_DATA_HOME": str(state / "xdg/data"),
                        "XDG_STATE_HOME": str(state / "xdg/state"),
                    })
                    result = subprocess.run([str(executable)], cwd=state, env=environment, text=True,
                                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)
                    marker = REACHABILITY.search(result.stdout)
                    teardown = "original recording factory teardown: resources=0" in result.stdout
                    rollback = "original graphics rollback:" in result.stderr and "owners=0" in result.stderr
                    if (result.returncode != 3 or not marker or marker.groups() != ("30", "4", "1") or SETUP not in result.stdout or
                            not teardown or not rollback):
                        reason = "other"
                        for candidate in ("retail cloud owner foreign", "terrain-guard", "enabled-shadow", "volume", "water", "display"):
                            if candidate in result.stderr:
                                reason = candidate
                                break
                        raise SystemExit("M22 08C retail scalar transaction contract failed; "
                                         f"status={result.returncode} marker={int(bool(marker))} "
                                         f"teardown={int(teardown)} rollback={int(rollback)} reason={reason}; private output redacted")
                    reached.append(marker.groups())
            if len(set(reached)) != 1 or before != tuple(snapshot(root) for root in roots):
                raise SystemExit("M22 08B retail reachability changed input or diverged")

    if args.reachability:
        print("M22 08B retail Recording reachability: mask=30 families=4 modes=2 generations=2 input=unchanged")
    else:
        print("M22 08A recording factory and retail configuration audit: ok; private input unchanged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
