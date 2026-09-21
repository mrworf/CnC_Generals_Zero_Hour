"""Original shipped special powers survive reset; map.ini overrides do not."""

import argparse
import importlib.util
import os
import pathlib
import subprocess
import tempfile


def load_setup(root: pathlib.Path):
    path = root / "tests/original_simulation/test_scenario_setup.py"
    spec = importlib.util.spec_from_file_location("m21_setup", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def run(executable: pathlib.Path, state: pathlib.Path, source: pathlib.Path,
        mode: str, map_name: str, gate: str, generals: pathlib.Path | None = None):
    cwd = state / f"cwd-{mode}"
    cwd.mkdir()
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": mode,
        "ZH_M21_MAP": map_name,
        "ZH_M24_POWER_BASELINE": gate,
        "XDG_CONFIG_HOME": str(state / f"xdg-{mode}/config"),
        "XDG_CACHE_HOME": str(state / f"xdg-{mode}/cache"),
        "XDG_DATA_HOME": str(state / f"xdg-{mode}/data"),
        "XDG_STATE_HOME": str(state / f"xdg-{mode}/state"),
    })
    if generals:
        env["ZH_GENERALS_DATA_ROOT"] = str(generals)
        env["ZH_M21_ALLOW_EMPTY_PROPS"] = "1"
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=120)


def metadata(root: pathlib.Path):
    return tuple(sorted((entry.relative_to(root), entry.stat().st_mode,
                         entry.stat().st_size, entry.stat().st_mtime_ns)
                        for entry in root.rglob("*") if entry.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--zh-data-root", type=pathlib.Path)
    parser.add_argument("--generals-data-root", type=pathlib.Path)
    args = parser.parse_args()
    executable = args.executable.resolve()
    with tempfile.TemporaryDirectory(prefix="zh-m24-power-") as directory:
        state = pathlib.Path(directory)
        if args.zh_data_root or args.generals_data_root:
            if not args.zh_data_root or not args.generals_data_root:
                raise SystemExit("both retail roots are required")
            roots = (args.zh_data_root.resolve(), args.generals_data_root.resolve())
            if not all(root.is_dir() for root in roots):
                raise SystemExit("retail roots are not readable")
            before = tuple(metadata(root) for root in roots)
            for mode, map_name in (("mission", r"Maps\MD_USA01\MD_USA01.map"),
                                   ("skirmish", r"Maps\BarrenBadlands\BarrenBadlands.map")):
                result = run(executable, state, roots[0], mode, map_name, "retail", roots[1])
                if result.returncode or "original power baseline:" not in result.stdout:
                    raise SystemExit(f"retail {mode} special-power reset failed; private output redacted")
            if before != tuple(metadata(root) for root in roots):
                raise SystemExit("retail corpus metadata changed")
            print("M24 retail shipped special powers: mission/skirmish reset ok; input unchanged")
            return 0

        root = args.source_root.resolve()
        setup = load_setup(root)
        fixture = setup.load_m20_fixture(root)
        source = state / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        fixture.write(source / "Data/INI/SpecialPower.ini",
                      "SpecialPower ShippedFixturePower\n  PublicTimer = Yes\nEND\n")
        fixture.write(source / "Maps/Owned/map.ini",
                      "SpecialPower MapFixturePower\nEND\n"
                      "SpecialPower ShippedFixturePower\n  PublicTimer = No\nEND\n")
        fixture.make_read_only(source)
        result = run(executable, state, source, "mission", r"Maps\Owned\Owned.map", "owned")
        if result.returncode or "original power baseline:" not in result.stdout:
            raise SystemExit(f"source special-power reset failed ({result.returncode}):\n"
                             f"{result.stdout}{result.stderr}")
        missing = state / "missing-shipped"
        missing.mkdir()
        missing_source = missing / "readonly-input"
        setup.prepare_owned_source(missing_source, fixture)
        fixture.write(missing_source / "Maps/Owned/map.ini",
                      "SpecialPower MapFixturePower\nEND\n")
        fixture.make_read_only(missing_source)
        rejected = run(executable, missing, missing_source, "mission",
                       r"Maps\Owned\Owned.map", "owned")
        if rejected.returncode == 0 or "original power baseline:" in rejected.stdout:
            raise SystemExit("missing shipped special-power definitions were accepted")
        print("M24 owned shipped and map-only special-power lifetime: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
