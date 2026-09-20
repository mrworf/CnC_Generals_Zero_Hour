#!/usr/bin/env python3
"""Generate and audit the explicit Zero Hour VC6 source inventory."""

from __future__ import annotations

import argparse
import collections
import os
from pathlib import Path, PureWindowsPath
import re
import subprocess
import sys


TARGET_BY_MANIFEST = {
    "GeneralsMD/Code/RTS.dsp": "zh_main",
    "GeneralsMD/Code/GameEngine/GameEngine.dsp": "zh_game_engine",
    "GeneralsMD/Code/GameEngineDevice/GameEngineDevice.dsp": "zh_game_device",
    "GeneralsMD/Code/Libraries/Source/Compression/Compression.dsp": "zh_compression",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/ww3d2.dsp": "zh_w3d",
    "GeneralsMD/Code/Libraries/Source/WWVegas/wwshade/wwshade.dsp": "zh_wwshade",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWDebug/wwdebug.dsp": "zh_wwsupport",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/wwlib.dsp": "zh_wwsupport",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWMath/wwmath.dsp": "zh_wwsupport",
    "GeneralsMD/Code/Libraries/Source/WWVegas/WWSaveLoad/wwsaveload.dsp": "zh_wwsupport",
    "GeneralsMD/Code/Libraries/Source/WWVegas/Wwutil/wwutil.dsp": "zh_wwsupport",
}

EXCLUDED_PROJECT_RULES = (
    ("/Tools/", "authoring or maintenance tool outside the game runtime"),
    ("/Libraries/Source/GameSpy/", "retired GameSpy service dependency"),
    ("/Libraries/Source/Benchmark/", "unused legacy benchmark target"),
    ("/Libraries/Source/EABrowserDispatch/", "obsolete embedded browser dependency"),
    ("/Libraries/Source/WWVegas/WWDownload/", "retired download service"),
    ("/Libraries/Source/WPAudio/", "obsolete Windows audio implementation replaced at M12"),
    ("/Libraries/Source/WWVegas/WWAudio/", "obsolete Windows audio implementation replaced at M12"),
    ("/Libraries/Source/debug/", "legacy developer-only diagnostic harness"),
    ("/Libraries/Source/profile/", "legacy developer-only profiling harness"),
)

SOURCE_EXCLUSION_RULES = (
    ("/GameSpy/", "retired GameSpy service path"),
    ("/GameNetwork/GameSpy", "retired GameSpy service path"),
    ("/WOL", "retired online-service path"),
    ("EABrowser", "obsolete embedded browser path"),
    ("WebBrowser", "obsolete embedded browser path"),
    ("DownloadManager", "retired download service path"),
    ("Win32", "Win32 platform implementation replaced by SDL/POSIX"),
    ("DirectX", "DirectX implementation replaced by SDL_GPU"),
    ("Bink", "Bink SDK implementation replaced by FFmpeg at M13"),
    ("Miles", "Miles implementation replaced by miniaudio at M12"),
    ("WPAudio", "Windows audio implementation replaced at M12"),
    ("Granny", "excluded proprietary animation dependency"),
    ("SafeDisc", "obsolete DRM path"),
)

PROJECT_RE = re.compile(r'^Project: "([^"]+)"=([^ ]+) - Package Owner=', re.MULTILINE)
SOURCE_RE = re.compile(r"^SOURCE=(.+?)\s*$", re.MULTILINE)


def tracked_manifests(root: Path) -> list[Path]:
    result = subprocess.run(
        ["git", "ls-files", "GeneralsMD/Code/*.dsp", "GeneralsMD/Code/**/*.dsp"],
        cwd=root,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
    )
    return [root / line for line in sorted(set(result.stdout.splitlines())) if line]


def file_lookup(root: Path) -> dict[str, str]:
    prefix = root / "GeneralsMD" / "Code"
    lookup: dict[str, str] = {}
    for directory, _, files in os.walk(prefix):
        for name in files:
            relative = (Path(directory) / name).relative_to(root).as_posix()
            lookup[relative.casefold()] = relative
    return lookup


def normalize_source(root: Path, manifest: Path, raw: str, lookup: dict[str, str]) -> tuple[str, bool]:
    value = raw.strip().strip('"').replace("\\", "/")
    if "$(" in value or value.startswith("/") or re.match(r"^[A-Za-z]:", value):
        return value, False
    windows_parts = PureWindowsPath(value).parts
    candidate = manifest.parent
    for part in windows_parts:
        if part in (".", ""):
            continue
        if part == "..":
            candidate = candidate.parent
        else:
            candidate = candidate / part
    try:
        relative = candidate.relative_to(root).as_posix()
    except ValueError:
        return candidate.as_posix(), False
    actual = lookup.get(relative.casefold())
    return (actual or relative), actual is not None


def project_exclusion(manifest: str) -> str:
    for marker, reason in EXCLUDED_PROJECT_RULES:
        if marker.casefold() in ("/" + manifest).casefold():
            return reason
    return "project is outside the selected Zero Hour runtime solution"


def source_exclusion(source: str) -> str | None:
    folded = ("/" + source).casefold()
    for marker, reason in SOURCE_EXCLUSION_RULES:
        if marker.casefold() in folded:
            return reason
    suffix = Path(source).suffix.casefold()
    if suffix in {".rc", ".ico", ".bmp", ".cur"}:
        return "Win32 resource compiled out of the native build"
    if suffix in {".asm", ".nvp", ".nvv"}:
        return "legacy assembly/shader input replaced by portable source"
    return None


def collect(root: Path) -> dict:
    lookup = file_lookup(root)
    records: list[dict[str, str | bool]] = []
    projects: list[dict[str, str | bool]] = []
    for manifest_path in tracked_manifests(root):
        manifest = manifest_path.relative_to(root).as_posix()
        target = TARGET_BY_MANIFEST.get(manifest)
        project_reason = None if target else project_exclusion(manifest)
        projects.append({
            "manifest": manifest,
            "state": "candidate" if target else "excluded",
            "target": target or "",
            "reason": project_reason or "portable runtime candidate",
        })
        text = manifest_path.read_text(encoding="latin-1")
        seen_sources: set[str] = set()
        for raw in SOURCE_RE.findall(text):
            source, exists = normalize_source(root, manifest_path, raw, lookup)
            source_key = source.casefold()
            if source_key in seen_sources:
                continue
            seen_sources.add(source_key)
            reason = None if target else project_reason
            if target and exists:
                reason = source_exclusion(source)
            if not exists:
                state = "unavailable"
                reason = "manifest input is absent from the repository or outside its source boundary"
            elif reason:
                state = "excluded"
            else:
                state = "candidate"
            records.append({
                "manifest": manifest,
                "source": source,
                "state": state,
                "target": target if state == "candidate" else "",
                "reason": reason or "portable runtime candidate",
            })

    solution = root / "GeneralsMD/Code/RTS.dsw"
    for name, raw_path in PROJECT_RE.findall(solution.read_text(encoding="latin-1")):
        source, exists = normalize_source(root, solution, raw_path, lookup)
        projects.append({
            "manifest": f"RTS.dsw::{name}",
            "state": "solution-present" if exists else "solution-unavailable",
            "target": "",
            "reason": source if exists else f"{source} is absent; project is unavailable/out of scope",
        })
    records.sort(key=lambda item: (str(item["target"]), str(item["state"]), str(item["manifest"]), str(item["source"])))
    projects.sort(key=lambda item: str(item["manifest"]))
    return {"projects": projects, "sources": records}


def cmake_quote(value: str) -> str:
    return value.replace("\\", "/").replace('"', '\\"')


def render_cmake(inventory: dict) -> str:
    grouped: dict[str, list[str]] = collections.defaultdict(list)
    excluded: list[str] = []
    unavailable: list[str] = []
    for item in inventory["sources"]:
        record = f'{item["manifest"]}|{item["source"]}|{item["reason"]}'
        if item["state"] == "candidate":
            grouped[str(item["target"])].append(str(item["source"]))
        elif item["state"] == "excluded":
            excluded.append(record)
        else:
            unavailable.append(record)
    lines = [
        "# Generated by tools/legacy_manifest_inventory.py; do not hand edit.",
        "# These are migration inputs. M0 does not compile legacy sources.",
        "",
    ]
    for target in sorted(set(TARGET_BY_MANIFEST.values())):
        lines.append(f"set(ZH_LEGACY_{target.upper()}_SOURCES")
        for source in sorted(set(grouped[target])):
            lines.append(f'  "{cmake_quote(source)}"')
        lines.extend([")", ""])
    for name, records in (("ZH_LEGACY_EXCLUDED_SOURCE_RECORDS", excluded), ("ZH_LEGACY_UNAVAILABLE_SOURCE_RECORDS", unavailable)):
        lines.append(f"set({name}")
        for record in records:
            lines.append(f'  "{cmake_quote(record)}"')
        lines.extend([")", ""])
    return "\n".join(lines)


def render_doc(inventory: dict) -> str:
    state_counts = collections.Counter(str(item["state"]) for item in inventory["sources"])
    target_counts = collections.Counter(str(item["target"]) for item in inventory["sources"] if item["state"] == "candidate")
    lines = [
        "# Legacy manifest inventory",
        "",
        "This M0 inventory classifies the VC6 Zero Hour manifests. Candidate entries are migration inputs assigned to a native target; they are not claimed to compile until their owning port milestones complete. `cmake/LegacySourceInventory.cmake` contains every explicit source record.",
        "",
        "## Source summary",
        "",
        "| Classification | Records |",
        "|---|---:|",
    ]
    for state in ("candidate", "excluded", "unavailable"):
        lines.append(f"| {state} | {state_counts[state]} |")
    lines.extend(["", "| Native target | Candidate records |", "|---|---:|"])
    for target in sorted(target_counts):
        lines.append(f"| `{target}` | {target_counts[target]} |")
    lines.extend(["", "## Project classification", "", "| Manifest/project | Classification | Native target or reason |", "|---|---|---|"])
    for item in inventory["projects"]:
        result = str(item["target"] or item["reason"]).replace("|", "\\|")
        lines.append(f'| `{item["manifest"]}` | {item["state"]} | {result} |')
    lines.extend([
        "",
        "## Audit",
        "",
        "Run `python3 tools/legacy_manifest_inventory.py --check`. Regenerate after an intentional manifest change with `python3 tools/legacy_manifest_inventory.py --generate`.",
        "",
    ])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--check", action="store_true")
    action.add_argument("--generate", action="store_true")
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--cmake-output", type=Path)
    parser.add_argument("--doc-output", type=Path)
    args = parser.parse_args()
    root = args.root.resolve()
    cmake_output = args.cmake_output or root / "cmake/LegacySourceInventory.cmake"
    doc_output = args.doc_output or root / "docs/legacy-manifest-inventory.md"
    inventory = collect(root)
    expected = {cmake_output: render_cmake(inventory), doc_output: render_doc(inventory)}
    if args.generate:
        for path, content in expected.items():
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
    else:
        stale = []
        for path, content in expected.items():
            if not path.is_file() or path.read_text(encoding="utf-8") != content:
                stale.append(str(path))
        if stale:
            print("legacy manifest inventory is missing or stale: " + ", ".join(stale), file=sys.stderr)
            return 1
    counts = collections.Counter(str(item["state"]) for item in inventory["sources"])
    print(f'legacy inventory: {counts["candidate"]} candidate, {counts["excluded"]} excluded, {counts["unavailable"]} unavailable')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
