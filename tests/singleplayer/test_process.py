#!/usr/bin/env python3
import argparse
import os
from pathlib import Path
import subprocess
import tempfile


def run(executable: Path, cwd: Path, environment: dict[str, str], *arguments: str):
    return subprocess.run([str(executable), "--single-player-integration-smoke", *arguments], cwd=cwd,
                          env=environment, text=True, capture_output=True, timeout=30)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True, type=Path)
    args = parser.parse_args()
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix="zh-single-player-process-") as temporary:
        root = Path(temporary)
        zh = root / "retail-zh"
        generals = root / "retail-generals"
        (zh / "Data/English").mkdir(parents=True)
        (zh / "Data/English/Language.ini").write_text("[Language]\n", encoding="ascii")
        (zh / "Maps/User/Acceptance").mkdir(parents=True)
        (zh / "Maps/User/Acceptance/map.ini").write_text("synthetic-map\n", encoding="ascii")
        generals.mkdir()
        cwd = root / "unrelated-working-directory"
        cwd.mkdir()
        env = os.environ.copy()
        for key, leaf in (("XDG_CONFIG_HOME", "config"), ("XDG_DATA_HOME", "data"),
                          ("XDG_STATE_HOME", "state"), ("XDG_CACHE_HOME", "cache")):
            env[key] = str(root / "xdg" / leaf)
        data = ("--zh-data", str(zh), "--generals-data", str(generals), "--language", "English")
        scenarios = [
            ("--mode", "campaign", "--scenario", "usa-campaign-01", "--faction", "usa"),
            ("--mode", "skirmish", "--scenario", "skirmish-usa", "--faction", "usa"),
            ("--mode", "skirmish", "--scenario", "skirmish-china", "--faction", "china"),
            ("--mode", "skirmish", "--scenario", "skirmish-gla", "--faction", "gla"),
            ("--mode", "user-map", "--scenario", "user-acceptance", "--faction", "china",
             "--map", "Maps/User/Acceptance/map.ini", "--outcome", "defeat"),
        ]
        for scenario in scenarios:
            result = run(args.executable.resolve(), cwd, env, *scenario, "--ticks", "80", *data)
            if result.returncode != 0 or "exited cleanly" not in result.stdout:
                failures.append(f"scenario {scenario}: rc={result.returncode} stderr={result.stderr}")
        missing = run(args.executable.resolve(), cwd, env, "--mode", "user-map", "--scenario", "missing",
                      "--faction", "usa", "--map", "Maps/User/Missing/map.ini", *data)
        if missing.returncode != 5 or "required user map is missing" not in missing.stderr:
            failures.append("missing user map did not fail with an actionable runtime diagnostic")
        bad = run(args.executable.resolve(), cwd, env, "--mode", "skirmish", "--ticks", "3", *data)
        if bad.returncode != 2 or "--ticks" not in bad.stderr:
            failures.append("invalid ticks did not fail as usage")
        if not (root / "xdg/data/generals-zero-hour/progression.txt").is_file():
            failures.append("XDG progression was not persisted")
        if any(cwd.iterdir()):
            failures.append("single-player wrote into the arbitrary current directory")
    for failure in failures:
        print(f"FAIL: {failure}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
