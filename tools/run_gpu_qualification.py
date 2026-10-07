#!/usr/bin/env python3
"""Run generated GPU qualification with validation, preserving complete diagnostics."""
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

if len(sys.argv) not in (5,6) or (len(sys.argv)==6 and sys.argv[5]!='--isolated-bus'):
    raise SystemExit('usage: run_gpu_qualification.py BINARY SHADERS LOG FAMILY [--isolated-bus]')
env = dict(os.environ)
env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_KHRONOS_validation'
env['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
env['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
env['LSAN_OPTIONS'] = 'exitcode=23'
with tempfile.TemporaryDirectory(prefix='zh-qualification-bus-') as isolated:
    if len(sys.argv)==6:
        # Empty private directory: no daemon, no host service change. This is an
        # explicit fixture variant, never a default runtime workaround.
        env['DBUS_SYSTEM_BUS_ADDRESS'] = f'unix:path={isolated}/system-bus'
        env['DBUS_SESSION_BUS_ADDRESS'] = f'unix:path={isolated}/session-bus'
    result = subprocess.run([sys.argv[1], sys.argv[2], sys.argv[4]], env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
Path(sys.argv[3]).write_text(result.stdout)
failures = re.findall(r'^.*(?:Validation Error|VUID-|FAIL:|Assertion failed|ERROR: .*Sanitizer|runtime error:).*$|^.*BGFX.*ASSERT.*$', result.stdout, re.M)
for line in result.stdout.splitlines():
    if line.startswith(('PASS:', 'FAIL:', 'GPU:')):
        print(line)
if result.returncode or failures:
    print(f'qualification failed: exit={result.returncode}, diagnostic_errors={len(failures)}; see {sys.argv[3]}')
    for line in failures[:10]:
        print(line)
    raise SystemExit(1)
if 'PASS: qualification complete' not in result.stdout:
    raise SystemExit('qualification did not produce its terminal completion marker')
if not re.search(r'Enabled instance layers:\n(?:(?!Enabled instance extensions:).)*VK_LAYER_KHRONOS_validation', result.stdout, re.S):
    raise SystemExit('active validation layer was not reported by the debug runtime')
print('PASS: no Vulkan validation error diagnostics')
