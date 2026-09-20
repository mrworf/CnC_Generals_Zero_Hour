#!/usr/bin/env python3
"""Exercise two real headless peers on distinct Linux loopback identities."""

from __future__ import annotations

import argparse
import os
import pathlib
import subprocess
import tempfile
import time


def peer_command(
    executable: pathlib.Path,
    state: pathlib.Path,
    role: str,
    bind: str,
    *,
    direct: str | None = None,
    data_identity: int = 111,
    timeout: int = 3000,
) -> list[str]:
    command = [
        str(executable),
        "--headless",
        "--ticks",
        "1",
        "--state-dir",
        str(state),
        "--lan-role",
        role,
        "--lan-bind-address",
        bind,
        "--lan-discovery-address",
        "127.255.255.255",
        "--lan-data-identity",
        str(data_identity),
        "--lan-map-identity",
        "222",
        "--lan-timeout-ms",
        str(timeout),
    ]
    if direct is not None:
        command.extend(["--lan-direct-connect", direct])
    return command


def launch_pair(executable: pathlib.Path, root: pathlib.Path, mismatch: bool = False) -> tuple[tuple[int, str, str], tuple[int, str, str]]:
    host_state = root / "host-state"
    join_state = root / "join-state"
    host_cwd = root / "host-cwd"
    join_cwd = root / "join-cwd"
    host_cwd.mkdir()
    join_cwd.mkdir()
    environment = os.environ.copy()
    environment.update({"DISPLAY": "invalid", "WAYLAND_DISPLAY": "invalid", "SDL_AUDIODRIVER": "invalid"})
    timeout = 1200 if mismatch else 3000
    host = subprocess.Popen(
        peer_command(executable, host_state, "host", "127.0.0.2", timeout=timeout),
        cwd=host_cwd,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    time.sleep(0.05)
    joiner = subprocess.Popen(
        peer_command(
            executable,
            join_state,
            "joiner",
            "127.0.0.3",
            direct="127.0.0.2",
            data_identity=112 if mismatch else 111,
            timeout=timeout,
        ),
        cwd=join_cwd,
        env=environment,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    join_stdout, join_stderr = joiner.communicate(timeout=8)
    host_stdout, host_stderr = host.communicate(timeout=8)
    assert list(host_cwd.iterdir()) == []
    assert list(join_cwd.iterdir()) == []
    return (host.returncode, host_stdout, host_stderr), (joiner.returncode, join_stdout, join_stderr)


def run() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True, type=pathlib.Path)
    args = parser.parse_args()
    executable = args.executable.resolve(strict=True)
    with tempfile.TemporaryDirectory(prefix="zh-lan-processes-") as temporary:
        root = pathlib.Path(temporary)
        success_root = root / "success"
        success_root.mkdir()
        host, joiner = launch_pair(executable, success_root)
        for result in (host, joiner):
            code, stdout, stderr = result
            assert code == 0, result
            assert stderr == "", result
            assert "lan: joined peer=" in stdout
            assert "data-identity=111" in stdout and "map-identity=222" in stdout
            assert "lan: disconnected cleanly" in stdout
            assert "display and input skipped" in stdout and "GPU skipped" in stdout
        assert "lan: command received=m11-command" in host[1]
        assert "lan: command sent=m11-command" in joiner[1]
        assert "127.0.0.3:8086" in host[1]
        assert "127.0.0.2:8086" in joiner[1]
        assert (success_root / "host-state" / "last-headless-run.txt").exists()
        assert (success_root / "join-state" / "last-headless-run.txt").exists()

        mismatch_root = root / "mismatch"
        mismatch_root.mkdir()
        bad_host, bad_joiner = launch_pair(executable, mismatch_root, mismatch=True)
        assert bad_joiner[0] != 0, bad_joiner
        assert "data identity mismatch" in bad_joiner[1] + bad_joiner[2]
        assert bad_host[0] != 0, bad_host

        timeout_state = root / "timeout-state"
        timeout_cwd = root / "timeout-cwd"
        timeout_cwd.mkdir()
        timed_out = subprocess.run(
            peer_command(executable, timeout_state, "joiner", "127.0.0.3", direct="127.0.0.4", timeout=200),
            cwd=timeout_cwd,
            text=True,
            capture_output=True,
            timeout=4,
            check=False,
        )
        assert timed_out.returncode != 0, timed_out
        assert "timed out" in timed_out.stdout + timed_out.stderr, timed_out
    return 0


if __name__ == "__main__":
    raise SystemExit(run())
