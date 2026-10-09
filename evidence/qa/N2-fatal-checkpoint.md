# N2 native fatal-error checkpoint — not milestone acceptance

Parent: `717a474b`. Same governing N2 plan01/slice03; both remain in progress.
This supplies the actual original ReleaseCrash and ReleaseCrashLocalized paths,
not an assertion suppression or a returning stub. No agents/vendor/retail/recovery
changes. Original native graph still excludes the alternative Win32 Debug.cpp.

Canonical cumulative source561 SHA256
`8638d4c6722e00c1979964ffff6704522d5e1acea9b4da19b88b5869effc481a`;
toolchain-inclusive566 SHA256
`cef73d71e18665ba890676775d229e72166b8e003c1a318d08ed08f131917004`.
Hash convention is the existing path/content cumulative identity since9e5c0e4f,
excluding documentation/workflow metadata.

All configured targets build under normal GCC, GCC ASan/UBSan/LSan and Clang
ASan/UBSan/LSan. Related74/74 PASS on all three normal-host configurations:
16.78s,67.13s,63.74s respectively. Leak checks/halt-on-error intact, no suppressions.
Two new fatal families execute27 generated child-process cases across three
repetitions: both entry points, null/raw private-looking input, exhausted allocation,
closed/broken stderr, exact fixed redacted report, no atexit callbacks and no
raw output files. Parent-owned caller strings and pipe descriptors retire normally.
The fatal child intentionally uses source-equivalent immediate exit, not normal
destruction/LSan shutdown; these tests do not accept original process teardown.
Normal focused2/2 additionally PASS0.02s before the frozen related matrix.

```sh
cmake --build build/original-core-<variant> -j4 -- -k0
ctest --test-dir build/original-core-<variant> --output-on-failure -R 'original_(fatal|core|statistics|storage|configuration|message|runtime|preferences|namespace)'
```

Variants: gcc,gcc-sanitize,clang-sanitize. Logs:
`/tmp/zh-fatal-<variant>-{build,frozen-build,related-tests,root-link}.log`.
A shared-header timestamp change (subsequently restored byte-for-byte) caused
the initial broader rebuild. Both that build and the incremental final-source
build passed all3; no failed compile/test or relaxed coverage in this checkpoint.

Link-only original_runtime_link_probe remains a diagnostic failure, never executed:
normal GCC6, GCC sanitizer218, Clang sanitizer222 unique unresolved symbols.
Normal remaining: HideDiplomacy, PopulateInGameDiplomacyPopup, ResetDiplomacy,
positionStartSpots, testMinimumRequirements, updateMapStartSpots. Sanitizer roots
remain wider. Current configured suite209 has not been run as a full matrix.
Whole startup/default statistics sampler/GameLogic/scenario acceptance is pending.

Source findings needed for the next batch are recorded in
`docs/original-engine-formats.md`: missing original benchmark implementation,
LOD discovery order and raw optional report, GUI static pointer retirement,
map-count/extents/sparse-target admission and projection-coordinate consistency.
Do not synthesize benchmark scores or install no-op callbacks to clear these roots.
