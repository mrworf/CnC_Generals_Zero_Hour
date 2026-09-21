"""Source GlobalData default/shipped/map override lifetime and validation."""

import argparse
import importlib.util
import os
import pathlib
import subprocess
import tempfile


def setup_module(root: pathlib.Path):
    path = root / "tests/original_simulation/test_scenario_setup.py"
    spec = importlib.util.spec_from_file_location("m21_setup", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def run(executable: pathlib.Path, state: pathlib.Path, source: pathlib.Path,
        expected: str) -> subprocess.CompletedProcess:
    cwd = state / "cwd"
    cwd.mkdir(parents=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": "mission",
        "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        "ZH_M24_GAMEDATA_BASELINE": expected,
        "XDG_CONFIG_HOME": str(state / "xdg/config"),
        "XDG_CACHE_HOME": str(state / "xdg/cache"),
        "XDG_DATA_HOME": str(state / "xdg/data"),
        "XDG_STATE_HOME": str(state / "xdg/state"),
    })
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=60)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    root = args.source_root.resolve()
    setup = setup_module(root)
    fixture = setup.load_m20_fixture(root)
    with tempfile.TemporaryDirectory(prefix="zh-m24-gamedata-") as directory:
        base = pathlib.Path(directory)
        for name, shipped, map_override, expected in (
                ("default", False, False, "16:16"),
                ("shipped", True, False, "40:40"),
                ("map", True, True, "5:40")):
            state = base / name
            source = state / "readonly-input"
            setup.prepare_owned_source(source, fixture)
            default = source / "Data/INI/Default/GameData.ini"
            contents = default.read_text()
            fixture.write(default, contents.replace(" FramesPerSecondLimit = 1000\n",
                                                    " PartitionCellSize = 16\n"
                                                    " FramesPerSecondLimit = 1000\n", 1))
            if shipped:
                fixture.write(source / "Data/INI/GameData.ini",
                              "GameData\n PartitionCellSize = 40\nEND\n")
            if map_override:
                fixture.write(source / "Maps/Owned/map.ini",
                              "GameData\n PartitionCellSize = 5\nEND\n")
            fixture.make_read_only(source)
            result = run(args.executable.resolve(), state, source, expected)
            if result.returncode or "original GameData baseline:" not in result.stdout:
                raise SystemExit(f"{name} original GameData lifetime failed "
                                 f"({result.returncode}):\n{result.stdout}{result.stderr}")

        state = base / "invalid"
        source = state / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        fixture.write(source / "Data/INI/GameData.ini",
                      "GameData\n PartitionCellSize = invalid\nEND\n")
        fixture.make_read_only(source)
        rejected = run(args.executable.resolve(), state, source, "40:40")
        if rejected.returncode == 0 or "original GameData baseline:" in rejected.stdout:
            raise SystemExit("invalid shipped GameData was accepted")
    print("M24 original GameData: default/shipped/map reset and invalid definition ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
