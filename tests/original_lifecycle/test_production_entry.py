#!/usr/bin/env python3
import argparse
import os
import pathlib
import stat
import struct
import subprocess
import tempfile


SUCCESS = ("original production lifecycle: logic=0>1>reset:0>1 "
           "client=0>0>reset:0>0 benchmark=-1 allocations=0 workers=0 devices=0")
BENCHMARK_SUCCESS = SUCCESS.replace("benchmark=-1", "benchmark=1")


def tag(value: bytes) -> int:
    return int.from_bytes(value, "big")


def csf() -> bytes:
    out = bytearray(struct.pack("<6I", tag(b"CSF "), 3, 2, 2, 0, 0))
    for label, text in (("GUI:Command&ConquerGenerals", "Zero Hour Fixture"),
                        ("MAP:Fixture", "Localized Fixture")):
        encoded = text.encode("utf-16le")
        encoded = b"".join(struct.pack("<H", (~struct.unpack_from("<H", encoded, i)[0]) & 0xffff)
                           for i in range(0, len(encoded), 2))
        out.extend(struct.pack("<3I", tag(b"LBL "), 1, len(label)))
        out.extend(label.encode("ascii"))
        out.extend(struct.pack("<2I", tag(b"STR "), len(text)))
        out.extend(encoded)
    return bytes(out)


DIRECT_INIS = (
    "Data/INI/Default/GameData.ini", "Data/INI/Default/Water.ini", "Data/INI/Water.ini",
    "Data/INI/Default/Weather.ini", "Data/INI/Weather.ini", "Data/INI/GameLOD.ini",
    "Data/INI/GameLODPresets.ini", "Data/INI/Default/Science.ini",
    "Data/INI/Default/Multiplayer.ini", "Data/INI/Default/Terrain.ini",
    "Data/INI/Default/Roads.ini", "Data/INI/Rank.ini",
    "Data/INI/Default/PlayerTemplate.ini", "Data/INI/Default/FXList.ini",
    "Data/INI/Weapon.ini", "Data/INI/Default/ObjectCreationList.ini",
    "Data/INI/Locomotor.ini", "Data/INI/Default/SpecialPower.ini",
    "Data/INI/DamageFX.ini", "Data/INI/Armor.ini", "Data/INI/Default/Object.ini",
    "Data/INI/Default/Upgrade.ini", "Data/INI/DrawGroupInfo.ini", "Data/INI/Mouse.ini",
    "Data/INI/Animation2D.ini", "Data/INI/WindowTransitions.ini",
    "Data/INI/Default/ShellMenuScheme.ini", "Data/INI/ShellMenuScheme.ini",
    "Data/INI/Default/CommandButton.ini", "Data/INI/CommandButton.ini", "Data/INI/CommandSet.ini",
    "Data/INI/Default/ControlBarScheme.ini", "Data/INI/ControlBarScheme.ini",
    "Data/INI/ControlBarResizer.ini",
    "Data/INI/InGameUI.ini", "Data/INI/ChallengeMode.ini", "Data/INI/Campaign.ini",
    "Data/INI/Eva.ini", "Data/INI/ParticleSystem.ini", "Data/INI/Default/Video.ini",
    "Data/INI/Video.ini", "Data/INI/Default/AIData.ini", "Data/INI/Default/Crate.ini",
    "Data/INI/AudioSettings.ini", "Data/INI/Default/Music.ini", "Data/INI/Music.ini",
    "Data/INI/Default/SoundEffects.ini", "Data/INI/SoundEffects.ini",
    "Data/INI/Default/Speech.ini", "Data/INI/Speech.ini", "Data/INI/Default/Voice.ini",
    "Data/INI/Voice.ini", "Data/INI/MiscAudio.ini", "Data/English/Language.ini",
    "Data/English/HeaderTemplate.ini", "Data/English/CommandMap.ini",
)


def write(path: pathlib.Path, data: bytes | str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(data, str):
        path.write_text(data, encoding="ascii")
    else:
        path.write_bytes(data)


def window_block(name: str, child: bool = False) -> str:
    end = "CHILD\n" if child else "END\n"
    return ("WINDOW\nWINDOWTYPE = USER;\n"
            "SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 10 10 CREATIONRESOLUTION: 800 600;\n"
            f"NAME = \"{name}\";\nSTATUS = ENABLED;\nSTYLE = USER;\n{end}")


def window_fixture(parent: str, children: list[str]) -> str:
    return ("FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n" +
            window_block(parent, True) +
            "".join(window_block(name) for name in children) +
            "ENDALLCHILDREN\nEND\n")


def control_bar_fixture() -> str:
    prefix = "ControlBar.wnd:"
    ordinary = [
        "UnderConstructionWindow", "OCLTimerWindow", "BeaconWindow", "ProductionQueueWindow",
        "ObserverPlayerListWindow", "ObserverPlayerInfoWindow", "WinUnitSelected", "CameoWindow",
        "PopupCommunicator", "ButtonOptions", "ButtonIdleWorker", "ButtonPlaceBeacon",
        "ButtonGeneral", "ButtonLarge", "PowerWindow", "MoneyDisplay", "GeneralsExp",
        "WinUAttack", "BackgroundMarker", "ButtonQueue", "TextEntryGeneralName",
    ]
    body = "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\n"
    body += window_block(prefix + "ControlBarParent", True)
    for name in ordinary:
        body += window_block(prefix + name)
    body += window_block(prefix + "CommandWindow", True)
    body += "".join(window_block(prefix + f"ButtonCommand{i:02d}") for i in range(1, 19))
    body += "ENDALLCHILDREN\nEND\n"
    body += window_block(prefix + "RightHUD", True)
    body += "".join(window_block(prefix + f"UnitUpgrade{i}") for i in range(1, 6))
    body += "ENDALLCHILDREN\nEND\n"
    body += "ENDALLCHILDREN\nEND\n"
    return body


def fixture(root: pathlib.Path) -> None:
    for name in DIRECT_INIS:
        write(root / name, "")
    write(root / "Data/INI/Default/GameData.ini",
          "GameData\n Windowed = Yes\n XResolution = 800\n YResolution = 600\n"
          " FramesPerSecondLimit = 1000\n AudioOn = No\n MusicOn = No\n SoundsOn = No\n"
          " SpeechOn = No\n ShellMapOn = No\n PlayIntro = No\nEND\n")
    write(root / "Data/INI/Default/Weather.ini",
          "Weather\n SnowTexture = FixtureSnow\n SnowFrequencyScaleX = 1\n"
          " SnowFrequencyScaleY = 1\n SnowAmplitude = 0\n SnowPointSize = 1\n"
          " SnowMaxPointSize = 1\n SnowMinPointSize = 1\n SnowQuadSize = 1\n"
          " SnowBoxDimensions = 1\n SnowBoxDensity = 1\n SnowVelocity = 0\n"
          " SnowPointSprites = No\n SnowEnabled = No\nEND\n")
    write(root / "Data/INI/Default/Water.ini", "WaterTransparency\nEND\n")
    write(root / "Data/INI/Default/Object.ini",
          "Object FixtureLaser\n"
          " Draw = W3DLaserDraw ModuleTag_FixtureLaser\n"
          "  NumBeams = 3\n"
          "  InnerBeamWidth = 1.25\n"
          "  OuterBeamWidth = 4.5\n"
          "  InnerColor = R:255 G:64 B:32\n"
          "  OuterColor = R:16 G:32 B:255\n"
          "  Texture = FixtureLaserTexture\n"
          "  Tile = Yes\n"
          "  Segments = 7\n"
          "  ArcHeight = 2.5\n"
          "  SegmentOverlapRatio = 0.2\n"
          "  TilingScalar = 3.0\n"
          " End\n"
          "End\n")
    write(root / "Data/English/Generals.csf", csf())
    write(root / "Maps/MapCache.ini",
          "MapCache Maps\\Fixture\\Fixture.map\n fileSize = 1234\n fileCRC = 305419896\n"
          " timestampLo = 11\n timestampHi = 22\n isOfficial = yes\n isMultiplayer = yes\n"
          " numPlayers = 2\n extentMin = X:0 Y:0 Z:0\n extentMax = X:100 Y:80 Z:0\n"
          " nameLookupTag = MAP:Fixture\n Player_1_Start = X:10 Y:20 Z:0\n"
          " Player_2_Start = X:90 Y:60 Z:0\n techPosition = X:50 Y:40 Z:0\n"
          " supplyPosition = X:25 Y:30 Z:0\nEND\n")
    write(root / "Menus/BlankWindow.wnd",
          "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\nWINDOW\nWINDOWTYPE = USER;\n"
          "SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
          "NAME = \"BlankWindow\";\nSTATUS = ENABLED IMAGE;\nSTYLE = USER;\nEND\n")
    write(root / "Menus/MainMenu.wnd",
          "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\nWINDOW\nWINDOWTYPE = USER;\n"
          "SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 800 600 CREATIONRESOLUTION: 800 600;\n"
          "NAME = \"MainMenu.wnd:MainMenuParent\";\nSTATUS = ENABLED;\nSTYLE = USER;\nEND\n")
    write(root / "Window/ControlBar.wnd", control_bar_fixture())
    science_names = ([f"ButtonRank1Number{i}" for i in range(4)] +
                     [f"ButtonRank3Number{i}" for i in range(15)] +
                     [f"ButtonRank8Number{i}" for i in range(4)] +
                     ["ProgressBarExperience", "StaticTextLevel", "StaticTextRankPointsAvailable",
                      "StaticTextTitle"])
    write(root / "Window/GeneralsExpPoints.wnd", window_fixture("GeneralsExpPoints.wnd:GenExpParent",
          [f"GeneralsExpPoints.wnd:{name}" for name in science_names]))
    write(root / "Window/ReplayControl.wnd",
          "FILE_VERSION = 2\nSTARTLAYOUTBLOCK\nENDLAYOUTBLOCK\nWINDOW\nWINDOWTYPE = USER;\n"
          "SCREENRECT = UPPERLEFT: 0 0 BOTTOMRIGHT: 1 1 CREATIONRESOLUTION: 800 600;\n"
          "NAME = \"ReplayControl.wnd:ReplayControl\";\nSTATUS = ENABLED;\nSTYLE = USER;\nEND\n")
    (root / "Data/INI/MappedImages/TextureSize_512").mkdir(parents=True)
    (root / "Data/INI/MappedImages/HandCreated").mkdir(parents=True)


def make_read_only(root: pathlib.Path) -> None:
    for path in root.rglob("*"):
        path.chmod(stat.S_IRUSR | (stat.S_IXUSR if path.is_dir() else 0))
    root.chmod(stat.S_IRUSR | stat.S_IXUSR)


def run(executable: str, base: pathlib.Path, source: pathlib.Path, *arguments: str,
        env_overrides=None):
    cwd = base / "arbitrary-cwd"
    cwd.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update({
        "ZH_DATA_ROOT": str(source), "ZH_M20_HEADLESS_PROFILE": "1",
        "XDG_CONFIG_HOME": str(base / "xdg/config"),
        "XDG_CACHE_HOME": str(base / "xdg/cache"),
        "XDG_DATA_HOME": str(base / "xdg/data"),
        "XDG_STATE_HOME": str(base / "xdg/state"),
    })
    if env_overrides:
        env.update(env_overrides)
    return subprocess.run([executable, *arguments], cwd=cwd, env=env, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=20)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--executable", required=True)
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()
    executable = str(pathlib.Path(args.executable).resolve())
    context = tempfile.TemporaryDirectory(prefix="zh-m20-production-")
    temp = context.name
    try:
        base = pathlib.Path(temp)
        source = base / "readonly-input"
        fixture(source)
        before = sorted((p.relative_to(source), p.read_bytes()) for p in source.rglob("*") if p.is_file())
        make_read_only(source)
        result = run(executable, base, source)
        if result.returncode:
            raise SystemExit(f"production entry failed ({result.returncode}):\n{result.stdout}{result.stderr}")
        if SUCCESS not in result.stdout:
            raise SystemExit(f"production entry omitted source-state proof:\n{result.stdout}{result.stderr}")
        warm = run(executable, base, source)
        if warm.returncode or SUCCESS not in warm.stdout:
            raise SystemExit(f"warm production entry failed ({warm.returncode}):\n{warm.stdout}{warm.stderr}")
        benchmark = run(executable, base, source, "-benchmark", "1")
        if benchmark.returncode or BENCHMARK_SUCCESS not in benchmark.stdout:
            raise SystemExit(f"bounded benchmark failed ({benchmark.returncode}):\n"
                             f"{benchmark.stdout}{benchmark.stderr}")
        after = sorted((p.relative_to(source), p.read_bytes()) for p in source.rglob("*") if p.is_file())
        if before != after:
            raise SystemExit("read-only input tree changed")

        # Source-controlled cache-build is a successful early-exit mode. It
        # must not enter the bounded update/reset lifecycle.
        cache_build = run(executable, base, source, "-buildmapcache")
        if cache_build.returncode or SUCCESS in cache_build.stdout:
            raise SystemExit(f"cache-build early exit failed ({cache_build.returncode}):\n"
                             f"{cache_build.stdout}{cache_build.stderr}")

        denied_cache = base / "denied-cache-file"
        denied_cache.write_text("not a directory", encoding="ascii")
        denied = run(executable, base / "denied-run", source,
                     env_overrides={"XDG_CACHE_HOME": str(denied_cache)})
        denied_output = denied.stdout + denied.stderr
        if denied.returncode == 0 or "cannot create XDG output directory" not in denied_output or SUCCESS in denied_output:
            raise SystemExit(f"denied XDG output did not fail closed ({denied.returncode}):\n{denied_output}")

        # Each malformed/missing dispatch case must preserve its primary
        # diagnostic and must never reach the execute/reset success marker.
        variants = []
        missing = base / "missing-input"
        fixture(missing)
        (missing / "Data/INI/Default/GameData.ini").unlink()
        variants.append((missing, "Data\\INI\\Default\\GameData.ini"))
        malformed = base / "malformed-input"
        fixture(malformed)
        write(malformed / "Data/INI/Default/GameData.ini", "GameData\n NotARealField = yes\nEND\n")
        variants.append((malformed, "Data\\INI\\Default\\GameData.ini"))
        unknown = base / "unknown-input"
        fixture(unknown)
        write(unknown / "Data/INI/Rank.ini", "UnknownRequiredBlock Fixture\nEND\n")
        variants.append((unknown, "UnknownRequiredBlock"))
        unknown_w3d = base / "unknown-w3d-input"
        fixture(unknown_w3d)
        write(unknown_w3d / "Data/INI/Default/Object.ini",
              "Object FixtureUnknownW3D\n"
              " Draw = W3DUnknownDraw ModuleTag_Missing\n"
              " End\n"
              "End\n")
        variants.append((unknown_w3d, "Object FixtureUnknownW3D"))
        malformed_w3d = base / "malformed-w3d-input"
        fixture(malformed_w3d)
        write(malformed_w3d / "Data/INI/Default/Object.ini",
              "Object FixtureMalformedW3D\n"
              " Draw = W3DLaserDraw ModuleTag_Malformed\n"
              "  NumBeams = not-a-number\n"
              " End\n"
              "End\n")
        variants.append((malformed_w3d, "Object FixtureMalformedW3D"))
        for variant, diagnostic in variants:
            make_read_only(variant)
            failure = run(executable, base / (variant.name + "-run"), variant)
            combined = failure.stdout + failure.stderr
            if failure.returncode == 0 or diagnostic not in combined or SUCCESS in combined:
                raise SystemExit(f"negative production case failed contract ({failure.returncode}):\n{combined}")
        print("original production entry: ok")
        return 0
    finally:
        if args.keep:
            print(f"kept fixture: {temp}")
            context._finalizer.detach()
        else:
            context.cleanup()


if __name__ == "__main__":
    raise SystemExit(main())
