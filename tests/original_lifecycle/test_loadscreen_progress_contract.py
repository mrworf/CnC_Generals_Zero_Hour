#!/usr/bin/env python3
"""Verify the Linux no-owner factory and redacted post-progress source order."""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


MARKER = "M22 original loadscreen progress: ok generations=2 modes=7 owners=0 windows=0 devices=0"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--owner", type=Path, required=True)
    args = parser.parse_args()
    if not args.source.is_file() or not args.owner.is_file():
        raise RuntimeError("missing source or generated no-owner witness")
    result = subprocess.run([str(args.owner)], text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode or MARKER not in result.stdout + result.stderr:
        raise RuntimeError("generated Linux no-owner lifecycle did not close two generations")

    source = args.source.read_text(encoding="utf-8")
    factory = re.search(
        r"LoadScreen \*GameLogic::getLoadScreen\( Bool loadingSaveGame \)\s*\{(.*?)#else",
        source,
        re.DOTALL,
    )
    if not factory or not re.search(r"#ifndef _WIN32\s*\(void\)loadingSaveGame;\s*return NULL;", factory.group(1)):
        raise RuntimeError("Linux factory no-owner guard changed or was removed")
    if re.search(r"\bNEW\b|new\s+", factory.group(1)):
        raise RuntimeError("Linux factory branch created an owner")

    dispatch = re.search(
        r"void GameLogic::updateLoadProgress\( Int progress \)\s*\{\s*if\( m_loadScreen \)\s*m_loadScreen->update\( progress \);",
        source,
        re.DOTALL,
    )
    if not dispatch:
        raise RuntimeError("optional source progress dispatch changed")
    post_progress = source.find("if(m_loadScreen)\n\t\tupdateLoadProgress(LOAD_PROGRESS_POST_PARTICLE_INI_LOAD);")
    map_ini = source.find("loadMapINI(", post_progress)
    if post_progress < 0 or map_ini < 0 or post_progress >= map_ini:
        raise RuntimeError("post-progress advance is no longer before map-INI loading")
    print("original generated loadscreen progress contract: generations=2 owners=0 advancement=redacted")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        raise SystemExit(1)
