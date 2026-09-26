# M22 plan 01 slice 08P0C2D0A: bounded Recording frame transaction

## Goal, dependencies and boundary

After accepted C2A and before B0/B, add an explicit bounded, opt-in public
command/resource transaction
capability with deterministic Recording commit/rollback. Existing begin/end
pass behavior remains unchanged unless the source explicitly selects this
transaction. Explicit modes are idle preparation and in-frame commands.
Unknown/SDL/other devices reject capability admission before source mutation
or C1 acceptance; bgfx support is D0B. B0/B and C2C/D require both.
No tree advancement/draw/factory/retail admission or generic renderer rewrite.
Authorization is not applicable to generated local tests.

## Exact transaction ownership

No existing checkpoint exists: Recording begin_pass increments view count and
appends commands, draw changes last-index state, uploads can change bytes,
end_pass marks attachments initialized and appends completion. Edge abort only
ends that pass. Introduce a source-compatible optional capability, explicit
device/generation transaction identity and finite command/resource/byte/view
budgets. Reject nested/stale/mismatched/malformed/over-capacity entry before
mutation. Only one declared transaction may own its device phase.

Idle-preparation mode admits only bounded declared resource/byte and marker
operations, never begin/end pass, view/viewport/clear/draw/present consumption.
Reject those operations before mutation; this mode cannot stand in for a frame.
In-frame mode owns the declared ordered pass/view/viewport/clear/draw/end/
present sequence with its frame baseline and budgets. Enter only from a valid
idle device baseline; an ordinary already-active pass, cross-mode operation,
nested transaction or mode switch rejects before mutation. Ordinary passes
remain unchanged outside an explicitly admitted transaction. Both modes are
needed: B0 preserves delayed source-stage admission outside a frame, while
C2C/D use the exact frame-mode capability after source preparation.

Record an admitted pre-phase baseline. Commit publishes complete owned ordered
commands/state once. Abort removes partial owned commands, restores pass/view/
target/last-draw state and owned modified bytes/initialization, and releases
new resources once. Prior live handles/resources/identities remain valid;
aborted candidate handles must never alias live retry handles. Do not roll
back consumed fault injection into endless retry. Diagnostics may retain fixed
failure categories but cannot masquerade as successful commands. Checkpoint
allocation failure leaves the device unchanged; destruction/rollback is
bounded and exception safe. Fixtures cannot edit private Recording containers.
The device owns command/resource rollback for either mode; B0 separately owns
engine texture refs/maps/transforms/revision/filter/sampler state. The engine
selects the declared phase; this slice never snapshots C1/RNG/FX. Preserve ordinary snapshots/pass behavior.

## Surfaces and acceptance

Expected surfaces: optional public GpuDevice capability, Recording owner,
contract docs, generated renderer tests and narrow edge readiness/integration.
Existing derived consumers compile with unsupported defaults. Positive tests
compare exact command/byte/resource/handle/pass/target/view baseline, complete
commit and two-generation retry. Negatives inject checkpoint/allocation,
create/upload, partial pass/viewport/clear/draw/end and release faults; prove
partial commands/resources removed and retry emits once. Cover bounds+1,
stale/nested transaction, unsupported device, duplicate commit/abort, owner
destruction and handle non-aliasing. Validate allocator pairing under both
sanitizers per AGENTS. Persist focused commands before edits. Run GCC/Clang
focused/sanitizer, six complete builds/canonical nonretail suites
(`-LE 'gpu|lan|retail'`, sanitizer `ASAN_OPTIONS=detect_leaks=0`), exact serial
host LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), established physical
Vulkan and serial LAN 4/4 all six, ledger/diff. Commit one slice:
`delivery: M22 08P0C2D0A own Recording frame transactions`.

Idle controls prove exact marker/resource/byte baseline, create/upload/release
fault rollback and retry, with explicit pass/view/viewport/clear/draw/present
rejection and no successful-command/view counters or target mutation; fixed
rejection diagnostics remain separate. Frame controls independently prove
complete frame commands/resources rollback and retry. Neither mode may consume
a fault twice or reuse an aborted candidate handle as an accepted retry handle.

Focused implementation command: `ctest --test-dir build/<preset> -R
'^(renderer_recording_transaction|renderer_recording_device|original_w3d_first_gpu_edge)$'
--output-on-failure`. New isolated renderer transaction witness owns its test
path; the unrelated renderer diagnostic remains excluded. Sanitizer focused
uses detect_leaks=0; serial strict host LSan selects
`renderer_recording_transaction|renderer_recording_device`, detect_leaks=1,
no UBSan override. Existing established physical gates remain required.
