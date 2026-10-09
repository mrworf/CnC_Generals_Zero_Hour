# N2 native LOD checkpoint — not milestone acceptance

Parent `6640b86820594027ecf95e40fa81875737a1b840`; existing plan01/slice03.
The worktree inventory at transaction start contained six pending files: actual
GameLOD header/source, native probe, original LOD fixture, CMake registration and
the existing slice plan. These were one unfinished coupled implementation batch,
not unrelated user changes. This checkpoint includes that batch and its directly
required durable findings/state. No agents, vendor/framework changes, retail
access or recovery imports. Build/cache/recovery/retail content remains ignored;
nothing is deleted to make Git status clean.

## Frozen validation

All configured targets build in normal GCC, GCC ASan/UBSan/LSan and Clang
ASan/UBSan/LSan. Focused six LOD families pass in all three configurations:
5.46s, 22.32s and 14.06s respectively. Related 106/106 tests pass in all three:
15.06s, 62.20s and 50.37s. Sanitizer executions use the normal host with all
instrumentation and leak checks intact. The configured full suite has 238 tests;
it was not run, and this is not N2 acceptance.

```sh
cmake --build build/original-core-<variant> -j4
ctest --test-dir build/original-core-<variant> -R '^original_lod_' --output-on-failure -j2
ctest --test-dir build/original-core-<variant> -R 'original_(lod|native_warning|header_paths|diplomacy|map|game_info|lexicon|function_registry|startup|service|configuration|runtime|core)' --output-on-failure -j2
```

Variants: `gcc`, `gcc-sanitize`, `clang-sanitize`. Temporary logs:
`/tmp/zh-lod-review-*build.log`, `/tmp/zh-lod-final-*-build.log`, and
`/tmp/zh-lod-related-{gcc,gccsan,clangsan}.log`. Earlier reviewed-build logs retain
the missing FileSystem declaration diagnostic. Initial focused checks found an
ignored unknown preset, a fixture texture oracle inconsistent with original cache
comparison, and storage faults occurring during required preference reads.
Those were addressed together without changing original texture behavior or
weakening coverage. I/O discovery then exposed a cold-versus-established directory
manifest mismatch (5 versus 6 operations); discovery now uses the same protected
namespace as every failure/retry pair and proves its exact terminal. Failure
paths preserve old metadata/publication/report; optional report errors distinguish
prepublication failure from a complete post-rename result. No timeout was raised.

Canonical 593 cumulative path/content SHA256 since `9e5c0e4f`:
`cb7760b9ae1633504d794b4e03aaec73cfeae4e782647c938751717880a2e9bc`.
Toolchain-inclusive 600:
`574621f88ae31a37ded3ee0577bfbb50da24d9ac7ea5c042aefe523bd3f3c9c2`.
Sorted path bytes, NUL, binary SHA256 of each file's bytes. Specification unchanged:
`0316702c3fb75979f3ef5cdafb3adcf3a4c1bfa3dc84e94f883aceb6ce49bf90`.

## Executed behavior and limits

Actual original GameLOD parses generated source-format tables and preferences,
admits wide physical RAM, preserves unknown classification without false ideal
settings, applies original source quality/dynamic transitions, rejects malformed
tables and scores, and retires complete owners three times. Synthetic established
legacy profiles exercise original comparison rules and protected recommendation
persistence; they do not prove modern calibration or native renderer capability.
Native POSIX hardware discovery checks positive RAM and consistent optional
frequency with exact descriptor/heap retirement, without printing private values.
Every discovered initialization/report allocation prefix and report I/O fault has
a corrected retry and exact success terminal. Whole memory-manager destruction
retires warmed registry capacity exactly. Legacy forced calibration is explicitly
unavailable by default, rather than fabricated from a modern clock/vendor.

Remaining: recommendation-specific allocation/storage failure sweeps, nonnull
presentation callback rollback, actual native factories and GameEngine startup,
compiler-matching original simulation/world scenario, pending actual animation
ownership execution, full N2 matrix and native quality/device evidence. Root and
animation diagnostics were not rerun or executed in this cleanup checkpoint.
N0/N1 remain accepted; N2/slice03 and N3–N7 remain incomplete. Verified format and
ownership facts are in `docs/original-engine-formats.md` and source integration
limits in `docs/original-runtime-source-graph.md`.
