#!/usr/bin/env python3
import argparse
import os
import pathlib
import subprocess
import tempfile


SUCCESS = ("original production lifecycle: logic=0>1>reset:0>1 "
           "client=0>0>reset:0>0 benchmark=-1 allocations=0 workers=0 devices=0")


def run(executable: pathlib.Path, zh_data_root: pathlib.Path,
        generals_data_root: pathlib.Path, state: pathlib.Path):
    cwd = state / "arbitrary-cwd"
    cwd.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(zh_data_root),
        "ZH_GENERALS_DATA_ROOT": str(generals_data_root),
        "ZH_M20_HEADLESS_PROFILE": "1",
        "XDG_CONFIG_HOME": str(state / "xdg/config"),
        "XDG_CACHE_HOME": str(state / "xdg/cache"),
        "XDG_DATA_HOME": str(state / "xdg/data"),
        "XDG_STATE_HOME": str(state / "xdg/state"),
    })
    return subprocess.run(
        [str(executable)], cwd=cwd, env=env, text=True,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=120,
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--zh-data-root", type=pathlib.Path, required=True)
    parser.add_argument("--generals-data-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    executable = args.executable.resolve()
    zh_data_root = args.zh_data_root.resolve()
    generals_data_root = args.generals_data_root.resolve()
    if not zh_data_root.is_dir() or not generals_data_root.is_dir():
        raise SystemExit("M20 retail gate was enabled without both readable data roots")

    with tempfile.TemporaryDirectory(prefix="zh-m20-retail-gate-") as directory:
        state = pathlib.Path(directory)
        for phase in ("cold", "warm"):
            result = run(executable, zh_data_root, generals_data_root, state)
            if result.returncode != 0:
                raise SystemExit(f"M20 retail {phase} lifecycle failed with exit {result.returncode}; private output redacted")
            if SUCCESS not in result.stdout:
                raise SystemExit(f"M20 retail {phase} lifecycle omitted bounded source-state/ownership proof")
            if "devices=0" not in result.stdout or "allocations=0" not in result.stdout or "workers=0" not in result.stdout:
                raise SystemExit(f"M20 retail {phase} lifecycle violated the no-device/ownership boundary")
    print("M20 retail lifecycle: cold/warm init-update-reset-teardown ok; devices=0 allocations=0 workers=0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
