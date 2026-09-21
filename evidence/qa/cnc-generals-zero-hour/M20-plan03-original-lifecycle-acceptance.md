# M20 plan 03 original lifecycle acceptance

## Authority and scope

- Milestone: `M20-original-lifecycle`.
- Reconciliation packet: `sha256:21ce030a34f700bbcf7773983454f0b741f384956f1ff6568d7fc6782bc5bfe9` (source `cb567c2`, planning payload `cd2389e`, closeout `75da052`).
- Transaction start: `33b36cf653bcc3c7e1cf5e0a638db98a3eae0c1d`.
- Plan 03 slice commits: slice 01 `e2d312a`; slice 02 `b90118a8130`; slice 03 is the commit containing this report.
- This report accepts only M20's bounded offline init/update/reset/teardown closure. M21-M25 retain scenario, hardware-scene, interactive-media, full save/replay-scenario, and lobby/transport acceptance.

## Outcome

The installed production Linux entry runs original `GameMain` and the integrated original engine/provider lifecycle from an arbitrary working directory. Owned fixtures and the separately provisioned read-only retail gate both reach init, post-load, a genuine original logic update, original reset, a second bounded update, and ordered teardown. No window, GPU, audio device, or online transport is acquired. Relevant original allocator counts, workers, physical-device counters, and reset-owned resources return to zero.

The bounded profile disables initial shell-map/movie dispatch only after normal configuration has been parsed. It does not alter normal defaults. Required W3D configuration uses the complete original 19-entry production registry and canonical concrete ModuleData; an unavailable draw-instance request still fails closed. The production BIG provider retains checked malformed/truncated/range/overflow, lookup, publication, and cleanup behavior while accepting valid original archive conventions.

## Current validation

| Context | Result | Evidence grade |
|---|---:|---|
| GCC Debug full asset-free CTest | 117/117 | component through original-source integration |
| Clang Debug full asset-free CTest | 117/117 | component through original-source integration |
| GCC Release full asset-free CTest | 117/117 | component through original-source integration |
| Clang Release full asset-free CTest | 117/117 | component through original-source integration |
| Clang Debug focused ASan+UBSan | 27/27 | original-source integration |
| Clean GCC Release install, owned arbitrary-CWD lifecycle | passed | installed original-source integration |
| Clean GCC Release install, read-only retail cold/warm lifecycle | passed | retail runtime |
| Identity, provider-removal, classification, ledger freshness | passed | original-source compile/link and integration |

All four full suites ran from current incremental builds of `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`. Release uses the repository's existing `-DNDEBUG` configuration. New tests use active runtime checks rather than the standard `assert` macro.

The focused sanitizer command used Clang Debug with ASan and UBSan enabled, GPU and retail tests disabled, and:

```text
ASAN_OPTIONS=detect_leaks=0:abort_on_error=1:strict_string_checks=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
```

LeakSanitizer was disabled, so sanitizer success is not presented as leak proof. The runtime independently requires zero original allocator live allocations, stopped workers, zero physical acquisitions, and release of reset-owned resources. Sanitizers additionally exposed and drove repairs for a recycled-pool ASan container annotation, an invalid enum sentinel conversion, and a fixture whose service order preceded its filesystem dependency.

## Acceptance mapping

- Complete W3D schema, inherited/default/override parsing, missing/unknown providers, deferred draw failure, teardown, and absence of Direct3D linkage: slice 01 focused runtime, identity, and provider-removal gates.
- BIGF/BIG4 lookup, ordering, RAM/streaming reads, malformed/truncated/table/name/range/overflow rejection, transactional publication, and repeated cleanup: slice 02 focused runtime, identity, provider-removal, and sanitizer gates.
- Original entry/init/post-load/update/reset/teardown, original state transitions, explicit no-match profile, arbitrary CWD, isolated XDG, source-defined offline network null, unsupported-network failure, all-stage failure unwind, and controlled-quit semantics: production-entry and headless/lifecycle suites in all four presets.
- Complete INI/module registry, original strings/pools in consumers, nonempty object/map-cache/localization data, cold/warm/cache/write-denial/benchmark/resource failures, and preserved initiating diagnostics: data-startup and production-entry positive/negative matrices plus focused sanitizers.
- No reduced provider or stale evidence substitution: target identity, deliberate provider-removal, source-classification, and dependency-ledger freshness gates.
- Read-only retail initialization and original configuration compatibility: private-safe cold/warm installed-form runtime gate. It reports only logical outcome and aggregate ownership counters; no retail host path, hash, or byte is retained.

## Ownership conclusion

M20 is acceptance-ready. The runtime evidence establishes the complete bounded offline lifecycle and `PRE-029` production boundary without claiming later scenario or device behavior. No known M20 blocker remains.
