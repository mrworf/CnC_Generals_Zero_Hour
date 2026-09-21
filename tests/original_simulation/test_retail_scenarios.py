#!/usr/bin/env python3
import argparse
import os
import pathlib
import subprocess
import tempfile


def snapshot(root):
    return tuple(sorted((entry.relative_to(root), entry.stat().st_mode,
                         entry.stat().st_size, entry.stat().st_mtime_ns)
                        for entry in root.rglob("*") if entry.is_file()))


def run(executable, zh_root, generals_root, state, mode, logical_map, allow_empty=False):
    cwd = state / mode
    cwd.mkdir(parents=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(zh_root),
        "ZH_GENERALS_DATA_ROOT": str(generals_root),
        "ZH_M21_SCENARIO": mode,
        "ZH_M21_MAP": logical_map,
        "XDG_CONFIG_HOME": str(state / f"xdg-{mode}/config"),
        "XDG_CACHE_HOME": str(state / f"xdg-{mode}/cache"),
        "XDG_DATA_HOME": str(state / f"xdg-{mode}/data"),
        "XDG_STATE_HOME": str(state / f"xdg-{mode}/state"),
    })
    if allow_empty:
        env["ZH_M21_ALLOW_EMPTY_PROPS"] = "1"
    return subprocess.run([str(executable)], cwd=cwd, env=env, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--zh-data-root", type=pathlib.Path, required=True)
    parser.add_argument("--generals-data-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    roots = (args.zh_data_root.resolve(), args.generals_data_root.resolve())
    if not all(root.is_dir() for root in roots):
        raise SystemExit("M21 retail gate requires both readable data roots")
    before = tuple(snapshot(root) for root in roots)
    with tempfile.TemporaryDirectory(prefix="zh-m21-retail-gate-") as directory:
        state = pathlib.Path(directory)
        cases = (("mission", r"Maps\MD_USA01\MD_USA01.map", False, "mode=0"),
                 ("skirmish", r"Maps\BarrenBadlands\BarrenBadlands.map", True, "mode=2"))
        for mode, logical_map, allow_empty, marker in cases:
            result = run(args.executable.resolve(), *roots, state, mode, logical_map, allow_empty)
            if result.returncode or f"original scenario setup: {marker} " not in result.stdout:
                raise SystemExit(f"M21 retail {mode} failed; private output redacted")
            if "devices=0" not in result.stdout or "recorder-controls=1" not in result.stdout:
                raise SystemExit(f"M21 retail {mode} omitted logical/no-device witness")
    if before != tuple(snapshot(root) for root in roots):
        raise SystemExit("M21 retail corpus metadata changed")
    print("M21 retail scenarios: campaign/skirmish setup-reset-teardown ok; devices=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
