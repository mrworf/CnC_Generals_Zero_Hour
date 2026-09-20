#!/usr/bin/env python3
"""Process-level proof that headless runs isolate writable state and need no devices."""

from __future__ import annotations

import argparse
import os
import pathlib
import subprocess
import tempfile


def run() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True, type=pathlib.Path)
    args = parser.parse_args()
    executable = args.executable.resolve(strict=True)

    with tempfile.TemporaryDirectory(prefix="zh-headless-isolation-") as temporary:
        base = pathlib.Path(temporary)
        roots = [base / "state-a", base / "state-b"]
        working = [base / "cwd-a", base / "cwd-b"]
        for directory in working:
            directory.mkdir()

        environment = os.environ.copy()
        environment.update(
            {
                "DISPLAY": "invalid-headless-display",
                "WAYLAND_DISPLAY": "invalid-headless-wayland",
                "SDL_VIDEODRIVER": "invalid-headless-video-driver",
                "SDL_AUDIODRIVER": "invalid-headless-audio-driver",
            }
        )
        commands = [
            [str(executable), "--headless", "--ticks", "1000000", "--state-dir", str(roots[0])],
            [str(executable), "--headless", "--ticks", "999999", "--state-dir", str(roots[1])],
        ]

        # Launch both before waiting for either: process isolation must not rely on
        # a global mutex, process working directory, or shared writable location.
        processes = [
            subprocess.Popen(
                command,
                cwd=working[index],
                env=environment,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )
            for index, command in enumerate(commands)
        ]
        results = [process.communicate(timeout=20) for process in processes]

        expected_ticks = ["1000000", "999999"]
        for index, process in enumerate(processes):
            stdout, stderr = results[index]
            assert process.returncode == 0, (process.returncode, stdout, stderr)
            assert stderr == "", stderr
            assert f"headless: completed ticks={expected_ticks[index]}" in stdout
            assert str(roots[index]) in stdout
            assert str(roots[1 - index]) not in stdout
            assert "SDL=" in stdout and "(not initialized)" in stdout
            assert "display and input skipped" in stdout
            assert "GPU skipped" in stdout
            assert "audio device skipped" in stdout
            assert "network skipped" in stdout

            completion = (roots[index] / "last-headless-run.txt").read_text(encoding="utf-8")
            log = (roots[index] / "logs" / "headless.log").read_text(encoding="utf-8")
            assert f"ticks={expected_ticks[index]}" in completion
            assert f"ticks={expected_ticks[index]}" in log
            assert str(roots[1 - index]) not in completion + log
            assert list(working[index].iterdir()) == []

        actual_top_level = {path.name for path in base.iterdir()}
        assert actual_top_level == {"state-a", "state-b", "cwd-a", "cwd-b"}, actual_top_level
    return 0


if __name__ == "__main__":
    raise SystemExit(run())
