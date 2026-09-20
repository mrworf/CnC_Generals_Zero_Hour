# M26 original process foundation evidence

Date: 2026-09-20
Scope: M26 only; asset-free x86-64 Linux process foundation

## Result

M26 acceptance is satisfied. Actual original allocator, pool bootstrap, synchronization, ASCII/UTF-16 strings, Version, and extracted FPU providers compile, link, and execute in GCC/Clang Debug/Release. No retail content, GPU, display, audio device, or full engine initialization was used.

## Provider and behavior evidence

- `zh_original_process` compiles the authoritative original `GameMemory.cpp`, `MemoryInit.cpp`, `CriticalSection.cpp`, `AsciiString.cpp`, `UnicodeString.cpp`, `version.cpp`, and extracted `FPUControl.cpp` source paths.
- Target-specific checks bind each source's absolute compile-command entry to its link-map object contribution, live demangled executable symbols, and a provider-naming runtime witness. A real consumer without `zh_original_process` fails to link.
- The checked ledger records source fingerprint, symbol, consumer, lifecycle/configuration, provider target, callback/vtable/global edges, assets, writes, owner, evidence grade, and tests. Scratch source drift and ownerless-edge mutations fail closed.
- Classification changes promote only the six legacy-manifest M26 providers. `FPUControl.cpp` is a new extraction outside that legacy inventory and is governed directly by the checked ledger.
- Exactly one production `setFPMode` definition remains. Its test changes the ambient mode, executes that definition, then observes nearest rounding and x87 24-bit precision.

Runtime witnesses from `linux-gcc-debug`:

```text
original-process allocator: ok provider=GameMemory.cpp config=defaults raw=23
original-process strings: ok providers=AsciiString.cpp,UnicodeString.cpp,version.cpp utf16=2 services=0 workers=0
original-process fpu: ok provider=FPUControl.cpp rounding=nearest precision=x87-24
```

The allocator test proves pre-main and cross-target allocation/free, standard and over-aligned paths, sized deletes, the real linkage counter, and expected static-lifetime retention. Pool configuration tests cover absent, valid, unknown compatible entry, malformed, duplicate, oversized file/line, invalid count, arbitrary CWD, explicit/executable-relative precedence, pre-main, re-entry, and late-retune rejection. String/service tests cover two-byte UTF-16, a surrogate pair, copy-on-write, formatting and bounds, each injected initialization failure, idempotent shutdown, zero live services, and zero workers.

## Validation

Focused `original-process` label after stabilization:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 10/10 passed |
| `linux-clang-debug` | 10/10 passed |
| `linux-gcc-release` | 10/10 passed |
| `linux-clang-release` | 10/10 passed |

Canonical full asset-free CTest, run once at milestone end with loopback-socket permission required by the existing LAN tests:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 72/72 passed |
| `linux-clang-debug` | 72/72 passed |
| `linux-gcc-release` | 72/72 passed |
| `linux-clang-release` | 72/72 passed |

Focused sanitizer build used Clang Debug with `ZH_ENABLE_ASAN=ON`, `ZH_ENABLE_UBSAN=ON`, `ZH_ENABLE_GPU_TESTS=OFF`, and `ZH_ENABLE_RETAIL_TESTS=OFF`. Execution used:

```text
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
ctest --test-dir /tmp/zh-m26-sanitizers -L original-process --output-on-failure
```

Result: 10/10 passed. The first sanitizer run found that the inherited 24-byte pool header misaligned a `CriticalSection` requiring 16-byte alignment. Linux pool headers/strides were corrected to `alignof(std::max_align_t)`, the ledger fingerprint was refreshed, and the complete focused sanitizer suite passed. Because leak detection was disabled, this is not a leak-freedom claim; explicit runtime ownership assertions instead establish raw-allocation deltas return to their expected static baseline and service/worker counts return to zero.

An unprivileged full-suite attempt could not create loopback UDP sockets in two pre-existing LAN tests. The identical canonical suites passed after granting the required loopback permission; this was an execution-sandbox constraint, not an M26 or product failure.

## Acceptance mapping

- Original consumers in four presets: focused results and target identity checks.
- Pre-main/cross-target/overload/alignment/UTF-16/static lifetime: allocator and string runtime tests plus sanitizer run.
- Bounded pre-VFS `MemoryPools.ini`: pool configuration matrix; no VFS/INI/engine string/log dependency.
- Failure cleanup and diagnostics: injected service-stage failures, idempotent teardown, counts at baseline/zero.
- Provider removal and non-proxy identity: real link-failure negative control and compile/link/symbol/runtime checks.
- Ledger ownership and drift: checked ledger plus scratch drift and ownerless controls; deferred edges name M27, M28, or M20.
- Existing regression safety: 72/72 in every required preset.

## Deferred boundary

M27 still owns actual INI and MapCache caller integration with the shared `setFPMode`; M28 and M20 retain their ledger-owned later consumers. No deferred finding blocks M26.
