#!/usr/bin/env python3
import argparse
import importlib.util
import os
import pathlib
import shutil
import struct
import subprocess
import tempfile


def load_m20_fixture(source_root: pathlib.Path):
    path = source_root / "tests/original_lifecycle/test_production_entry.py"
    spec = importlib.util.spec_from_file_location("m20_production_fixture", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader
    spec.loader.exec_module(module)
    return module


def ascii_string(value: str) -> bytes:
    encoded = value.encode("ascii")
    return struct.pack("<H", len(encoded)) + encoded


def dictionary(entries: list[tuple[int, int, object]]) -> bytes:
    out = bytearray(struct.pack("<H", len(entries)))
    for name_id, data_type, value in entries:
        out.extend(struct.pack("<I", (name_id << 8) | data_type))
        if data_type == 0:
            out.extend(struct.pack("<B", int(bool(value))))
        elif data_type == 1:
            out.extend(struct.pack("<i", int(value)))
        elif data_type == 3:
            out.extend(ascii_string(str(value)))
        else:
            raise AssertionError(f"unsupported fixture Dict type {data_type}")
    return bytes(out)


def chunk(chunk_id: int, version: int, payload: bytes) -> bytes:
    return struct.pack("<IHI", chunk_id, version, len(payload)) + payload


def owned_map() -> bytes:
    names = [
        "HeightMapData", "WorldInfo", "ObjectsList", "Object", "SidesList",
        "PlayerScriptsList", "ScriptList", "originalOwner", "playerName",
        "playerIsHuman", "playerFaction", "playerAllies", "playerEnemies",
        "multiplayerIsLocal",
    ]
    ids = {name: index + 1 for index, name in enumerate(names)}
    toc = bytearray(b"CkMp" + struct.pack("<I", len(names)))
    for name, value in ids.items():
        encoded = name.encode("ascii")
        toc.extend(struct.pack("<B", len(encoded)) + encoded + struct.pack("<I", value))

    height = struct.pack("<7i", 8, 8, 0, 1, 8, 8, 64) + bytes(range(64))
    owners = {
        "LogicFixture": "teamplayerA",
        "EnemyFixture": "teamplayerB",
        "AllyBase": "teamplayerA",
        "FixtureProp": "teamplayerA",
    }
    objects = bytearray()
    for name, x in (("LogicFixture", 20.0), ("EnemyFixture", 60.0),
                    ("AllyBase", 10.0), ("FixtureProp", 40.0)):
        owner = dictionary([(ids["originalOwner"], 3, owners[name])])
        body = struct.pack("<4fI", x, 20.0, 0.0, 0.0, 0) + ascii_string(name) + owner
        objects.extend(chunk(ids["Object"], 3, body))

    player_a = dictionary([
        (ids["playerName"], 3, "playerA"),
        (ids["playerIsHuman"], 0, True),
        (ids["playerFaction"], 3, "FactionPlayerA"),
        (ids["playerAllies"], 3, ""),
        (ids["playerEnemies"], 3, "playerB"),
        (ids["multiplayerIsLocal"], 0, True),
    ])
    player_b = dictionary([
        (ids["playerName"], 3, "playerB"),
        (ids["playerIsHuman"], 0, False),
        (ids["playerFaction"], 3, "FactionPlayerB"),
        (ids["playerAllies"], 3, ""),
        (ids["playerEnemies"], 3, "playerA"),
    ])
    sides = bytearray(struct.pack("<I", 2))
    sides.extend(player_a + struct.pack("<I", 0))
    sides.extend(player_b + struct.pack("<I", 0))
    sides.extend(struct.pack("<I", 0))
    scripts = chunk(ids["ScriptList"], 1, b"") + chunk(ids["ScriptList"], 1, b"")
    sides.extend(chunk(ids["PlayerScriptsList"], 1, scripts))

    toc.extend(chunk(ids["HeightMapData"], 4, height))
    toc.extend(chunk(ids["WorldInfo"], 1, dictionary([])))
    toc.extend(chunk(ids["ObjectsList"], 3, bytes(objects)))
    toc.extend(chunk(ids["SidesList"], 2, bytes(sides)))
    return bytes(toc)


def run(executable: pathlib.Path, base: pathlib.Path, source: pathlib.Path,
        scenario: str, map_name: str = "Maps\\Owned\\Owned.map") -> subprocess.CompletedProcess:
    cwd = base / f"cwd-{scenario}"
    cwd.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source),
        "ZH_M21_SCENARIO": scenario,
        "ZH_M21_MAP": map_name,
        "XDG_CONFIG_HOME": str(base / f"xdg-{scenario}/config"),
        "XDG_CACHE_HOME": str(base / f"xdg-{scenario}/cache"),
        "XDG_DATA_HOME": str(base / f"xdg-{scenario}/data"),
        "XDG_STATE_HOME": str(base / f"xdg-{scenario}/state"),
    })
    return subprocess.run([str(executable)], cwd=cwd, env=env, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=30)


def prepare_owned_source(source: pathlib.Path, fixture) -> None:
    fixture.fixture(source)
    fixture.write(source / "Data/INI/Default/Multiplayer.ini",
                  "MultiplayerColor Owned\n TooltipName = COLOR:Owned\n"
                  " RGBColor = R:1 G:2 B:3\n RGBNightColor = R:1 G:2 B:3\nEND\n")
    fixture.write(source / "Data/INI/Default/PlayerTemplate.ini",
                  "PlayerTemplate FactionCivilian\n Side = Civilian\n PlayableSide = No\nEND\n"
                  "PlayerTemplate FactionPlayerA\n Side = PlayerA\n PlayableSide = Yes\nEND\n"
                  "PlayerTemplate FactionPlayerB\n Side = PlayerB\n PlayableSide = Yes\nEND\n"
                  "PlayerTemplate FactionObserver\n Side = Observer\n PlayableSide = No\nEND\n")
    fixture.write(source / "Data/INI/Default/Object.ini",
                  "Object LogicFixture\n KindOf = SELECTABLE VEHICLE\n"
                  " WeaponSet\n  Conditions = None\n  Weapon = PRIMARY FixtureWeapon\n End\n"
                  " Body = ActiveBody ModuleTag_Body\n  MaxHealth = 100\n InitialHealth = 100\n End\n"
                  " Behavior = AIUpdateInterface ModuleTag_AI\n"
                  "  AutoAcquireEnemiesWhenIdle = No\n  MoodAttackCheckRate = 33\n End\n"
                  " Locomotor = SET_NORMAL FixtureLocomotor\n"
                  " Behavior = DestroyDie ModuleTag_Die\n End\nEnd\n"
                  "Object EnemyFixture\n KindOf = SELECTABLE STRUCTURE MP_COUNT_FOR_VICTORY\n"
                  " Body = ActiveBody ModuleTag_Body\n  MaxHealth = 100\n InitialHealth = 100\n End\nEnd\n"
                  "Object AllyBase\n KindOf = SELECTABLE STRUCTURE MP_COUNT_FOR_VICTORY\n"
                  " Body = ActiveBody ModuleTag_Body\n  MaxHealth = 100\n InitialHealth = 100\n End\nEnd\n"
                  "Object FixtureProp\n KindOf = PROP\nEnd\n")
    fixture.write(source / "Data/INI/Weapon.ini",
                  "Weapon FixtureWeapon\n PrimaryDamage = 5\n PrimaryDamageRadius = 0\n"
                  " AttackRange = 100\n DamageType = SMALL_ARMS\n DeathType = NORMAL\n"
                  " WeaponSpeed = 999999\n ProjectileObject = NONE\n DelayBetweenShots = 100\n"
                  " ClipSize = 0\n RadiusDamageAffects = ENEMIES\nEnd\n")
    fixture.write(source / "Data/INI/Locomotor.ini",
                  "Locomotor FixtureLocomotor\n Surfaces = GROUND\n Speed = 30\n SpeedDamaged = 30\n"
                  " TurnRate = 360\n TurnRateDamaged = 360\n Acceleration = 30\n"
                  " AccelerationDamaged = 30\n Braking = 30\n MinTurnSpeed = 0\n"
                  " ZAxisBehavior = NO_Z_MOTIVE_FORCE\n Appearance = TWO_LEGS\nEnd\n")
    fixture.write(source / "Maps/Owned/Owned.map", owned_map())


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", type=pathlib.Path, required=True)
    parser.add_argument("--source-root", type=pathlib.Path, required=True)
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()
    fixture = load_m20_fixture(args.source_root.resolve())
    context = tempfile.TemporaryDirectory(prefix="zh-m21-setup-")
    try:
        base = pathlib.Path(context.name)
        source = base / "readonly-input"
        prepare_owned_source(source, fixture)
        before = sorted((p.relative_to(source), p.read_bytes()) for p in source.rglob("*") if p.is_file())

        missing_template_root = base / "missing-template-input"
        shutil.copytree(source, missing_template_root)
        missing_map = (missing_template_root / "Maps/Owned/Owned.map").read_bytes()
        (missing_template_root / "Maps/Owned/Owned.map").write_bytes(
            missing_map.replace(b"LogicFixture", b"MissingThing"))

        missing_module_root = base / "missing-module-input"
        shutil.copytree(source, missing_module_root)
        fixture.write(missing_module_root / "Data/INI/Default/Object.ini",
                      "Object LogicFixture\n"
                      " Behavior = MissingBehavior ModuleTag_Missing\n End\n"
                      "End\n")

        malformed_root = base / "malformed-input"
        shutil.copytree(source, malformed_root)
        fixture.write(malformed_root / "Maps/Owned/Owned.map", b"CkM")
        fixture.make_read_only(source)
        fixture.make_read_only(missing_template_root)
        fixture.make_read_only(missing_module_root)
        fixture.make_read_only(malformed_root)
        expected = {"mission": 0, "skirmish": 2}
        for scenario, mode in expected.items():
            result = run(args.executable.resolve(), base, source, scenario)
            combined = result.stdout + result.stderr
            marker = f"original scenario setup: mode={mode} "
            if result.returncode or marker not in result.stdout:
                raise SystemExit(f"{scenario} setup failed ({result.returncode}):\n{combined}")
            for witness in ("players=", "teams=", "objects=3", "props=1",
                            "texture-preloads=", "recorder-controls=1", "devices=0"):
                if witness not in result.stdout:
                    raise SystemExit(f"{scenario} omitted {witness}:\n{combined}")

        missing = run(args.executable.resolve(), base, source, "mission", "Maps\\Missing\\Missing.map")
        if missing.returncode == 0 or "original map is missing" not in missing.stderr or "original scenario setup:" in missing.stdout:
            raise SystemExit(f"missing map did not fail closed:\n{missing.stdout}{missing.stderr}")

        missing_template = run(args.executable.resolve(), base, missing_template_root, "mission")
        if (missing_template.returncode == 0 or
                "required map object template is missing" not in missing_template.stderr or
                "original scenario setup:" in missing_template.stdout):
            raise SystemExit(f"missing template did not fail closed:\n{missing_template.stdout}{missing_template.stderr}")

        missing_module = run(args.executable.resolve(), base, missing_module_root, "mission")
        if (missing_module.returncode == 0 or
                "Object LogicFixture" not in (missing_module.stdout + missing_module.stderr) or
                "original scenario setup:" in missing_module.stdout):
            raise SystemExit(f"missing module did not fail closed:\n{missing_module.stdout}{missing_module.stderr}")

        malformed = run(args.executable.resolve(), base, malformed_root, "skirmish")
        if malformed.returncode == 0 or "original scenario setup:" in malformed.stdout:
            raise SystemExit(f"malformed map did not fail closed:\n{malformed.stdout}{malformed.stderr}")

        after = sorted((p.relative_to(source), p.read_bytes()) for p in source.rglob("*") if p.is_file())
        if before != after:
            raise SystemExit("read-only scenario input changed")
        print("M21 original mission/skirmish setup: ok maps=2 failures=closed devices=0")
        return 0
    finally:
        if args.keep:
            print(f"kept fixture: {context.name}")
            context._finalizer.detach()
        else:
            context.cleanup()


if __name__ == "__main__":
    raise SystemExit(main())
