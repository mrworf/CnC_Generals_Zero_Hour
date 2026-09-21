#!/usr/bin/env python3
import argparse
import importlib.util
import os
import pathlib
import re
import subprocess
import tempfile


CHECKPOINT = re.compile(
    r"original simulation checkpoint: frames=(\d+)>(\d+) ai=(\d+) scripts=(\d+) "
    r"position=(-?\d+)>(-?\d+) actor=(\d+) target=(\d+) moved=(\d+) attacked=(\d+) "
    r"invalid-rejected=(\d+) terminal=(\d+) target-health=(-?\d+)>(-?\d+)"
)


def load_setup(root: pathlib.Path):
    path = root / "tests/original_simulation/test_scenario_setup.py"
    spec = importlib.util.spec_from_file_location("m21_setup_fixture", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def execute(executable: pathlib.Path, base: pathlib.Path, source: pathlib.Path,
            scenario: str, client_burn: int, audio_burn: int):
    label = f"{scenario}-{client_burn}-{audio_burn}"
    cwd = base / f"cwd-{label}"
    cwd.mkdir(parents=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": scenario,
        "ZH_M21_SIMULATION": "1",
        "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        "ZH_M21_CLIENT_RANDOM_BURN": str(client_burn),
        "ZH_M21_AUDIO_RANDOM_BURN": str(audio_burn),
        "XDG_CONFIG_HOME": str(base / f"xdg-{label}/config"),
        "XDG_CACHE_HOME": str(base / f"xdg-{label}/cache"),
        "XDG_DATA_HOME": str(base / f"xdg-{label}/data"),
        "XDG_STATE_HOME": str(base / f"xdg-{label}/state"),
    })
    result = subprocess.run([str(executable)], cwd=cwd, env=env, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)
    match = CHECKPOINT.search(result.stdout)
    if result.returncode or not match:
        raise SystemExit(f"{label} failed ({result.returncode}):\n{result.stdout}{result.stderr}")
    values = tuple(int(value) for value in match.groups())
    if not all(values[index] for index in (8, 9, 10, 11)):
        raise SystemExit(f"{label} omitted a source-owned transition: {values}")
    if values[1] <= values[0] or values[2] == 0 or values[3] == 0:
        raise SystemExit(f"{label} did not advance original simulation systems: {values}")
    return values


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    setup = load_setup(args.source_root.resolve())
    fixture = setup.load_m20_fixture(args.source_root.resolve())
    with tempfile.TemporaryDirectory(prefix="zh-m21-simulation-") as directory:
        base = pathlib.Path(directory)
        source = base / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        before = sorted((path.relative_to(source), path.read_bytes())
                        for path in source.rglob("*") if path.is_file())
        fixture.make_read_only(source)
        presets = ((0, 0), (19, 0), (0, 23), (31, 37))
        for scenario in ("mission", "skirmish"):
            checkpoints = [execute(args.executable.resolve(), base, source, scenario, *preset)
                           for preset in presets]
            if any(checkpoint != checkpoints[0] for checkpoint in checkpoints[1:]):
                raise SystemExit(f"{scenario} changed with client/audio randomness: {checkpoints}")
        after = sorted((path.relative_to(source), path.read_bytes())
                       for path in source.rglob("*") if path.is_file())
        if before != after:
            raise SystemExit("read-only simulation input changed")
    print("M21 original simulation behavior: mission/skirmish commands, AI, scripts, defeat/victory, randomness ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
