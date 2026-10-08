# N2 slice 01 — original core-owner acceptance

2026-10-07; transaction parent `11295c4feb535f97978b8335632440fccae82794`.
This accepts only slice 01, not N2/GameLogic or complete game startup.
No retail data was accessed; generated fixtures only. No adopted dependency edits.

## Delivered graph and boundaries

`original_core` compiles original RandomValue, CRC, Trig, AsciiString,
UnicodeString, GameMemory, MemoryInit and CriticalSection owners. Public native
headers and PreRTS no longer require Windows SDK allocation/math services.
Core-only CMake requires no bgfx/SDL/GPU. The original default DMA configuration
is used by three real init/shutdown cycles, not a replacement toy allocator.

Explicit game allocation is separate from standard ordinary/array/nothrow/aligned
allocation. Atomic string units outlive game pools. Class constructor rollback,
shared-pool DMA ownership, late rejected graph cleanup and corrected retries are
tested. Rejected malformed W3D pool identities and over-aligned classes do not
dereference providers or publish pools. Noncopyable holders cannot overwrite a
live acquisition. Initialized backing includes all declared capacity slots.

Five manifest-registered families cover values/CRC, independent original RNG
streams, strings, pools and repeated owners. The strings family holds 70,000 real
aliases for each string type and exercises concurrent independent copies. Pool
tests use four concurrent workers, grow/release exact slot counts and preserve
accepted owners through foreign/interior/duplicate/negative/zero rejection.
Valid RNG/CRC checkpoints and limitations are recorded in
`docs/original-engine-formats.md`; CSF UTF-16 is not inferred from native wchar_t.

## Frozen validation

Combined CMake/CTest build includes the unchanged N1 renderer regression tests.
All repository-owned sources are instrumented in sanitizer configurations;
stock renderer internals remain the independently built upstream GCC Debug SDK.

| Configuration | Result | Elapsed | Log SHA256 |
| --- | --- | --- | --- |
| GCC normal | 23/23 | 9.20s | b988fae979b2546b3fff35dc0d37825d173f2c737a70a9876e18810c46a073eb |
| GCC ASan/UBSan | 23/23 | 50.96s | c5952e82b2c15975be1ffe75621a3037a8b2b8a5e6773816d547e3274a502ca0 |
| Clang ASan/UBSan | 23/23 | 47.57s | 467f0e5ba2f2159cadf99f346896f5f21ca28e98076ff2b9aa67e3da3ee0b497 |

Logs: `build/qualification-{gcc,gcc-sanitize,clang-sanitize}/N2-core-matrix.log`.
Core-only sanitizer matrices also passed 5/5 each on the host. Initial sandbox
executions failed at LeakSanitizer's ptrace restriction, not an observed memory
defect; host reruns retain detect_leaks=1 and halt-on-error for both sanitizers.
No suppressions. Only GPU child processes use the approved isolated-bus fixture.
Native core builds had no compiler warnings after the coupled cleanup.
`git diff --check` and post-matrix `tools/upstream_bgfx.py verify` passed.

Frozen source identities:

- core fixture: e31260be8eb1ad84d1cbb100028c5deb8bca110fcd8a817b5255b06b7688bfbc
- core CMake: b5333e6cd226b3e0cc9d46291606c2a755b5b3f3332d211cc54420ef89a808cd
- top CMake: 805464571e444d539722284c1703198ed672dcc40b23e4243e7af3f1be93e5a7
- GameMemory.cpp: 2e73f2628e0cbc21bd69bc11f9c38227c912be4b219d800ece17b5b87723af41
- GameMemory.h: 9060d8ef941c28e6bebce553f6ecb6b1b1f90953aed5103146205cd9b72acc36
- RandomValue.cpp: 201b667765b8b6d52ad173973356ecf453451ead4d080655910fc01b8eb8c93e
- StringStorage.h: 99a33252aa740c926da7451a9e675122d113d6bdf1c798209a9f2c057e959931

## Remaining N2 gates

Rooted BIG/INI/CSF readers, actual original GameEngine/GameLogic/module updates,
compiler-matching simulation checkpoints and private-input read-only integrity/
completeness remain slices 02/03. This fixture cannot prove these gates, visual
parity or the later native renderer/source integration.
