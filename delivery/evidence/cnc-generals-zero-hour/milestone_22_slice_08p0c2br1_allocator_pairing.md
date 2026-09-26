# M22 slice 08P0C2BR1: ordinary nothrow allocator pairing

Status: complete; accepted final-source allocator behavior and gates.
Plan checkpoint: `9f17c079ebd1a293dafa078025e780767c13e950`.
C2B executable/test changes and unrelated renderer diagnostic remain unstaged.

## Exact correction and failure classification

Vulkan validation SPIRV-Tools stable-sort temporary allocation used runtime
ordinary nothrow new; cleanup resolved original ordinary/sized delete. Both
sanitizer shroud physical proofs failed during bgfx initialization before physical
shroud owner creation. Independent unchanged GCC tree-program physical reproduced
the same external-library cleanup route. Public source categories: pool list
access GameMemory.cpp:989 and freeBlock:1794; Clang symbolized checkLayout temporary
cleanup. This was not a VUID, timeout or shroud pixel assertion. Generated private
localization logs stay outside tracked artifacts.

Linux now defines only ordinary non-aligned scalar/array nothrow new and their
matching placement deletes. They call existing ordinary allocation/deallocation;
bounded Int/private-header/rounding preflight rejects before narrowing. Every
allocation exception becomes null; original throwing new is unchanged. Aligned
overloads remain C++-runtime-owned. No DMA/pool algorithm, concurrency, public
fault selector, fallback malloc, Windows or serialized ABI is changed.

Only the allocator test links a one-shot malloc wrapper. A genuine failed raw
request produces null once with exact counts, then retry succeeds. Pool/raw,
zero-size, header overflow, cross-target ordinary/sized/placement cleanup and
scalar/array throwing-constructor unwind are asserted. Existing aligned foreign
and cross-target controls remain active. Defined symbol inspection shows exactly
the four new strong ordinary nothrow overloads in all six configurations and no
original aligned override. GCC release also emits local cold clones; Clang
sanitizer retains weak aligned interceptors owned by
libclang_rt.asan_cxx's asan_new_delete object (link-map proof), not GameMemory.cpp.
Those runtime-owned aligned symbols and existing foreign aligned behavior are
unchanged; raw symbol counts are not mistaken for original ownership.

## Frozen source identity

| Path | SHA256 |
| --- | --- |
| `GeneralsMD/Code/GameEngine/Source/Common/System/GameMemory.cpp` | `ab0e83c6b109d22299f17b467ae2e20badabe53ddae8b154046ef524122f2712` |
| `tests/original_runtime/test_allocator.cpp` | `3f77d156e54b7c1d01ce47cbaa4f37e0d9b90abeabe842d09dc7aeec8c9b17d4` |
| `tests/original_runtime/allocator_provider.cpp` | `79d069e8b0a53dd639dee51dd5ffdcbd71b259d892aa18b28d59ea0bdd2d8599` |
| `CMakeLists.txt` | `c46f30721109cd1444a9167239dda40371277dcf569d1c8e2638ef06051f4b9a` |

## Final-source gates

Four native/sanitizer focused selections pass3/3 each. Exact host strict LSan
with `ASAN_OPTIONS=detect_leaks=1`, no UBSan override, passes3/3 both toolchains.
Independent validation-enabled GCC and Clang sanitizer tree-program physical
controls pass, preserving source program/pixel assertions; complete logs contain
no ASan/UBSan/validation category. All physical/strict runs use host escalation.
Ledger freshness passes. Six complete builds, six serial canonical nonretail
suites, established native Vulkan, six serial LAN4/4 and final ownership/diff
review remain required before acceptance. No required gate is waived.

Approved provenance boundary: these complete builds/canonicals/LAN/established
Vulkan gates execute the exact frozen BR1+C2B composition. The allocator-only
commit changes no executable bytes; C2B may reuse those gates after exact hash
verification, refreshing its own nine-control focused/strict LSan and four
generated physical configurations. This is unchanged-source reuse, not acceptance
of a failed or stale gate; C2B remains independently unaccepted/uncommitted.

First full GCC Debug canonical completed275/276; only C2B map-frame fixture's
obsolete projected marker assertion failed. Generated debugger evidence showed
the accepted shroud upload before view/scene/all14 draws and no live device
checkpoint. Approved C2B fixture correction asserts exact accepted content/epoch,
descriptor/bytes and the sole candidate/upload sequence instead, preserving
draw/rollback/retry/teardown coverage. BR1 production/test hashes above are
unchanged. The failed canonical is superseded; corrected frozen composition
refreshes affected gates and restarts all six serial canonical suites.

Corrected composition six builds complete, C2B ten-control focus passes10/10
all four configurations, both strict10/10 and generated physical proofs pass
all four. Complete logs are clean. The six canonical suites restart serially
from zero under isolated load; this owner remains unaccepted until they pass.
The recurring Clang Ninja premature-log recovery expands the mandatory build
to860 actions; it completes without compiler error. No build metadata/source
workaround or duplicate build was introduced.

Corrected canonical checkpoints: GCC Debug276/276 (285.96s), Clang Debug276/276
(268.18s); complete logs clean. Release and sanitizer selections remain queued
in the same serial run. No acceptance claim precedes their completion.

Both corrected release canonicals also pass276/276: GCC151.73s, Clang102.96s;
complete logs clean. Four native suites complete; the two sanitizer suites are
now the remaining canonical gates, isolated and serial.

Corrected GCC sanitizer canonical passes276/276 (882.57s). Archived complete
per-test LastTest.log and console log are both clean, with no runtime/sanitizer/
validation/failure category. This is normal isolated workload duration, not a
timeout. Clang sanitizer is the final queued/running canonical gate.

The initial compile rejected the legacy max macro at a standard numeric-limits
call; parenthesized max invocation corrected test portability before freeze.
No allocator semantics were changed to address this compile-only issue.

## Final accepted checkpoint

All six complete builds pass. Corrected serial canonical suites, including full
per-test output audit, are clean276/276 each:

| Configuration | Seconds |
| --- | ---: |
| GCC Debug | 285.96 |
| Clang Debug | 268.18 |
| GCC Release | 151.73 |
| Clang Release | 102.96 |
| GCC ASan+UBSan | 882.57 |
| Clang ASan+UBSan | 708.58 |

No runtime-error/ASan/LSan/validation/failure categories appear in the archived
complete per-test logs, not merely the console summaries. Native Vulkan passes
3/3 GCC and2/2 Clang; serial LAN passes4/4 all six. Allocator focused3/3 all four,
strict host LSan3/3 both and independent validation-enabled sanitizer tree-program
physical both pass. Corrected composition additionally passes C2B focus10/10
all four, strict10/10 both and generated physical all four. Ledger/hash/symbol/
header/diff review is clean; all four allocator hashes remain unchanged.

Only allocator source, test/provider, its test-only link option, this plan/index/
evidence and the GameMemory ledger row enter the implementation commit. All
eleven C2B code/test files, remaining C2B ledger hunks/evidence/plan and the two-line
unrelated renderer diagnostic stay unstaged. C2B is separately accepted only
after its post-commit focused/strict/physical refresh and exact identity check.
Progress checkpoints above are historical; this final checkpoint supersedes
pending-status statements, never the documented failed275/276 classification.
