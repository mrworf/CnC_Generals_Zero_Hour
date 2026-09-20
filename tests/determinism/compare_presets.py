#!/usr/bin/env python3
"""Compare canonical M5 checkpoint reports and diagnose the first divergence."""

from __future__ import annotations

import argparse
from pathlib import Path


def parse(path: Path) -> tuple[str, str, dict[int, int], bytes]:
    raw = path.read_bytes()
    lines = raw.decode("utf-8").splitlines()
    if len(lines) < 3 or lines[0] != "format=zh-determinism-v1":
        raise ValueError(f"{path}: unsupported determinism report")
    if not lines[1].startswith("scenario=") or not lines[2].startswith("config="):
        raise ValueError(f"{path}: missing scenario/config context")
    scenario = lines[1].removeprefix("scenario=")
    config = lines[2].removeprefix("config=")
    checkpoints: dict[int, int] = {}
    for line in lines[3:]:
        fields = dict(field.split("=", 1) for field in line.split())
        tick = int(fields["checkpoint"])
        crc = int(fields["crc"])
        if tick in checkpoints:
            raise ValueError(f"{path}: duplicate checkpoint {tick}")
        checkpoints[tick] = crc
    return scenario, config, checkpoints, raw


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("reports", nargs="+", type=Path)
    args = parser.parse_args()
    if len(args.reports) < 2:
        parser.error("at least two reports are required")

    expected_scenario, expected_config, expected, expected_raw = parse(args.reports[0])
    for path in args.reports[1:]:
        scenario, config, actual, actual_raw = parse(path)
        if scenario != expected_scenario or config != expected_config:
            raise SystemExit(
                f"determinism divergence source={path} scenario={scenario} config={config} "
                f"expected-scenario={expected_scenario} expected-config={expected_config}"
            )
        if actual_raw != expected_raw:
            for tick in sorted(set(expected) | set(actual)):
                if expected.get(tick) != actual.get(tick):
                    raise SystemExit(
                        f"determinism divergence source={path} scenario={scenario} config={config} "
                        f"checkpoint={tick} expected={expected.get(tick)} actual={actual.get(tick)}"
                    )
            raise SystemExit(f"determinism report byte divergence source={path}")
    print(
        f"determinism reports match: scenario={expected_scenario} config={expected_config} "
        f"checkpoints={len(expected)} sources={len(args.reports)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
