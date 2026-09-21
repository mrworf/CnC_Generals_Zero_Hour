"""Gated, read-only retail original save/load and exact partition CRC check."""

import argparse
import os
import pathlib
import subprocess
import tempfile


SCENARIOS = (("mission", r"Maps\MD_USA01\MD_USA01.map"),)


def metadata(root: pathlib.Path):
    return tuple(sorted((entry.relative_to(root), entry.stat().st_mode,
                         entry.stat().st_size, entry.stat().st_mtime_ns)
                        for entry in root.rglob("*") if entry.is_file()))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--zh-data-root", type=pathlib.Path, required=True)
    parser.add_argument("--generals-data-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    roots = (args.zh_data_root.resolve(), args.generals_data_root.resolve())
    if not all(root.is_dir() for root in roots):
        raise SystemExit("retail roots are not readable")
    before = tuple(metadata(root) for root in roots)
    with tempfile.TemporaryDirectory(prefix="zh-m24-retail-partition-") as directory:
        state = pathlib.Path(directory)
        for mode, map_name in SCENARIOS:
            scoped = state / mode
            cwd = scoped / "cwd"
            cwd.mkdir(parents=True)
            env = os.environ.copy()
            env.update({
                "ZH_DATA_ROOT": str(roots[0]),
                "ZH_GENERALS_DATA_ROOT": str(roots[1]),
                "ZH_M21_SCENARIO": mode,
                "ZH_M21_MAP": map_name,
                "ZH_M21_ALLOW_EMPTY_PROPS": "1",
                "ZH_M21_SIMULATION": "1",
                "ZH_M24_SAVE_AT_START": "1",
                "ZH_M24_SAVE_FILENAME": "roundtrip.sav",
                "ZH_M24_LOAD_AFTER_SAVE": "1",
                "ZH_M24_REPEAT_ROUNDTRIP": "1",
                "XDG_CONFIG_HOME": str(scoped / "xdg/config"),
                "XDG_CACHE_HOME": str(scoped / "xdg/cache"),
                "XDG_DATA_HOME": str(scoped / "xdg/data"),
                "XDG_STATE_HOME": str(scoped / "xdg/state"),
            })
            result = subprocess.run([str(args.executable.resolve())], cwd=cwd,
                                    env=env, text=True, capture_output=True, timeout=180)
            if (result.returncode or "original persistence load:" not in result.stdout or
                    "original persistence repeated:" not in result.stdout):
                detail = next((line for line in (result.stdout + result.stderr).splitlines()
                               if "original scenario load changed checkpoint:" in line or
                               "original scenario save failed" in line or
                               "repeated original scenario" in line or
                               "original load rejected" in line), "")
                stages = [line.split(":", 1)[0] for line in result.stdout.splitlines()
                          if line.startswith("original persistence load:") or
                          line.startswith("original persistence repeated:")]
                raise SystemExit(f"retail {mode} exact source load failed "
                                 f"(exit {result.returncode}, stages {stages}): "
                                 f"{detail or 'private output redacted'}")
            saves = list((scoped / "xdg/data").rglob("roundtrip.sav"))
            repeated = list((scoped / "xdg/data").rglob("repeated.sav"))
            rollback = list((scoped / "xdg/data").rglob(".rollback.*"))
            if (len(saves) != 1 or len(repeated) != 1 or rollback or
                    saves[0].stat().st_size <= 128 or repeated[0].stat().st_size <= 128):
                raise SystemExit(f"retail {mode} original repeated save missing or temporary rollback left")
    if before != tuple(metadata(root) for root in roots):
        raise SystemExit("retail corpus metadata changed")
    print("M24 retail original mission exact repeated load: pass; corpus unchanged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
