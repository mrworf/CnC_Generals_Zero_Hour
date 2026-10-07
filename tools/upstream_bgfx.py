#!/usr/bin/env python3
"""Acquire/verify/build official stock sources; never patches or resets a checkout."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
LOCK = ROOT / 'third_party/bgfx.lock.json'


def output(*args):
    return subprocess.check_output(args, text=True).strip()


def verify(project, path, spec):
    if path.is_symlink() or not (path / '.git').is_dir():
        raise ValueError(f'{project}: expected a standalone upstream Git checkout')
    revision = output('git', '-C', str(path), 'rev-parse', 'HEAD')
    if revision != spec['revision']:
        raise ValueError(f'{project}: revision differs from lock; acquire into a fresh root')
    origin = output('git', '-C', str(path), 'remote', 'get-url', 'origin')
    if origin != spec['url']:
        raise ValueError(f'{project}: origin is not the official locked upstream')
    dirty = output('git', '-C', str(path), 'status', '--porcelain', '--untracked-files=all')
    if dirty:
        raise ValueError(f'{project}: source is modified or contains untracked source files')
    if not (path / 'LICENSE').is_file():
        raise ValueError(f'{project}: upstream license is missing')
    return revision


def read_lock():
    lock = json.loads(LOCK.read_text())
    if lock['schema_version'] != 1 or set(lock['projects']) != {'bgfx', 'bx', 'bimg'}:
        raise ValueError('invalid upstream lock schema')
    for name, spec in lock['projects'].items():
        if spec['url'] != f'https://github.com/bkaradzic/{name}.git':
            raise ValueError('only the official project origins are permitted')
        if not re.fullmatch('[0-9a-f]{40}', spec['revision']):
            raise ValueError('dependency revision must be a full commit ID')
    return lock['projects']


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['acquire', 'verify', 'build'])
    parser.add_argument('--source-root', type=Path, default=ROOT / 'build/upstream')
    parser.add_argument('--compiler', choices=['gcc', 'clang'], default='gcc')
    parser.add_argument('--configuration', choices=['debug64', 'release64'], default='debug64')
    parser.add_argument('--jobs', type=int, default=4)
    args = parser.parse_args()
    if args.jobs < 1 or args.jobs > 64:
        parser.error('--jobs must be between 1 and 64')
    specs = read_lock()
    root = Path(os.path.abspath(args.source_root))
    # Acquisition/build may only use an ignored build tree, never a source or archive tree.
    build_root = ROOT / 'build'
    if not root.is_relative_to(build_root) or any(p.is_symlink() for p in [root, *root.parents] if p != ROOT and p.is_relative_to(ROOT)):
        raise ValueError('source root must be a real directory under this checkout\'s build/')
    if args.action == 'acquire':
        root.mkdir(parents=True, exist_ok=True)
        for name, spec in specs.items():
            path = root / name
            if not path.exists():
                path.mkdir()
                subprocess.run(['git', 'init', '-q', str(path)], check=True)
                subprocess.run(['git', '-C', str(path), 'remote', 'add', 'origin', spec['url']], check=True)
                subprocess.run(['git', '-C', str(path), 'fetch', '--depth', '1', 'origin', spec['revision']], check=True)
                subprocess.run(['git', '-C', str(path), 'checkout', '--detach', 'FETCH_HEAD'], check=True)
            verify(name, path, spec)
    for name, spec in specs.items():
        print(f'{name}: verified official pristine {verify(name, root / name, spec)}', flush=True)
    if args.action == 'build':
        bgfx = root / 'bgfx'
        project = f'.build/projects/gmake-linux-{args.compiler}'
        subprocess.run(['make', '-C', str(bgfx), project], check=True)
        subprocess.run(['make', '-R', '-C', str(bgfx / project),
                        f'config={args.configuration}', f'-j{args.jobs}',
                        'bgfx-shared-lib', 'shaderc'], check=True)
        for name, spec in specs.items():
            verify(name, root / name, spec)
        print('Stock runtime/shaderc built; all three source trees remain pristine.', flush=True)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (ValueError, OSError, subprocess.CalledProcessError) as exc:
        print(f'upstream-bgfx: {exc}', file=sys.stderr)
        sys.exit(1)
