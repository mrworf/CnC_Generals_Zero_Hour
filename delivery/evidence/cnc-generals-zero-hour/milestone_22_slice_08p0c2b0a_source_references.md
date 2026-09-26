# M22 slice 08P0C2B0A: bounded source reference retirement

Parent: `46f8cbf` (persisted B0A → B0B → B0 lifetime split).
Status: independently accepted on the frozen source below.
Generated assets only; no retail admission or private diagnostic data.

## Ownership and recovery

An exact edge/generation/monotonic-sequence/count reservation acquires at most 64
regular ready resident TextureClass reference units. Membership precedes source
dereference, and read-only native format membership rejects destroyed device
handles. Repeated identities preserve every acquired reference, not just unique
objects. Null/stale/unready/foreign/nested/capacity rejection does not alter
references, tokens, source access time, initialization or device publication.
Sequence/refcount/revision arithmetic rejects exhaustion before acquisition or
publication; actual64-unit capacity bounds all local counters. These exhaustion
checks are source-audited, without manufacturing billions of refcount units or
adding a private mutation API. Fixed aggregates initialize explicitly, and
identity/token assertions compare fields rather than padding.

Finish/cancel transfers fixed storage into the queue without allocation, marker,
Release_Ref or native callback. Exact ordinary drain counts source multiplicity
and native source-owner aliases independently, preflights arithmetic/ownership,
and prepares pending sampler/last-native-texture removal inside accepted D0 idle
transactions. Only after commit does it erase terminal maps/pending metadata and
release pooled references. Surviving identities keep their publication. Checkpoint,
destroy/diagnostic-allocation and commit failures preserve queue/maps/revision,
refcounts and accepted resources; retry consumes exactly once. Empty drain does
not touch the device. No private checkpoint, frame advancement or broad allocator
change is introduced.

Enrolled CPU invalidation pins its receiver before cancellation/drain, then
performs ordinary mutation. Failed notification leaves the receiver initialized
and queue pinned. A sole queued receiver cannot delete itself mid-call. Explicit
shutdown is retryable; destructor cancels/drains before invalidating its generation
and cannot return with retained pins. Fixed-category fatal subprocess models
prove precommit retained versus postcommit consumed disposition and mandatory
destructor failure; these are not physical device-loss injection. Actual native
fatal loss remains process-fatal, with no replacement-generation replay or normal
teardown/leak waiver.

## Generated witnesses and fixture classification

Two generations cover exact64/+1/full queue, all four token mismatch fields,
nested ownership, stale generation, sole/surviving refs, repeated units, distinct
shared-handle aliases, pending sampler cleanup, shared missing native owner,
invalidation failure/retry, sole receiver lifetime, shutdown failure/retry and
active-reservation owner destruction. Native and source residuals are checked
immediately before teardown, with the shared fallback explicitly owned by the
edge until its destructor. No source-stage or shroud transaction is claimed.

Physical video-shader sampling proves accepted green pixels survive injected
cleanup rejection, source identity stays exact, and successful finish/drain makes
no immediate native destroy/frame call. Ordinary wait drains native ownership.
The first physical fixture incorrectly requested an RGBA target for its existing
BGRA video pipeline. Correcting only the fixture target format resolved this;
production shader, texture format and ownership behavior were unchanged.

## Frozen source

- CMakeLists.txt: `69ae03a8e6ec4ee2e2e05dc9c714657b1c2d9ae4dde039da5eb6fbd667cdc9c8`
- src/original_runtime/original_gpu_edge.cpp: `274cc64c98ffb6dab866d7e9f87a1161ad77ece2d88471ac150e5c19af28d541`
- src/original_runtime/original_gpu_edge.h: `4fddc13a0f16e74a0cc455856932d385a54b4f7148158c281898a742cd43b93d`
- src/original_runtime/texture_cpu.inc: `9c9f7b0719167b723be80c3e8c5628bf4907c2883a99f2f61a89964a97849abb`
- tests/original_rendering/test_source_reference.cpp: `f73d855c7a440716d01823c540c8f3b348c4e1ec30cae79d0d1888d20f48c955`

## Final acceptance checkpoint

Exact focused regex selects source-reference, original edge failure, Recording
transaction and bgfx resource transaction: 4/4 each GCC/Clang debug and sanitizer.
Both sanitizer complete focused logs have zero sanitizer/runtime-error findings.
Strict serial host LSan uses the same four tests, `ASAN_OPTIONS=detect_leaks=1`
with no UBSan override after all six complete-build relinks: GCC 4/4 in 0.59s
and Clang 4/4 in 0.30s, clean logs. Release focused sets also pass 4/4 each.
Four serial generated physical source-reference Vulkan runs pass the established
validation-clean wrapper with host graphical escalation and validation layer.

All six complete builds pass. GCC debug canonical passes 275/275 in 320.30s,
Clang debug 275/275 in 368.66s, GCC release 275/275 in 297.81s and Clang release
275/275 in 172.90s, each with clean complete-log audit.
Established host Vulkan GCC display/bootstrap/map
passes 3/3 in 7.70s and Clang display/map passes 2/2 in 4.38s, validation-clean.
Serial host LAN passes 4/4 in all six configurations, with clean full-log audits,
in 1.78s, 1.78s, 1.79s, 1.79s, 2.01s and 1.93s respectively.
GCC sanitizer canonical passes 275/275 in 948.91s, including the full terrain
witness in 266.49s, with clean complete-log audit. Clang sanitizer canonical
passes 275/275 in 696.98s, including terrain in 223.40s, with clean complete-log
audit. Both suites use `ASAN_OPTIONS=detect_leaks=0`, no UBSan override, and run
serially without concurrent heavyweight build/test load. Every canonical complete
log is audited before later selections can replace it.

Ledger/public-header/diff checks pass and all five frozen source hashes remain
exact. Native bounded-submission patch bytes remain unchanged. Exact staging
owns only this slice's source/test/CMake, plan/index, ledger/contract/discovery
and evidence paths; the unrelated two-line renderer diagnostic stays unstaged.
B0B resident selected-stage atomic application remains dependency-next; shroud,
tree draw/factory and retail admission remain closed.
