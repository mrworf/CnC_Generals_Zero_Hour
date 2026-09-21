"""Source-owned visible-shroud save/load and corrupt partition rollback."""

import argparse
import contextlib
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


def run(executable: pathlib.Path, base: pathlib.Path, source: pathlib.Path,
        name: str, existing: bool = False):
    cwd = base / "cwd"
    cwd.mkdir(exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": "mission",
        "ZH_M21_MAP": r"Maps\Owned\Owned.map",
        "ZH_M21_SIMULATION": "1",
        "ZH_M24_SAVE_FILENAME": name,
        "ZH_M24_LOAD_AFTER_SAVE": "1",
        "XDG_CONFIG_HOME": str(base / "xdg/config"),
        "XDG_CACHE_HOME": str(base / "xdg/cache"),
        "XDG_DATA_HOME": str(base / "xdg/data"),
        "XDG_STATE_HOME": str(base / "xdg/state"),
    })
    if existing:
        env["ZH_M24_LOAD_EXISTING"] = "1"
    else:
        env["ZH_M24_REPEAT_ROUNDTRIP"] = "1"
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=90)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()
    executable = args.executable.resolve()
    root = args.source_root.resolve()
    setup = load_setup(root)
    fixture = setup.load_m20_fixture(root)
    context = (contextlib.nullcontext(tempfile.mkdtemp(prefix="zh-m24-shroud-"))
               if args.keep else tempfile.TemporaryDirectory(prefix="zh-m24-shroud-"))
    with context as directory:
        base = pathlib.Path(directory)
        if args.keep:
            print(f"M24 retained owned test: {base}", flush=True)
        source = base / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        object_ini = source / "Data/INI/Default/Object.ini"
        original = object_ini.read_text()
        for name in ("LogicFixture", "EnemyFixture"):
            needle = f"Object {name}\n KindOf"
            if needle not in original:
                raise SystemExit("source-owned object fixture changed")
            original = original.replace(needle, f"Object {name}\n VisionRange = 100\n"
                                        " ShroudClearingRange = 100\n KindOf", 1)
        fixture.write(object_ini, original)
        fixture.make_read_only(source)
        result = run(executable, base, source, "visible.sav")
        if (result.returncode or "original persistence load:" not in result.stdout or
                "original persistence repeated:" not in result.stdout):
            raise SystemExit(f"visible-shroud exact source load failed ({result.returncode}):\n"
                             f"{result.stdout}{result.stderr}")
        saves = list((base / "xdg/data").rglob("visible.sav"))
        if len(saves) != 1:
            raise SystemExit("visible-shroud original save missing")
        candidate = saves[0].read_bytes()
        marker = b"CHUNK_Partition"
        at = candidate.index(marker) + len(marker) + 4
        invalid = bytearray(candidate)
        invalid[at] = 255
        negative = base / "negative"
        corrupt = negative / "xdg/data/generals-zero-hour/Save/corrupt-partition.sav"
        corrupt.parent.mkdir(parents=True)
        corrupt.write_bytes(invalid)
        rejected = run(executable, negative, source, corrupt.name, existing=True)
        if rejected.returncode or "rollback=1" not in rejected.stdout:
            raise SystemExit(f"corrupt source partition did not roll back ({rejected.returncode}):\n"
                             f"{rejected.stdout}{rejected.stderr}")
        if corrupt.read_bytes() != invalid or saves[0].read_bytes() != candidate:
            raise SystemExit("corrupt partition load changed candidate or prior save")
        if list((negative / "xdg/data").rglob(".rollback.*")):
            raise SystemExit("corrupt partition rollback left a private checkpoint")
        print("M24 source-owned visible shroud: exact save/load and corrupt rollback ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
