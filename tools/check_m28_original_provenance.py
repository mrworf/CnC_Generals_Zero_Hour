#!/usr/bin/env python3
import argparse
import pathlib
import sys


def require(path: pathlib.Path, markers: list[str], errors: list[str]) -> None:
    if not path.is_file():
        errors.append(f"missing provenance source {path}")
        return
    text = path.read_text(encoding="utf-8", errors="replace")
    for marker in markers:
        if marker not in text:
            errors.append(f"{path}: missing provenance marker {marker!r}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True, type=pathlib.Path)
    args = parser.parse_args()
    root = args.root.resolve()
    errors: list[str] = []
    extracted = root / "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/OriginalCpuPresentation.cpp"
    require(extracted, ["M28 extraction provenance", "W3DDisplay::init", "W3DAssetManager",
                        "DX8Wrapper", "provider_cpu_identity"], errors)
    require(root / "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp",
            ["void W3DDisplay::initAssets", "void W3DDisplay::init3DScene", "void W3DDisplay::init2DScene",
             "void W3DDisplay::init", "void W3DDisplay::reset"], errors)
    require(root / "GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DAssetManager.cpp",
            ["W3DAssetManager::W3DAssetManager", "WW3DAssetManager::Get_Texture"], errors)
    require(root / "GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp",
            ["bool DX8Wrapper::Init", "void DX8Wrapper::Shutdown"], errors)
    require(root / "include/zh/original_resources.h",
            ["class CpuPresentation", "physical_device", "web_browser"], errors)
    require(root / "CMakeLists.txt",
            ["add_library(zh_original_resources STATIC", "OriginalCpuPresentation.cpp"], errors)
    cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
    target = cmake.split("add_library(zh_original_resources STATIC", 1)[1].split(")", 1)[0]
    forbidden = ["W3DDisplay.cpp", "W3DAssetManager.cpp", "dx8wrapper.cpp", "dx8webbrowser.cpp"]
    for name in forbidden:
        if name in target:
            errors.append(f"coupled/device source entered independent M28 target: {name}")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print("M28 original extraction provenance: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
