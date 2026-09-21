"""Exercise original GameState snapshot writing from a source-engine scenario."""

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


def run(executable: pathlib.Path, base: pathlib.Path, source: pathlib.Path,
        name: str, embedded_map: str = "") -> subprocess.CompletedProcess:
    cwd = base / "arbitrary-cwd"
    cwd.mkdir(exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": "mission",
        "ZH_M21_MAP": "Maps\\Owned\\Owned.map",
        "ZH_M21_SIMULATION": "1",
        "ZH_M24_SAVE_FILENAME": name,
        "XDG_CONFIG_HOME": str(base / "xdg/config"),
        "XDG_CACHE_HOME": str(base / "xdg/cache"),
        "XDG_DATA_HOME": str(base / "xdg/data"),
        "XDG_STATE_HOME": str(base / "xdg/state"),
    })
    if embedded_map:
        env["ZH_M24_TEST_EMBEDDED_MAP"] = embedded_map
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=60)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    setup = load_setup(args.source_root.resolve())
    fixture = setup.load_m20_fixture(args.source_root.resolve())
    with tempfile.TemporaryDirectory(prefix="zh-m24-save-") as directory:
        base = pathlib.Path(directory)
        source = base / "readonly-input"
        setup.prepare_owned_source(source, fixture)
        oversized_map = source / "Maps/Oversize/Oversize.map"
        oversized_map.parent.mkdir(parents=True)
        with oversized_map.open("wb") as stream:
            stream.truncate(256 * 1024 * 1024 + 1)
        fixture.make_read_only(source)
        before = sorted((p.relative_to(source), p.read_bytes())
                        for p in source.rglob("*") if p.is_file() and p != oversized_map)
        result = run(args.executable.resolve(), base, source, "original.sav")
        if result.returncode or "original simulation checkpoint:" not in result.stdout:
            raise SystemExit(f"original save failed ({result.returncode}):\n{result.stdout}{result.stderr}")
        saves = list((base / "xdg/data").rglob("*.sav"))
        if len(saves) != 1 or not saves[0].is_file() or saves[0].stat().st_size <= 128:
            raise SystemExit(f"original XDG save was not published: {len(saves)}")
        data = saves[0].read_bytes()
        for block in (b"CHUNK_GameStateMap", b"CHUNK_Players", b"CHUNK_GameLogic",
                      b"CHUNK_ScriptEngine", b"CHUNK_SidesList"):
            if block not in data:
                raise SystemExit(f"original save omitted {block!r}")
        invalid_name = run(args.executable.resolve(), base, source, "../original.sav")
        if invalid_name.returncode == 0 or saves[0].read_bytes() != data:
            raise SystemExit("invalid save basename changed existing save")
        failed_embed = run(args.executable.resolve(), base, source, "original.sav",
                           "Maps\\Missing\\Missing.map")
        if failed_embed.returncode == 0 or saves[0].read_bytes() != data:
            raise SystemExit("failed original map embedding changed existing save")
        oversized = run(args.executable.resolve(), base, source, "original.sav",
                        "Maps\\Oversize\\Oversize.map")
        if oversized.returncode == 0 or saves[0].read_bytes() != data:
            raise SystemExit("oversized original map changed existing save")
        leftovers = list((base / "xdg/data").rglob("*.tmp.*"))
        if leftovers:
            raise SystemExit(f"failed original save left {len(leftovers)} temporary files")
        autosave = run(args.executable.resolve(), base, source, "")
        if autosave.returncode or "original simulation checkpoint:" not in autosave.stdout:
            raise SystemExit(f"original numbered autosave/metadata failed:\n{autosave.stdout}{autosave.stderr}")
        auto_files = [p for p in (base / "xdg/data").rglob("*.sav")
                      if p.name.endswith("00000000.sav")]
        if len(auto_files) != 1 or auto_files[0].stat().st_size <= 128:
            raise SystemExit("original numbered autosave was not published")
        save_parent = saves[0].parent
        old_mode = save_parent.stat().st_mode & 0o777
        save_parent.chmod(0o555)
        try:
            denied = run(args.executable.resolve(), base, source, "original.sav")
            if denied.returncode == 0 or saves[0].read_bytes() != data:
                raise SystemExit("write denial changed existing original save")
        finally:
            save_parent.chmod(old_mode)
        after = sorted((p.relative_to(source), p.read_bytes())
                       for p in source.rglob("*") if p.is_file() and p != oversized_map)
        if before != after:
            raise SystemExit("read-only source changed during save")
        print("M24 original source snapshot: full XDG blocks; invalid name and mid-write failure preserve prior save")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
