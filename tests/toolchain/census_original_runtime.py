"""Read-only compilation census, not startup or milestone acceptance.

Probe the whole original logic owner graph before choosing coupled portability
batches. No proprietary inputs or copied provider implementations are involved.
"""
from pathlib import Path
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
import re
import subprocess

REPO = Path(__file__).resolve().parents[2]
CODE = REPO / "GeneralsMD/Code"
ROOTS = ("GameEngine/Include", "GameEngine/Include/Precompiled", "Libraries/Include",
         "Libraries/Source/WWVegas", "Libraries/Source/WWVegas/WWLib",
         "Libraries/Source/GameSpy", "Libraries/Source/Compression")
ERROR = re.compile(r'^(.+?):(\d+)(?::\d+)?:\s*(?:fatal )?error:\s*(.*)$')

def probe(source, compiler):
    command = [compiler, "-std=c++20", "-D_OPERATOR_NEW_DEFINED_", "-fsyntax-only",
               "-fmax-errors=0", *("-I" + str(CODE / root) for root in ROOTS), str(source)]
    completed = subprocess.run(command, capture_output=True, text=True, errors="replace", timeout=60)
    errors = []
    for line in completed.stderr.splitlines():
        match = ERROR.match(line)
        if match:
            path = Path(match[1])
            if path.is_absolute():
                try: path = path.relative_to(REPO)
                except ValueError: path = Path("system-header") / path.name
            errors.append({"source": path.as_posix(), "line": int(match[2]), "message": match[3]})
    return {"translation_unit": source.relative_to(REPO).as_posix(),
            "sha256": hashlib.sha256(source.read_bytes()).hexdigest(),
            "returncode": completed.returncode, "errors": errors}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", default="g++")
    parser.add_argument("--jobs", type=int, default=4, choices=range(1,9))
    parser.add_argument("--scope", choices=("logic","common","client"), default="logic")
    args = parser.parse_args()
    directory = {"logic":"GameLogic", "common":"Common", "client":"GameClient"}[args.scope]
    sources = sorted((CODE / "GameEngine/Source" / directory).rglob("*.cpp"))
    results = []
    with ThreadPoolExecutor(max_workers=args.jobs) as executor:
        futures = [executor.submit(probe, source, args.compiler) for source in sources]
        for future in as_completed(futures):
            results.append(future.result())
            if len(results) % 32 == 0:
                print(f"source-only {args.scope} census {len(results)}/{len(sources)}", flush=True, file=__import__('sys').stderr)
    print(json.dumps({"kind": "unaccepted-source-compilation-census", "scope":args.scope, "compiler": args.compiler,
                      "translation_units": sorted(results, key=lambda item:item["translation_unit"])},indent=2))

if __name__ == "__main__": main()
