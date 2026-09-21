"""Exercise original GameState snapshot writing from a source-engine scenario."""

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
        name: str, embedded_map: str = "", load: bool = False,
        load_fault: str = "", load_existing: bool = False) -> subprocess.CompletedProcess:
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
    if load:
        env["ZH_M24_LOAD_AFTER_SAVE"] = "1"
    if load_fault:
        env["ZH_M24_TEST_LOAD_FAULT"] = load_fault
    if load_existing:
        env["ZH_M24_LOAD_EXISTING"] = "1"
    return subprocess.run([str(executable)], cwd=cwd, env=env,
                          text=True, capture_output=True, timeout=60)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    setup = load_setup(args.source_root.resolve())
    fixture = setup.load_m20_fixture(args.source_root.resolve())
    keep = bool(os.getenv("ZH_M24_KEEP"))
    context = (contextlib.nullcontext(tempfile.mkdtemp(prefix="zh-m24-save-")) if keep
               else tempfile.TemporaryDirectory(prefix="zh-m24-save-"))
    with context as directory:
        base = pathlib.Path(directory)
        if keep:
            print(f"M24 retained fixture: {base}", flush=True)
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
        loaded = run(args.executable.resolve(), base, source, "original.sav", load=True)
        if loaded.returncode or "original persistence load: objects=" not in loaded.stdout:
            raise SystemExit(f"original save/load failed ({loaded.returncode}):\n{loaded.stdout}{loaded.stderr}")
        latest_save = saves[0].read_bytes()
        for fault in ("after-reset", "traversal", "postprocess"):
            failed_load = run(args.executable.resolve(), base, source, "original.sav",
                              load=True, load_fault=fault)
            if failed_load.returncode or "rollback=1" not in failed_load.stdout:
                raise SystemExit(f"{fault} failed to restore original state ({failed_load.returncode}):\n"
                                 f"{failed_load.stdout}{failed_load.stderr}")
            if list((base / "xdg/data").rglob(".rollback.*")):
                raise SystemExit(f"{fault} left a rollback snapshot")
        latest_save = saves[0].read_bytes()
        malformed = {
            "truncated.sav": latest_save[:-13],
            "trailing.sav": latest_save + b"unexpected tail",
        }
        version = bytearray(latest_save)
        marker = b"CHUNK_GameLogic"
        offset = version.index(marker) + len(marker) + 4
        if version[offset] != 11:
            raise SystemExit("original GameLogic version location changed")
        version[offset] = 255
        malformed["version.sav"] = bytes(version)
        map_bytes = (source / "Maps/Owned/Owned.map").read_bytes()
        map_at = latest_save.find(map_bytes)
        if map_at < 4 or int.from_bytes(latest_save[map_at - 4:map_at], "little") != len(map_bytes):
            raise SystemExit("original embedded-map block could not be located")
        oversized_block = bytearray(latest_save)
        oversized_block[map_at - 4:map_at] = (256 * 1024 * 1024 + 1).to_bytes(4, "little")
        malformed["map-size.sav"] = bytes(oversized_block)
        invalid_reference = bytearray(latest_save)
        reference_at = invalid_reference.find(b"LogicFixture", version.index(marker))
        if reference_at < 0:
            raise SystemExit("original object TOC reference could not be located")
        invalid_reference[reference_at:reference_at + 12] = b"UnknownThing"
        malformed["unknown-object.sav"] = bytes(invalid_reference)
        for name, contents in malformed.items():
            corrupt = saves[0].parent / name
            corrupt.write_bytes(contents)
            rejected = run(args.executable.resolve(), base, source, name,
                           load=True, load_existing=True)
            if rejected.returncode or "rollback=1" not in rejected.stdout:
                raise SystemExit(f"{name} changed the live checkpoint ({rejected.returncode}):\n"
                                 f"{rejected.stdout}{rejected.stderr}")
            if corrupt.read_bytes() != contents:
                raise SystemExit(f"{name} changed on disk during rejected load")
            corrupt.unlink()
        escaped = run(args.executable.resolve(), base, source, "../original.sav",
                      load=True, load_existing=True)
        if escaped.returncode or "rollback=1" not in escaped.stdout:
            raise SystemExit("load basename traversal changed the live checkpoint")
        unrelated_map = saves[0].parent / "user-kept.map"
        unrelated_map.write_bytes(b"user data")
        restored = run(args.executable.resolve(), base, source, "original.sav", load=True)
        if restored.returncode or "rollback=0" not in restored.stdout or unrelated_map.read_bytes() != b"user data":
            raise SystemExit("original load removed an unrelated XDG map")
        unrelated_map.unlink()
        colliding_map = saves[0].parent / "owned.map"
        colliding_map.write_bytes(b"do not overwrite")
        rejected = run(args.executable.resolve(), base, source, "original.sav",
                       load=True, load_existing=True)
        if rejected.returncode or "rollback=1" not in rejected.stdout or colliding_map.read_bytes() != b"do not overwrite":
            raise SystemExit(f"original map extraction overwrote an existing XDG map:\n"
                             f"{rejected.stdout}{rejected.stderr}")
        colliding_map.unlink()
        latest_save = saves[0].read_bytes()
        save_parent = saves[0].parent
        old_mode = save_parent.stat().st_mode & 0o777
        save_parent.chmod(0o555)
        try:
            denied = run(args.executable.resolve(), base, source, "original.sav")
            if denied.returncode == 0 or saves[0].read_bytes() != latest_save:
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
