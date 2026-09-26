# M22 plan 01 slice 08P0C2D0A: bounded Recording frame transaction

## Goal, dependencies and boundary

After C2A/B, add an explicit bounded, opt-in public frame-command transaction
capability with deterministic Recording commit/rollback. Existing begin/end
pass behavior remains unchanged unless the source explicitly selects this
transaction. Unknown/SDL/other devices reject capability admission before any
source C1 acceptance; bgfx support is D0B. C2C/D cannot proceed until both.
No tree advancement/draw/factory/retail admission or generic renderer rewrite.
Authorization is not applicable to generated local tests.

## Exact transaction ownership

No existing checkpoint exists: Recording begin_pass increments view count and
appends commands, draw changes last-index state, uploads can change bytes,
end_pass marks attachments initialized and appends completion. Edge abort only
ends that pass. Introduce a source-compatible optional capability, explicit
device/generation transaction identity and finite command/resource/byte/view
budgets. Reject nested/stale/mismatched/malformed/over-capacity entry before
mutation. Only one declared transaction may own its frame.

Record an admitted pre-frame baseline. Commit publishes complete owned ordered
commands/state once. Abort removes partial owned commands, restores pass/view/
target/last-draw state and owned modified bytes/initialization, and releases
new resources once. Prior live handles/resources/identities remain valid;
aborted candidate handles must never alias live retry handles. Do not roll
back consumed fault injection into endless retry. Diagnostics may retain fixed
failure categories but cannot masquerade as successful commands. Checkpoint
allocation failure leaves the device unchanged; destruction/rollback is
bounded and exception safe. Fixtures cannot edit private Recording containers.
The device owns command rollback, the engine selects source phase, and this
slice never snapshots C1/RNG/FX. Preserve ordinary snapshots/pass behavior.

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
