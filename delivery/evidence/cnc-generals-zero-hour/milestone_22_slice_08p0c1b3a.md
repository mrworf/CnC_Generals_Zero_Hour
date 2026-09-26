# M22 slice 08P0C1B3A: read-only position FX graph admission

Linux original-source `FXList` now preflights a positional call without
dispatching a nugget. It admits an absent optional root as a no-op, rejects
invalid coordinates and unavailable shroud/local-player providers, verifies
every list identity against the live store before dereference, and bounds
recursive traversal to 16 active levels and 64 distinct lists. Active-stack
cycles fail closed; repeated DAG children are allowed. The scan continues
past an unsupported nugget to catch later structural faults, so a late
unsupported or foreign child cannot follow an earlier irreversible effect.
Source dispatch remains closed until B3B.

All eight native nugget categories have explicit Linux-only positional
readiness. Light pulse (pending CPU display implementation), object-only
`FXListAtBonePos`, unknown and null nuggets reject. Tracer and ray need a
secondary position and an existing template; their ThingFactory check is an
exact read-only map membership peek because ordinary lookup can synthesize
`LOAD_TEST_ASSETS` templates. Audio, GameClient, GameLogic and particle
providers are checked as applicable. No fields or Windows declarations
changed; the Linux provider archive and both full-draw and minimal/headless
consumers compile the same virtual definitions. `FXNugget` was already
polymorphic, so the new Linux virtual slots add no object field; the guard is
absent from Windows/retail declarations. The provider archive exports the
preflight symbol, both `zh_original_w3d_full_probe` and `zh_original_main`
link, and the native `original_headless_update` controls pass under both
toolchains.

The generated two-generation witness has null/empty/one/multi positives,
available and missing tracer/ray template cases, absent providers including
the fresh-null PlayerList global and initialized-local-player retry,
nonfinite primary and secondary coordinates, late light-pulse and
object-only rejection, non-store root/child and stale-address rejection,
self/mutual cycles, a shared DAG, depth/node bounds, null/unknown nuggets,
and a clean graph retry. Distinct pooled edge nuggets are uniquely owned by
their list, while child pointers are non-owning. The fixture clears every
synthetic graph edge before each terrain generation tears down. Factory
count and existing-template identity, Client and Logic RNG states, frame,
Recording resource counts, audio owner and an explicit nugget-dispatch
counter are invariant across admission and rejection. The original
64-type/4000-instance terrain workload and redacted timeout remain intact.

Final-source GCC and Clang Debug focused generated witnesses pass 1/1 in
58.67s and 60.53s. All four GCC/Clang Debug and Release canonical nonretail
suites pass 267/267 each under `-LE gpu|lan|retail`, including generated
terrain in 60.29s, 60.17s, 25.00s and 24.03s. All six GCC/Clang Debug,
Release and ASan+UBSan complete builds pass. GCC and Clang ASan+UBSan
canonical nonretail suites pass 267/267 each, serially with
`ASAN_OPTIONS=detect_leaks=0`; generated terrain passes in 264.29s and
217.31s without a timeout or sanitizer finding. Separate focused GCC and
Clang sanitizer generated witnesses pass 1/1 each in 265.60s and 217.49s.
Host-escalated physical Vulkan passes GCC display/bootstrap/map 3/3 and
Clang display/map 2/2 under CTest's
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, with no validation-error or
VUID category; host driver reports 615.71.09.

Serial host LAN passes 4/4 in all six configurations. Host-escalated strict
GCC and Clang LSan use exactly `ASAN_OPTIONS=detect_leaks=1` with no
`UBSAN_OPTIONS` override and pass display-owner plus generated terrain 2/2
each (GCC 6.14s/263.32s; Clang 5.77s/215.22s) without leaks. The dependency
ledger and `git diff --check` pass. The pre-existing unrelated renderer test
diagnostic is untouched and remains unstaged.
