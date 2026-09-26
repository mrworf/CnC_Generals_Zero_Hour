# M22 08P0C2B0: delayed-source transaction aggregate

Status: accepted evidence-only composition. Parent/payload dependency:
`bbe0b93a9842fbdc080eae346ccf645e044344cc` (B0B).
Reference prerequisite: `85ac8129fa70851d86c37ffb8f2c6b663dc002e5` (B0A).
Approved composition refinement: `b06e99ee8fc7e215378374fca36c39f991e72f4e`.

## Reviewed source lifetime and stage boundary

[A](milestone_22_slice_08p0c2b0a_source_references.md) owns fixed 64 acquired
source-reference units, exact generation/sequence/count identity, no-throw
finish/cancel, and ordinary transactional cleanup before terminal metadata
detach and pooled release. Native alias ownership is multiplicity, not unique
handle count. Failed admission/destroy/commit preserves queue and publication
for exact retry; mandatory final cleanup cannot abandon pins. Modeled fatal
subprocesses distinguish retained versus consumed terminal disposition, without
claiming physical device-loss coverage or stale-generation replay.

[B](milestone_22_slice_08p0c2b0b_stage_transaction.md) composes A with complete
resident selected-stage tuple/ref/map/transform/LastAccessed/revision/filter/
sampler atomicity. Every declared provider and bound is admitted before pins,
load/init/access mutation or idle-device admission. Selected-only application
leaves pending shader/material and unselected stage state unchanged. Native
commit precedes allocation-free pin transfer; abort restores native state
before source state and cancellation transfer. No possibly terminal source
release occurs in the no-throw stage commit/abort boundary.

B's approved A correction groups exact source identities. Only when every group
has `Num_Refs > queued multiplicity` can ordinary drain release units without
allocations, callbacks, native admission/touch or fault consumption. Equality,
mixed terminal groups, stale generations and uncertain ownership retain A's
existing transactional/rejection path. This supplies complete begin-failure
source cancellation even when device capacity rejection persists.

Reset, source invalidation/removal and edge destruction cancel while the same
owner generation is live. Ordinary drain then retires terminal metadata and
resources once; native B1 retirement remains deferred until its valid boundary.
Scope guards precede provider dereference, phase diagnostics and allocation;
commit-ready negatives prove poisoning, not merely unapplied state.

## Unchanged final-source acceptance reuse

B0B's complete final-source evidence reruns A's changed composition rather than
reusing pre-correction A acceptance. Six complete builds and six serial canonical
nonretail suites pass 276/276 with clean complete logs. Native and both sanitizer
focused unions pass 6/6; exact strict host LSan (`ASAN_OPTIONS=detect_leaks=1`, no
UBSan override) passes 6/6 both. Generated A/B physical Vulkan passes 4/4 each;
established native GCC display/bootstrap/map 3/3 and Clang display/map 2/2 pass
validation-clean. Serial LAN 4/4 all six, ledger/public headers/diff and exact
owned staging pass. Frozen hashes and pinned native patch remain exact.

This aggregate changes only plan/index/discovery/evidence, with no executable
surface or test workload. The minimal-slice documentation exemption applies:
review composition and diffs, reuse the unchanged gates, do not rerun them.
The unrelated renderer diagnostic remains unstaged. Exact shroud semantics
are dependency-next; tree preparation/draw/factory, retail and M22 acceptance
remain closed. The approved critical-path scope freeze remains in force.
