#!/usr/bin/env python3
import argparse
import importlib.util
import os
import pathlib
import re
import subprocess
import tempfile


REENTRY = re.compile(
    r"original re-entry checkpoint: reset-objects=(\d+) reset-map-objects=(\d+) "
    r"reset-props=(\d+) reset-model-preloads=(\d+) reset-texture-preloads=(\d+) "
    r"reset-recorder-controls=(\d+) positions=(-?\d+)>(-?\d+) second-objects=(\d+) "
    r"second-props=(\d+) second-recorder-controls=(\d+) devices=0"
)


def load_setup(root: pathlib.Path):
    path = root / "tests/original_simulation/test_scenario_setup.py"
    spec = importlib.util.spec_from_file_location("m21_setup_fixture", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def execute(executable: pathlib.Path, base: pathlib.Path, source: pathlib.Path,
            client_burn: int, audio_burn: int):
    label = f"{client_burn}-{audio_burn}"
    cwd = base / f"cwd-{label}"
    cwd.mkdir(parents=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": "mission",
        "ZH_M21_SIMULATION": "1",
        "ZH_M21_REENTRY": "1",
        "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        "ZH_M21_REENTRY_MAP": r"Maps\OwnedReentry\OwnedReentry.map",
        "ZH_M21_CLIENT_RANDOM_BURN": str(client_burn),
        "ZH_M21_AUDIO_RANDOM_BURN": str(audio_burn),
        "XDG_CONFIG_HOME": str(base / f"xdg-{label}/config"),
        "XDG_CACHE_HOME": str(base / f"xdg-{label}/cache"),
        "XDG_DATA_HOME": str(base / f"xdg-{label}/data"),
        "XDG_STATE_HOME": str(base / f"xdg-{label}/state"),
    })
    result = subprocess.run([str(executable)], cwd=cwd, env=env, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=45)
    match = REENTRY.search(result.stdout)
    if result.returncode or not match:
        raise SystemExit(f"re-entry {label} failed ({result.returncode}):\n{result.stdout}{result.stderr}")
    values = tuple(int(value) for value in match.groups())
    if any(values[:6]):
        raise SystemExit(f"re-entry {label} retained pre-reset state: {values}")
    if values[6] == values[7] or values[8:] != (3, 1, 1):
        raise SystemExit(f"re-entry {label} did not publish a fresh second scenario: {values}")
    if "original scenario setup: mode=2 " not in result.stdout:
        raise SystemExit(f"re-entry {label} did not finish in original skirmish mode")
    if "original simulation checkpoint:" not in result.stdout:
        raise SystemExit(f"re-entry {label} omitted second simulation checkpoint")
    return values


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    root = args.source_root.resolve()
    setup = load_setup(root)
    fixture = setup.load_m20_fixture(root)
    with tempfile.TemporaryDirectory(prefix="zh-m21-reentry-") as directory:
        base = pathlib.Path(directory)
        source = base / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        before = sorted((path.relative_to(source), path.read_bytes())
                        for path in source.rglob("*") if path.is_file())
        fixture.make_read_only(source)
        baseline = execute(args.executable.resolve(), base, source, 0, 0)
        varied = execute(args.executable.resolve(), base, source, 31, 37)
        if baseline != varied:
            raise SystemExit(f"re-entry checkpoint changed with client/audio randomness: {baseline} != {varied}")
        after = sorted((path.relative_to(source), path.read_bytes())
                       for path in source.rglob("*") if path.is_file())
        if before != after:
            raise SystemExit("read-only re-entry input changed")
    print("M21 original lifecycle: mission-reset-skirmish re-entry fresh and deterministic")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
