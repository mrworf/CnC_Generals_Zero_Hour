# N2 rooted original-data slice acceptance

Date: 2026-10-07. N2 remains in progress; this accepts slice02, not simulation.
Parent: `e7abf390d1b24d8b44c9bf8c0db1ff419f60b98c`.
Commit boundary: `delivery: port rooted original data consumers to Linux`.
Governing plan: `delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01.md`.

## Frozen source and environment

51 changed/added production/build/test files: paths under `GeneralsMD/Code/`,
`tests/original/`, plus `cmake/OriginalCore.cmake` and `tools/run_data_audit.py`.
Sorted path + NUL + binary SHA256(content) records, hashed together:
`bff9a5477827ddcd7440465e7ed58ec9875de7c2b6fe612099814d485721ee9b`.
The commit diff against the parent defines the exact source set. Documentation
and delivery status are excluded from this executable-source fingerprint.

Arch x86-64; GCC16.2.1 and Clang22.1.8; CMake/Ninja Debug C++20. The native
targets link actual original values, pools, strings, File/RAMFile, INI field
dispatch, GameText and LanguageFilter owners. No Windows SDK or replacement
simulation is linked. `INI.cpp`'s full original gameplay block table remains
uncompiled pending the actual startup graph in slice03.

## Executed acceptance

| Configuration | Result | Time |
| --- | --- | --- |
| normal `build/original-core-gcc` | 18/18 | .80s |
| GCC ASan/UBSan `build/original-core-gcc-sanitize` | 18/18 | 4.36s |
| Clang ASan/UBSan `build/original-core-clang-sanitize` | 18/18 | 3.11s |

Commands: configure with `-DZH_BUILD_RENDERER=OFF`, sanitizer variants with
`-DZH_SANITIZE=ON` and Clang with `-DCMAKE_CXX_COMPILER=clang++`; then
`cmake --build <directory> -j4` and `ctest --test-dir <directory> --output-on-failure`.
Sanitizer tests executed on the host because sandbox ptrace prevents LeakSanitizer.
All address, undefined-behavior and leak checks remain enabled; no suppressions,
isolated bus or GPU needed for these CPU cases. CTest LastTest.log retains the
generated-only evidence. Clang reports legacy missing-override declarations in
RAMFile and a valid fixture pointer-arithmetic spelling; neither is suppressed.

The six allocation batches prove exact contiguous ordinal coverage and terminal
success in all configurations, each repeated three times in its process:

| Batch | Rejected ordinals | First success |
| --- | --- | --- |
| mount, including nested physical directories | [0,21) | 21 |
| RAM snapshot | [0,3) | 3 |
| CSF candidate | [0,10) | 10 |
| map catalog, lookup and language-filter callbacks | [0,59) | 59 |
| language-filter map | [0,8) | 8 |
| actual INI field dispatch | [0,8) | 8 |

Every failure checks immediate standard-allocation counts, descriptor counts and
both native/RAM File pool units, preserves accepted state, and retries on the
same owner. Baselines follow public initialization/pool warming. The complete
fixture-only standard allocator covers ordinary, array, nothrow, aligned and
sized deallocation paths; production uses no global interposition.

Positive/negative families cover ordered roots/loose/archive precedence, archive
truncated prefixes and encoded ranges, independent shared views after remount,
empty archive sentinels, original numeric scanning, thrown borrowed reads,
inverted UTF16 CSF alternatives/counts/surrogates, source-string/map grammar,
lookup/reset/filter publication, original INI dispatch/numeric-prefix conventions,
malformed candidates, corrected retries and diagnostic copy/move ownership.
The generated wrapper test proves incomplete-content rejection, successful
container/text admission, mutating fake-runner detection, untrusted-output
redaction and integrity checking even when execution fails.

The combined renderer-enabled GCC configuration built successfully and registers
36 cases (18 existing renderer/provenance plus 18 core/data). Previously accepted
GPU gates were not repeated without a renderer change. The complete applicable
matrix is due at actual N2 completion in slice03.

## Supplied-data audit (redacted)

Initial: `READ_ONLY_DATA_AUDIT stage=1 mask=0 status=REJECTED`;
`INPUT_INTEGRITY=UNCHANGED`.
Source/range-family review established empty BIG members with offset zero.
Generated boundary tests were added before the corrected audit.
Corrected: `READ_ONLY_DATA_AUDIT stage=4 mask=63 status=PASS`;
`INPUT_INTEGRITY=UNCHANGED`.

Mask bits are public probe outcomes: mount admission1, GameData presence2,
Object family4, W3D family8, Maps family16 and populated actual GameText32.
The wrapper snapshots all regular input files with SHA256 plus size/mtime and
explicit-root identity before/after execution. No raw private output, input
hashes, selectors, names or roots are retained. Reads never change supplied data.
This partially resolves PRE04: usable data indexing/basic families/text is
directly established; actual startup and representative scenario completeness
remain required in slice03 and later consumers.

## Boundaries and next work

Verified source findings are durable in `docs/original-engine-formats.md`.
The obsolete startup archive deletion is removed, and that source-established
duplicate is excluded from archive selection without modifying its bytes.
bgfx/bx/bimg verification
still proves all three official source trees pristine; no dependency was edited.
The product plan hash remains
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.

No decoder cache or writable path is introduced. Actual GameEngine/GameLogic,
full gameplay INI dispatch, XDG writers/cache fallback, MemoryPools.ini overrides,
native timestamp/map-cache integration and non-BMP filter-width compatibility
remain explicit slice03 work. This evidence does not accept full gameplay,
rendering, media, persistence, LAN or clean-distribution release.
