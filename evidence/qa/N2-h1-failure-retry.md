# N2 plan02 slice05 — H1 failure, retry and ownership

Source base: `6548b4af` plus this slice. Renderer-off Debug builds with only
generated inputs and separate user storage. No retail paths or data were used.
The tests retain every rejection and corrected H1 retry in one process.

The fixture rejects a missing required GameData file, malformed GameData,
missing data root, truncated map, nonfinite actor coordinate, unknown module
and unsupported road provider. Each rejection reports non-success, retires
the native root, checks zero used units in every typed actor/map/script/
deletion pool that was acquired, restores the same generated fixture and
then admits corrected H1. A deliberately interrupted loaded world also
retires before corrected H1 completes. Complete successful H1 retirement
requires all four typed pools to exist with zero used units.

The cold normal GCC pending-map census observed 11,074 standard-allocation
attempts. For the integration fault, the fixture first warms an identical
complete root, then measures two successful admissions on the same
configuration value and script-ID start state. It proves source script cursor
advancement before an outer fixture lease restores that start state. The two
warmed counts match; current normal GCC measured terminal is 10,937. A fault
at ordinal 10,936 rejects late during original CRC generation, with exactly
10,937 attempts and the trigger observed. Its root and standard-allocation
baseline retire exactly; corrected H1 then succeeds. Arming ordinal 10,937
succeeds without triggering and reports exactly 10,937 attempts. Counts are
derived per run/configuration, never used as a portable constant.

GCC LeakSanitizer identified two leaked allocations (57 bytes) after the
injected CRC failure in the previous source state: the raw `XferCRC` owner
and its string. `GameLogic::getCRC` now keeps that transfer in a local owning
guard across `open`, snapshots and `close`. The final late-failure test passes
GCC and Clang ASan/UBSan/LSan with exact `AllocationFault::live` baseline.
The exhaustive full-root sweep of more than ten thousand parser allocations
is not used as a proxy for owner-specific coverage. Current component tests
separately exhaust actual map/script and actor typed pools, retain their
failure/retry pairs and pass all three configurations. The pre-existing dirty
actor test source was run for current-state validation but was not staged or
accepted into this slice commit.

```sh
cmake --build build/original-core-gcc --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc -R '^original_native_headless_root_h1-' --output-on-failure
cmake --build build/original-core-gcc-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-gcc-sanitize -R '^original_native_headless_root_' --output-on-failure
cmake --build build/original-core-clang-sanitize --target original_native_headless_root_fixture -j4
ctest --test-dir build/original-core-clang-sanitize -R '^original_native_headless_root_' --output-on-failure
ctest --test-dir build/original-core-gcc -R '^(original_logical_world_pool-failures|original_object_ownership_pool-failures)$' --output-on-failure
```

Normal GCC: eight H1 cases passed after the final pool assertion. GCC and
Clang sanitizers: 12/12 native-root cases passed before the final pool
assertion and the five changed failure-path cases passed again afterward.
All 34 related normal original-world/transfer regressions passed. Map/script
and actor typed-pool exhaustion tests each passed normal GCC, GCC sanitizers
and Clang sanitizers. The frozen N2 matrix and supplied-data startup remain
pending.
