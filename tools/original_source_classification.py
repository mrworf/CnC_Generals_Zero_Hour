#!/usr/bin/env python3
"""Generate and validate the auditable legacy-source disposition table."""

from __future__ import annotations

import argparse
import csv
import io
import pathlib
import re
import sys
from dataclasses import dataclass


ROOT = pathlib.Path(__file__).resolve().parents[1]
INVENTORY = ROOT / "cmake" / "LegacySourceInventory.cmake"
OUTPUT = ROOT / "data" / "original-source-classification.tsv"

ACTIVE_POLICY = {
    "ZH_LEGACY_ZH_COMPRESSION_SOURCES": ("excluded", "M19", "deferred until the M19 original compression provider is linked"),
    "ZH_LEGACY_ZH_WWSUPPORT_SOURCES": ("excluded", "M19", "deferred until the M19 original WW support provider is linked"),
    "ZH_LEGACY_ZH_MAIN_SOURCES": ("excluded", "M20", "original executable entry and factories are owned by M20"),
    "ZH_LEGACY_ZH_GAME_ENGINE_SOURCES": ("excluded", "M20-M25", "original engine consumers are promoted by the owning source-integration milestone"),
    "ZH_LEGACY_ZH_W3D_SOURCES": ("excluded", "M22", "original W3D producers are owned by M22"),
    "ZH_LEGACY_ZH_WWSHADE_SOURCES": ("excluded", "M22", "original WWShade producers are owned by M22"),
    "ZH_LEGACY_ZH_GAME_DEVICE_SOURCES": ("excluded", "M22-M23", "original device consumers are owned by M22 and M23"),
}

PRODUCTION_SOURCES = {
    "GeneralsMD/Code/Libraries/Source/Compression/EAC/refabout.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/Compression/EAC/refdecode.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/Compression/EAC/refencode.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/FastAllocator.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/chunkio.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/gcd_lcm.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/nstrdup.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/ramfile.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/tri.cpp": "M19",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/pointerremap.cpp": "M19",
    "GeneralsMD/Code/GameEngine/Source/Common/System/GameMemory.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/System/MemoryInit.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/System/CriticalSection.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/System/AsciiString.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/System/UnicodeString.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/version.cpp": "M26",
    "GeneralsMD/Code/GameEngine/Source/Common/RandomValue.cpp": "M27",
    "GeneralsMD/Code/GameEngine/Source/Common/crc.cpp": "M27",
    "GeneralsMD/Code/GameEngine/Source/Common/GameMain.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/SubsystemInterface.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/GlobalData.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/NameKeyGenerator.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/FileSystem.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/LocalFileSystem.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/Xfer.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/XferCRC.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/System/SaveGame/GameState.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INIGameData.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INIMapCache.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/INI/INIObject.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/Thing/ThingFactory.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/Common/Thing/ThingTemplate.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/GameClient/GameText.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/GameClient/LanguageFilter.cpp": "M20",
    "GeneralsMD/Code/GameEngine/Source/GameClient/MapUtil.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/Common/Thing/W3DModuleFactory.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DDependencyModelDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DLaserDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DModelDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordAircraftDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordTankDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DOverlordTruckDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DProjectileStreamDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DPropDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DScienceModelDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DSupplyDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTankDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTankTruckDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTreeDraw.cpp": "M20",
    "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DTruckDraw.cpp": "M20",
}

PRODUCTION_PROVIDER_RE = re.compile(r"^(?!.*(?:bootstrap|fixture|toy)).+\.(?:c|cc|cpp|cxx)$", re.IGNORECASE)


@dataclass(frozen=True)
class Record:
    path: str
    disposition: str
    provider: str
    rationale: str
    origins: tuple[str, ...]


def _sets(text: str) -> list[tuple[str, list[str]]]:
    result: list[tuple[str, list[str]]] = []
    for match in re.finditer(r"set\((ZH_LEGACY_[A-Z0-9_]+)\n(.*?)\n\)", text, re.DOTALL):
        result.append((match.group(1), re.findall(r'^\s*"([^"]+)"', match.group(2), re.MULTILINE)))
    return result


def _external_disposition(rationale: str) -> tuple[str, str]:
    folded = rationale.casefold()
    if "tool" in folded or "authoring" in folded or "developer-only" in folded or "profiling harness" in folded:
        return "tool-only", "none"
    windows_terms = ("win32", "windows", "directx", "direct3d", "mfc", "registry", "winsock", "browser", "launcher")
    if any(term in folded for term in windows_terms):
        return "platform-replaced", "native-adapter"
    return "excluded", "none"


def classify(text: str) -> list[Record]:
    grouped: dict[str, dict[str, object]] = {}
    for variable, entries in _sets(text):
        for raw in entries:
            if variable in ("ZH_LEGACY_EXCLUDED_SOURCE_RECORDS", "ZH_LEGACY_UNAVAILABLE_SOURCE_RECORDS"):
                fields = raw.split("|", 2)
                if len(fields) != 3:
                    raise ValueError(f"malformed {variable} record: {raw}")
                _, path, rationale = fields
                disposition, provider = _external_disposition(rationale)
            else:
                path = raw
                try:
                    disposition, provider, rationale = ACTIVE_POLICY[variable]
                except KeyError as exc:
                    raise ValueError(f"missing classification policy for {variable}") from exc
                if path in PRODUCTION_SOURCES:
                    disposition = "production-compiled"
                    provider = path
                    milestone = PRODUCTION_SOURCES[path]
                    if milestone == "M19":
                        rationale = "compiled and runtime-witnessed by the M19 original-support harness"
                    elif milestone == "M20":
                        rationale = "actual M20 lifecycle source compiled; live subsystem behavior is runtime-witnessed"
                    else:
                        rationale = f"compiled and runtime-witnessed by the {milestone} original-provider harness"

            if not path or "\t" in path or "\n" in path:
                raise ValueError(f"invalid inventory path: {path!r}")
            current = grouped.get(path)
            origin = variable
            candidate = {
                "disposition": disposition,
                "provider": provider,
                "rationale": rationale.replace("\t", " ").replace("\n", " "),
                "origins": [origin],
            }
            if current is None:
                grouped[path] = candidate
                continue
            current["origins"].append(origin)  # type: ignore[index, union-attr]
            # An active runtime inventory takes precedence over a duplicate project/exclusion record.
            if origin in ACTIVE_POLICY and not any(existing in ACTIVE_POLICY for existing in current["origins"][:-1]):  # type: ignore[index]
                candidate["origins"] = current["origins"]
                grouped[path] = candidate

    return [
        Record(path, str(value["disposition"]), str(value["provider"]), str(value["rationale"]), tuple(sorted(set(value["origins"]))))
        for path, value in sorted(grouped.items())
    ]


def render(records: list[Record]) -> str:
    stream = io.StringIO(newline="")
    writer = csv.writer(stream, delimiter="\t", lineterminator="\n")
    writer.writerow(("path", "disposition", "provider", "rationale", "inventory_origins"))
    for record in records:
        writer.writerow((record.path, record.disposition, record.provider, record.rationale, ",".join(record.origins)))
    return stream.getvalue()


def validate(records: list[Record]) -> None:
    allowed = {"production-compiled", "platform-replaced", "tool-only", "excluded"}
    seen: set[str] = set()
    for record in records:
        if record.path in seen:
            raise ValueError(f"duplicate classification: {record.path}")
        seen.add(record.path)
        if record.disposition not in allowed:
            raise ValueError(f"invalid disposition for {record.path}: {record.disposition}")
        if not record.provider or not record.rationale or not record.origins:
            raise ValueError(f"incomplete classification for {record.path}")
        if record.disposition == "production-compiled" and not PRODUCTION_PROVIDER_RE.match(record.provider):
            raise ValueError(f"production provider is not an original translation unit: {record.provider}")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--write", action="store_true")
    group.add_argument("--check", action="store_true")
    args = parser.parse_args(argv)

    records = classify(INVENTORY.read_text(encoding="utf-8"))
    validate(records)
    generated = render(records)
    if args.write:
        OUTPUT.write_text(generated, encoding="utf-8")
        print(f"wrote {len(records)} classified source paths")
        return 0
    if not OUTPUT.exists() or OUTPUT.read_text(encoding="utf-8") != generated:
        print(f"{OUTPUT.relative_to(ROOT)} is stale; run {pathlib.Path(__file__).name} --write", file=sys.stderr)
        return 1
    print(f"original source classification: {len(records)} paths covered exactly once")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
