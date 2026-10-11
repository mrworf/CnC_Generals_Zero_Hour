#!/usr/bin/env python3
"""Redacted read-only audit of the actual original native startup entry."""

import argparse
from pathlib import Path
import subprocess
import tempfile

from run_data_audit import snapshot


def roots_from_options(options):
    if options.workspace_link:
        links = [entry for entry in Path.cwd().iterdir()
                 if entry.is_symlink() and entry.is_dir()]
        if len(links) != 1 or options.data_root:
            raise ValueError("ambiguous supplied roots")
        return [str(links[0])]
    if not options.data_root:
        raise ValueError("missing supplied roots")
    return options.data_root


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--data-root", action="append",
                        help="ordered supplied roots, Zero Hour before Generals")
    parser.add_argument("--workspace-link", action="store_true",
                        help="use the sole top-level directory symlink without printing it")
    options = parser.parse_args()
    try:
        roots = roots_from_options(options)
        before = snapshot(roots)
        with tempfile.TemporaryDirectory(prefix="zh-native-audit-") as temporary:
            work = Path(temporary)
            def startup(data_roots, user, cache):
                command = [options.binary, "--startup"]
                for root in data_roots:
                    command.extend(("--data-root", str(root)))
                command.extend(("--user-data", str(user), "--user-cache", str(cache)))
                try:
                    return subprocess.run(command, capture_output=True, timeout=180)
                except (OSError, subprocess.TimeoutExpired):
                    return None
            def rejected(result):
                return (result is not None and result.returncode == 1 and
                        result.stdout == b"" and
                        result.stderr == b"original startup rejected supplied configuration or data\n")
            missing = startup([work / "missing-root"], work / "missing-user", work / "missing-cache")
            blocked_user = work / "blocked-user"
            blocked_user.write_bytes(b"not a directory")
            storage = startup(roots, blocked_user / "child", work / "storage-cache")
            blocked_cache = work / "blocked-cache"
            blocked_cache.mkdir(mode=0o500)
            cache_fallback = startup(roots, work / "fallback-user", blocked_cache)
            user = work / "user"
            cache = work / "cache"
            result = startup(roots, user, cache)
        unchanged = before == snapshot(roots)
        success = (result is not None and result.returncode == 0 and
                   result.stdout == b"original startup and shutdown complete\n" and
                   not result.stderr)
        fallback_ok = (cache_fallback is not None and cache_fallback.returncode == 0 and
                       cache_fallback.stdout == b"original startup and shutdown complete\n" and
                       not cache_fallback.stderr)
        if success:
            print("NATIVE_STARTUP_AUDIT stage=post-init status=PASS")
        else:
            print("NATIVE_STARTUP_AUDIT stage=startup status=REJECTED")
        print("MISSING_INPUT=" + ("REJECTED" if rejected(missing) else "FAILED"))
        print("STORAGE_FAILURE=" + ("REJECTED" if rejected(storage) else "FAILED"))
        print("CACHE_FALLBACK=" + ("PASS" if fallback_ok else "FAILED"))
        print("INPUT_INTEGRITY=" + ("UNCHANGED" if unchanged else "CHANGED"))
        return 0 if success and rejected(missing) and rejected(storage) and fallback_ok and unchanged else 1
    except Exception:
        print("NATIVE_STARTUP_AUDIT stage=wrapper status=REJECTED")
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
