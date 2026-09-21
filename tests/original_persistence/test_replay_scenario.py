"""Exercise the original RecorderClass with project-owned source-game commands."""

import argparse
import contextlib
import importlib.util
import os
import pathlib
import re
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
        extras: dict[str, str | None]) -> subprocess.CompletedProcess:
    cwd = base / "arbitrary-cwd"
    cwd.mkdir(exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": "skirmish",
        "ZH_M21_MAP": "Maps\\Owned\\Owned.map",
        "ZH_M21_SIMULATION": "1",
        "XDG_CONFIG_HOME": str(base / "xdg/config"),
        "XDG_CACHE_HOME": str(base / "xdg/cache"),
        "XDG_DATA_HOME": str(base / "xdg/data"),
        "XDG_STATE_HOME": str(base / "xdg/state"),
    })
    for key, value in extras.items():
        if value is None:
            env.pop(key, None)
        else:
            env[key] = value
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=60)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--cross-executable", type=pathlib.Path, action="append", default=[])
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    setup = load_setup(args.source_root.resolve())
    fixture = setup.load_m20_fixture(args.source_root.resolve())
    keep = bool(os.getenv("ZH_M24_KEEP"))
    context = (contextlib.nullcontext(tempfile.mkdtemp(prefix="zh-m24-replay-")) if keep
               else tempfile.TemporaryDirectory(prefix="zh-m24-replay-"))
    with context as directory:
        base = pathlib.Path(directory)
        if keep:
            print(f"M24 retained replay fixture: {base}", flush=True)
        source = base / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        fixture.write(source / "Data/INI/Default/Multiplayer.ini",
                      "MultiplayerColor Blue\n TooltipName = COLOR:Blue\n"
                      " RGBColor = R:1 G:2 B:3\n RGBNightColor = R:1 G:2 B:3\nEND\n"
                      "MultiplayerColor Red\n TooltipName = COLOR:Red\n"
                      " RGBColor = R:3 G:2 B:1\n RGBNightColor = R:3 G:2 B:1\nEND\n")
        fixture.write(source / "Maps/Owned/Owned.map",
                      setup.owned_map(skirmish=True).replace(b"teamplayerA", b"teamplayer0"))
        fixture.make_read_only(source)
        recorded = run(args.executable.resolve(), base, source, {"ZH_M24_RECORD_REPLAY": "1"})
        if recorded.returncode or "original replay checkpoint:" not in recorded.stdout:
            raise SystemExit(f"original recording failed ({recorded.returncode}):\n"
                             f"{recorded.stdout}{recorded.stderr}")
        replays = list((base / "xdg").rglob("*.rep"))
        if len(replays) != 1 or replays[0].stat().st_size < 128:
            raise SystemExit(f"original replay not published: {replays}")
        if not replays[0].read_bytes().startswith(b"GENREP"):
            raise SystemExit("original replay magic is missing")
        if list((base / "xdg").rglob("*.part-*")):
            raise SystemExit("original replay left a staging file after publication")
        checkpoint = re.search(r"original replay checkpoint: frame=(\d+) crc=(\d+) seedcrc=(\d+) mode=(\d+)", recorded.stdout)
        if checkpoint is None:
            raise SystemExit("source recorder did not publish a replay checkpoint")
        replayed = run(args.executable.resolve(), base, source, {
            "ZH_M21_SIMULATION": None,
            "ZH_M24_REPLAY_EXISTING": replays[0].name,
            "ZH_M24_REPLAY_EXPECTED_FRAME": checkpoint.group(1),
            "ZH_M24_REPLAY_EXPECTED_CRC": checkpoint.group(2),
            "ZH_M24_REPLAY_EXPECTED_SEED_CRC": checkpoint.group(3),
        })
        if replayed.returncode or "original replay existing:" not in replayed.stdout:
            raise SystemExit(f"separate-process original replay diverged ({replayed.returncode}):\n"
                             f"{replayed.stdout}{replayed.stderr}")
        wrong_crc = run(args.executable.resolve(), base, source, {
            "ZH_M21_SIMULATION": None,
            "ZH_M24_REPLAY_EXISTING": replays[0].name,
            "ZH_M24_REPLAY_EXPECTED_FRAME": checkpoint.group(1),
            "ZH_M24_REPLAY_EXPECTED_CRC": str(int(checkpoint.group(2)) ^ 1),
            "ZH_M24_REPLAY_EXPECTED_SEED_CRC": checkpoint.group(3),
        })
        if wrong_crc.returncode == 0 or "replay CRC diverged" not in wrong_crc.stderr:
            raise SystemExit("source replay CRC mismatch was not detected")
        for cross_executable in args.cross_executable:
            crossed = run(cross_executable.resolve(), base, source, {
                "ZH_M21_SIMULATION": None,
                "ZH_M24_REPLAY_EXISTING": replays[0].name,
                "ZH_M24_REPLAY_EXPECTED_FRAME": checkpoint.group(1),
                "ZH_M24_REPLAY_EXPECTED_CRC": checkpoint.group(2),
                "ZH_M24_REPLAY_EXPECTED_SEED_CRC": checkpoint.group(3),
            })
            if crossed.returncode or "original replay existing:" not in crossed.stdout:
                raise SystemExit(f"cross-preset source replay diverged ({cross_executable}):\n"
                                 f"{crossed.stdout}{crossed.stderr}")
        data = replays[0].read_bytes()
        cursor = 6 + 2 * 8 + 4 + (2 + 8) * 4
        for index in range(3):
            while data[cursor:cursor + 2] != b"\0\0":
                cursor += 2
            cursor += 2
            if index == 0:
                cursor += 16
        version_offset = cursor
        cursor += 12
        for _ in range(2):
            cursor = data.index(b"\0", cursor) + 1
        command_offset = cursor + 16
        if command_offset + 13 >= len(data):
            raise SystemExit("original source command stream is missing")
        malformed = {
            "magic.rep": b"BADREP" + data[6:],
            "short-header.rep": data[:30],
            "short-command.rep": data[:-2],
        }
        map_offset = data.find(b"maps/owned")
        if map_offset < 0:
            raise SystemExit("source replay omitted the fixture map reference")
        malformed["missing-map.rep"] = data[:map_offset] + b"maps/ghost" + data[map_offset + 10:]
        for name, location, replacement in (
            ("version.rep", version_offset, b"\xff\xff\xff\xff"),
            ("exe-crc.rep", version_offset + 4, b"\xff\xff\xff\xff"),
            ("ini-crc.rep", version_offset + 8, b"\xff\xff\xff\xff"),
            ("command-type.rep", command_offset + 4, b"\xff\xff\xff\xff"),
            ("frame.rep", command_offset, (10000001).to_bytes(4, "little")),
        ):
            mutated = bytearray(data)
            mutated[location:location + 4] = replacement
            malformed[name] = bytes(mutated)
        for name, contents in malformed.items():
            candidate = replays[0].parent / name
            candidate.write_bytes(contents)
            rejected = run(args.executable.resolve(), base, source, {
                "ZH_M21_SIMULATION": None,
                "ZH_M24_REPLAY_EXISTING": name,
                "ZH_M24_REPLAY_REJECT": "1",
            })
            if rejected.returncode or "original replay rejected:" not in rejected.stdout:
                raise SystemExit(f"{name} altered live source state ({rejected.returncode}):\n"
                                 f"{rejected.stdout}{rejected.stderr}")
            if candidate.read_bytes() != contents:
                raise SystemExit(f"{name} changed during rejection")
            candidate.unlink()
        missing = run(args.executable.resolve(), base, source, {
            "ZH_M21_SIMULATION": None, "ZH_M24_REPLAY_EXISTING": "missing.rep",
            "ZH_M24_REPLAY_REJECT": "1",
        })
        escaped = run(args.executable.resolve(), base, source, {
            "ZH_M21_SIMULATION": None, "ZH_M24_REPLAY_EXISTING": "../00000000.rep",
            "ZH_M24_REPLAY_REJECT": "1",
        })
        if any(result.returncode or "original replay rejected:" not in result.stdout
               for result in (missing, escaped)):
            raise SystemExit("missing/traversal replay changed live source state")
        print(f"M24 source recorder: {replays[0].stat().st_size} bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
