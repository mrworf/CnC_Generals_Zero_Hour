#!/usr/bin/env python3
import argparse
import pathlib
import sys


EXTRACTIONS = {
    "GeneralsMD/Code/GameEngine/Source/Common/INI/OriginalINI.cpp": (
        "GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp",
        ("INI::loadDirectory", "INI::load", "INI::readLine", "INI::scanBool", "INI::scanInt", "INI::scanReal"),
    ),
    "GeneralsMD/Code/GameEngine/Source/Common/System/OriginalXfer.cpp": (
        "GeneralsMD/Code/GameEngine/Source/Common/System/Xfer.cpp",
        ("Xfer::xferBool", "Xfer::xferInt", "Xfer::xferInt64", "Xfer::xferReal", "Xfer::xferAsciiString", "Xfer::xferUnicodeString"),
    ),
    "GeneralsMD/Code/GameEngine/Source/Common/System/OriginalDataChunk.cpp": (
        "GeneralsMD/Code/GameEngine/Source/Common/System/DataChunk.cpp",
        ("DataChunkTableOfContents", "DataChunkInput::openDataChunk", "DataChunkInput::closeDataChunk"),
    ),
    "GeneralsMD/Code/GameEngine/Source/GameClient/OriginalCSF.cpp": (
        "GeneralsMD/Code/GameEngine/Source/GameClient/GameText.cpp",
        ("GameTextManager::getCSFInfo", "GameTextManager::parseCSF"),
    ),
    "GeneralsMD/Code/GameEngine/Source/GameClient/OriginalMapMetadata.cpp": (
        "GeneralsMD/Code/GameEngine/Source/GameClient/MapUtil.cpp",
        ("calcCRC", "ParseWorldDictDataChunk", "ParseSizeOnly", "loadMap"),
    ),
}

DECLARATIONS = (
    "parse_ini", "parse_csf", "write_xfer_record", "read_xfer_record",
    "write_data_chunks", "read_data_chunks", "characterize_random_streams", "load_map_catalog",
)


def validate(root: pathlib.Path) -> list[str]:
    errors: list[str] = []
    for extraction, (authority, markers) in EXTRACTIONS.items():
        extracted_text = (root / extraction).read_text(encoding="utf-8")
        authority_text = (root / authority).read_text(encoding="utf-8")
        for marker in markers:
            if marker not in extracted_text:
                errors.append(f"extraction provenance omits {marker}: {extraction}")
            if marker not in authority_text:
                errors.append(f"authoritative method disappeared: {marker}: {authority}")
    header = (root / "include/zh/original_data.h").read_text(encoding="utf-8")
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    for declaration in DECLARATIONS:
        if declaration not in header:
            errors.append(f"production-facing declaration disappeared: {declaration}")
    for extraction in EXTRACTIONS:
        if extraction not in cmake:
            errors.append(f"extracted provider not compiled by production target: {extraction}")
    ini = (root / "GeneralsMD/Code/GameEngine/Source/Common/INI/INI.cpp").read_text(encoding="utf-8")
    for coupled in ("AIData", "AudioEvent", "MapCache", "Object", "ScriptAction", "ScriptCondition"):
        if f'{{ "{coupled}"' not in ini and f'{{\t"{coupled}"' not in ini:
            errors.append(f"complete production INI registry lost coupled entry: {coupled}")
    definitions = 0
    for source in (root / "GeneralsMD/Code/GameEngine/Source").rglob("*.cpp"):
        definitions += source.read_text(encoding="utf-8", errors="replace").count("void setFPMode( void )")
    if definitions != 1:
        errors.append(f"expected one source setFPMode definition, found {definitions}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=pathlib.Path, required=True)
    args = parser.parse_args()
    errors = validate(args.root.resolve())
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("M27 original extraction provenance: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
