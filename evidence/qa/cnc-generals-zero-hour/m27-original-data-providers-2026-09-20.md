# M27 original data providers evidence

Date: 2026-09-20
Scope: M27 only; asset-free x86-64 Linux data providers and independent map metadata

## Result

M27 acceptance is satisfied at its independent-provider boundary. Actual original random/CRC translation units and source-owned extractions of original file, INI, CSF, transfer, chunk, and MapUtil methods compile, link, and execute in GCC/Clang Debug/Release. Owned fixtures suffice; no retail content, private path, GPU, display, audio device, ARM64 target, or full engine initialization was used.

## Provider and behavior evidence

- `zh_original_data` exposes one production-facing interface used by the focused consumers and later integration. It compiles the actual original `RandomValue.cpp` and `crc.cpp`, plus original-tree extraction units for FileSystem, INI, CSF, Xfer, DataChunk, and independent MapUtil behavior.
- Checked provenance binds every extraction to named authoritative methods, verifies those methods still exist, verifies the complete production INI registry still names AIData, AudioEvent, MapCache, Object, ScriptAction, and ScriptCondition, and verifies the declarations under test are the declarations compiled by the production target.
- The config provider preserves separator/case normalization, logical traversal rejection, loose-over-archive resolution, archive/root ordering, and the original root-files-first then subdirectories INI two-pass order. Typed callback values and UTF-16LE CSF strings are bounded and malformed input fails deterministically.
- Fixed-width transfer and versioned nested chunks round-trip Bool, Int, Int64, Real, ASCII, and two-byte `WideChar` data. Invalid booleans, truncated data, impossible sizes, wrong versions, and output overflow fail without publishing partial output.
- The actual random provider retains independent logic/client/audio streams. Seed `0x12345678` has an initial six-word CRC of `0x933b34ac` and a logic CRC of `0xc4405f5c` after eight logic draws; intervening client/audio draws do not change the logic checkpoint.
- Independent MapUtil behavior parses owned WorldInfo/height/empty-ObjectsList metadata, skips unknown chunks, computes source rotate-add CRCs, enumerates standard and user maps with user precedence, and rejects nonempty ObjectsList with an explicit M20 integration requirement.
- Map cache records are bounded, validated, stored only below an injected absolute XDG cache root, and replaced atomically. Cold, warm, corrupt, and stale paths converge on identical metadata. A deterministic injected writer denial exercises the failure branch even under privileged test execution; arbitrary CWD and before/after read-root snapshots prove that read roots are unchanged.
- Provider-removal linking and scratch mutations of provenance markers, declarations, and the complete INI registry fail closed. The checked ledger keeps source-inspected coupled `INI.cpp` and `MapUtil.cpp` edges at inspected grade and assigns them to M20 rather than upgrading them to runtime evidence.

Runtime witnesses from `linux-gcc-debug`:

```text
original-data config: ok providers=OriginalFileSystem.cpp,OriginalINI.cpp,OriginalCSF.cpp blocks=4 labels=1 raw=23
original-data codecs: ok providers=OriginalXfer.cpp,OriginalDataChunk.cpp,RandomValue.cpp initial-crc=933b34ac logic-crc=c4405f5c raw=23
original-data map: ok provider=OriginalMapMetadata.cpp maps=2 cache=xdg crcs=17abf264,dda2c265 cache-files=1 raw=23
```

## Validation

Focused `original-data` label after stabilization:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 10/10 passed |
| `linux-clang-debug` | 10/10 passed |
| `linux-gcc-release` | 10/10 passed |
| `linux-clang-release` | 10/10 passed |

Canonical full asset-free CTest, run at milestone end with loopback-socket permission required by the existing LAN tests:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 82/82 passed |
| `linux-clang-debug` | 82/82 passed |
| `linux-gcc-release` | 82/82 passed |
| `linux-clang-release` | 82/82 passed |

Focused sanitizer validation used Clang Debug with `ZH_ENABLE_ASAN=ON`, `ZH_ENABLE_UBSAN=ON`, `ZH_ENABLE_GPU_TESTS=OFF`, and `ZH_ENABLE_RETAIL_TESTS=OFF`. Execution used:

```text
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
ctest --test-dir /tmp/zh-m27-sanitizers -L original-data --output-on-failure
```

Result: 10/10 passed. Because leak detection was disabled, this is not a leak-freedom claim. Runtime ownership probes instead establish that raw-allocation counts return to their pre-operation baseline in the config, codec, and map consumers; the map case also observes exactly one generated cache file.

An unprivileged full-suite attempt could not create loopback UDP sockets in two pre-existing LAN tests. The identical canonical suites passed after granting the required loopback permission; this was an execution-sandbox constraint, not an M27 or product failure.

## Acceptance mapping

- Actual original consumers in four presets: focused runtime results, compile-command/source checks, linked symbols, and provider-naming runtime witnesses.
- Encoding and random independence: fixed-width boundary fixtures, two-byte UTF-16 including a surrogate pair, characterized CRCs, and independent stream tests.
- Independent metadata/cache behavior: cold/warm/corrupt/stale, malformed/truncated/oversized chunks, standard/user precedence, nonempty-object rejection, deterministic write denial, and unchanged read roots.
- XDG confinement and arbitrary CWD: absolute injected cache-root containment, atomic replacement, generated-file count, and CWD-independent fixtures.
- Provider and registry integrity: extraction provenance, complete-registry source gate, provider-removal link failure, and negative drift controls.
- Ledger and classification: all checks pass; only manifest-listed original units actually compiled (`RandomValue.cpp` and `crc.cpp`) are promoted, and all 3,293 classified paths remain covered exactly once.
- Existing regression safety: 82/82 in every required preset and focused ASan/UBSan 10/10.

## Deferred boundary

M20 retains the complete `INI.cpp` dispatch integration, `ParseObjectDataChunk`, ThingFactory/MapObject creation, nonempty object classification, `addMap`, `INIMapCache`, localized `map.str`, registry callbacks, and complete production MapCache cold/warm equivalence. This evidence does not claim those behaviors. M24 retains full snapshot/replay compatibility. No deferred finding blocks the independent M27 provider contract.
