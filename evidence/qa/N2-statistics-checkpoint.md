# N2 original statistics checkpoint — not milestone acceptance

Parent commit: `8cab2148cc2a1ab0cb508c463042a6834ed9c624`.
This commit closes the pending statistics batch within existing N2 plan01/slice03;
N2 and slice03 remain in progress. No agents, vendor, retail or recovery edits.

## Frozen source and validation

Canonical cumulative source: 559 paths, SHA256
`bcc50fb3f0c0cc746c400170b9cfeb85b9efdcf47d81cadcc941fb050eafac0e`.
Toolchain-inclusive: 564 paths, SHA256
`abf08c7c215ca0b6c3360776b6d62dd10a84d18c013db4fc05b63a285f1e7d5c`.
Identity follows the existing cumulative path/content hash convention since
`9e5c0e4f`; documentation and workflow metadata are excluded.

All configured targets built for GCC, GCC ASan/UBSan/LSan, and Clang
ASan/UBSan/LSan. Related 67/67 tests PASS on each normal-host configuration:
GCC 16.63s, GCC sanitizer 66.19s, Clang sanitizer 63.04s. Leak detection and
halt-on-error remain enabled; no suppression or test weakening.

Commands per variant `gcc`, `gcc-sanitize`, `clang-sanitize`:

```sh
cmake --build build/original-core-<variant> -j4 -- -k0
ctest --test-dir build/original-core-<variant> --output-on-failure -R 'original_(statistics|storage|configuration|message|runtime|preferences|namespace)'
```

Temporary logs: `/tmp/zh-statistics-<variant>-reviewed-{build,tests}.log`.
Six statistics families cover actual collector behavior, malformed inputs, all
storage-phase failures with retry, and separate reset/update/end allocation
sweeps. Each executes three complete ownership lifetimes. Exact failure/retry
manifests and terminals are reset22, update32, end37 on all three configurations;
each failure retains accepted backing, retires candidate allocations/descriptors/
temporary files, and retries on the same owner. Durability-unknown publication
advances state without duplicate rows/footer. Actual pooled GameMessages exercise
source command filtering. Failed new-session reset blocks appending to old output.

Earlier incomplete attempts remain non-acceptance: duplicate constructor
declaration and fixture AsciiString/std::string assignment caused build failures;
an initial test attempt had no built fixture. Both mechanical defects were
corrected before this frozen matrix. The later coupled review added session-reset
admission and absolute-root directory rejection before the recorded runs.

## Boundaries and remaining work

The actual default constructor and source GameLogic/player/object adapter compile;
tests provide explicit generated synchronous inputs to the actual collector.
These tests do not accept whole GameLogic sampling, debug/LAN naming, startup,
rendering, or scenario gameplay. All 207 configured tests were not run as a full
matrix; earlier full198 evidence remains historical.

The link-only `original_runtime_link_probe` still fails, as expected, and is never
executed. Unique unresolved symbols: normal GCC7, GCC sanitizer219, Clang
sanitizer223. Normal remaining symbols are HideDiplomacy, PopulateInGameDiplomacyPopup,
ReleaseCrash, ResetDiplomacy, positionStartSpots, testMinimumRequirements,
updateMapStartSpots. Sanitizer-retained callbacks remain wider. Logs:
`/tmp/zh-statistics-<variant>-root-link.log`. These are diagnostics, not a passed
process acceptance gate. Continue with native capability/diplomacy/start-spot
providers, sanitizer roots, and the original CPU world/scenario gates.
