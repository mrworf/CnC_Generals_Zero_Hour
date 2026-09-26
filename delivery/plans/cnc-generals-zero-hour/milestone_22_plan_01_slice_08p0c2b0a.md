# M22 plan 01 slice 08P0C2B0A: bounded source-reference retirement

## Goal, dependencies and scope

After C2A and accepted D0 Recording/bgfx idle capability, establish an exact
OriginalGpuEdge-generation owner for retained regular TextureClass reference
units. Future B0B stage commit/cancel can retire pins without invoking a fallible
source destructor/device callback after acceptance. This is independently
reviewable source lifetime behavior, not the already accepted B1 native queue.
No source-stage maps/transform/filter transaction, shroud semantics, source load,
tree/frame/factory/retail or allocator rewrite. Generated local work has no
authorization change. Ordinary unowned texture/refcount behavior stays intact.

## Confirmed callback and admitted ownership

RefCountClass::Release_Ref deletes at zero. TextureBaseClass destruction invokes
release_texture_if_owned, which retires pending samplers and the last native
texture alias. Recording destroy can allocate command/diagnostic strings;
therefore a sole prior source pin cannot be released directly from B0B's
allocation-free no-throw finish. Do not infer an external reference remains.

Use a fixed bounded queue of acquired reference units (64 maximum), plus one
nonnested reservation binding exact edge/generation/monotonic sequence/count.
The immutable admission list may repeat a source identity: each acquired unit
has exactly one eventual release, never collapse reference multiplicity.
Preflight full count/remaining capacity, active edge/generation, idle/device
capability and each exact source map membership before pointer dereference or
Add_Ref. Require supported initialized regular resident TextureClass providers;
no Init/file lookup/filter/access/resource mutation. Null/stale/foreign/unready
providers, count65, queue capacity+1, sequence overflow and nested admission
reject before any reference or queue/token mutation. Failed/foreign admission
cannot poison an existing reservation. Every accepted unit pins its object.

Admission acquires units only after full immutable-list validation and bounded
storage readiness. Finish and cancellation transfer all already-owned units
into reserved queue space, no allocation, Release_Ref, marker, callback or native
operation. They are no-throw and exactly once; all token fields must match.
Canceled units still require release; failure-injection counters remain consumed.
Bounded diagnostics/read-only counts distinguish reserved, queued and drained
units without relying on implicit padding or undocumented private checkpoints.

## Exact-generation ordinary drain

Drain is explicit at an ordinary API-thread boundary, not inside any source/device
transaction or active frame/pass. Require this same live active edge, device and
generation; foreign/stale generation fails without touching queue or source.
No queue migration or replay into a later generation. Future B0B uses this
accepted boundary after its own attempt ends; no stage semantics are added here.

Count queued units by source identity against Num_Refs. Surviving sources keep
their exact identity/native publication; release only the queued unit count.
For terminal sole-owned identities, preflight complete source/native alias
ownership and pending sampler cleanup before mutation. Compose the existing D0
idle device journal: prepare each required native resource destroy once according
to actual source-handle owner multiplicity, commit all device cleanup, then
nonallocating edge-map/ref/pending metadata detach and exact pooled Release_Ref.
The terminal destructor now finds no owned edge entry and makes no device call.
Shared missing-texture ownership is not destroyed by an individual source;
distinct source aliases sharing one handle retire that handle only at its last
source ownership unit. Multiple pins for one source are not native acquisitions.

Source maps/pending values/revision and the queue stay unchanged until device
cleanup commits. Preflight revision arithmetic before the bounded metadata
publication. A failed checkpoint/allocation/provider/destroy/commit is retryable:
abort that exact device journal and preserve every queued unit and accepted
source/native identity. No half-detach, duplicate decrement or half-retirement.
After success, queue units are consumed exactly once, surviving refs stay exact,
terminal metadata is absent, and repeated drain is an allocation-free no-op.
Do not allocate a private duplicate device checkpoint or use frame/reset/readback
to obtain capacity/readiness. Physical readback is outside completed cleanup.

## Reset, shutdown and terminal error disposition

Reset/removal/shutdown first cancels any admitted pin reservation into its already
reserved queue, then drains while the exact edge/device generation is valid,
before ordinary texture-map invalidation or changing the active-edge provider.
Where an existing CPU texture lifecycle entry can invalidate an enrolled source,
use a narrow owner notification before its ordinary mutation; pin the receiver
across that notification so draining a sole queue reference cannot delete it
mid-call. Unenrolled ordinary behavior and Windows/retail declarations remain
unchanged. Do not leave a deferred pointer to an invalidated or foreign owner.
Expose a retryable shutdown/drain result while the owner remains alive: failure
retains queue/pins/maps and permits exact clean retry. Destruction cannot silently
return with queued pins, abandon them to another generation or claim zero residual.
If mandatory final drain cannot complete, emit only a fixed public cleanup category
and terminate through the existing process-fatal boundary; never ignore failure.

Native device loss/fatal backend allocation is not recoverable queue rejection.
Before successful cleanup commit, the queue is terminal-failed/not consumed and
source metadata stays attached; the process must terminate with the original
owner still holding those references. It cannot retry, resume or instantiate a
replacement generation. If native loss is reported after cleanup commit, there
is no rollback: the admitted CPU cleanup consumes its source units/detaches
metadata exactly once before reporting terminal failure when API control remains;
native asynchronous fatal termination may stop the process directly. No wrapper
claims reversible worker/backend state or leak-free normal teardown for a process
that terminates. Normal successful/error-retry paths require zero residuals.
Do not add a broad device-loss detection/interface or suppress native fatal errors.
Generated subprocess models explicitly assert precommit terminal-failed versus
postcommit-consumed dispositions and fixed fatal categories/no normal return;
these are modeled contract controls, not a claim to inject real GPU device loss.

## Surfaces, witnesses and acceptance

Expected surfaces: OriginalGpuEdge bounded source-reference owner/API and cleanup
ordering, narrow CPU texture lifecycle notification where required, isolated
generated source-reference witness, CMake, ledger/contract docs
and evidence. No original RefCountClass layout/global allocation change. Do not
touch the unrelated renderer diagnostic. New aggregate/value members initialize
explicitly; comparisons use fields. Avoid test-only shipping lifecycle logic.

Two generations prove sole-ref deletion, surviving refcounts, repeated source
pins, distinct aliases/shared native ownership, pending samplers, shared missing
source, cancellation/retry and active-owner shutdown. Include exact64/capacity+1,
null/stale/nonresident/foreign generation, nested reservation/all token mismatch,
no-mutation admission, failed device checkpoint/destroy/commit rollback, duplicate
finish/drain and ordinary behavior. Verify exact source/native residual immediately
before complete teardown and clean next generation. Modeled fatal subprocesses
prove terminal outcome cannot become an ordinary leak waiver or stale replay.
Physical bgfx proves prior accepted sampled pixels/resource identity survive failed
cleanup, then successful exact-generation drain retires once without frame calls.

Persisted focused commands:

`cmake --build build/<preset> --target original_w3d_source_reference_tests original_w3d_gpu_edge_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_source_reference|original_w3d_gpu_edge_failure|renderer_recording_transaction|renderer_bgfx_transaction_resource)$' --output-on-failure`

Run both native/both sanitizer focused sets; exact strict serial host LSan selects
the same four CPU tests with `ASAN_OPTIONS=detect_leaks=1`, no UBSan override.
Generated `original_w3d_source_reference_tests --gpu` uses the established host
validation-clean wrapper on both native/both sanitizer configurations. Six full
builds/canonicals `-LE 'gpu|lan|retail'` require clean complete-log audits with
sanitizer detect_leaks=0. Established host Vulkan and six serial LAN4/4,
ledger/header/diff and exact-owned stage review complete acceptance. Commit:
`delivery: M22 08P0C2B0A retire bounded source references`.
B0B resident selected-stage transaction is dependency-next, then B0 aggregate.

## Implementation audit checkpoint

Resident admission includes the device's read-only `describe_texture_format`
membership check, before dereferencing the mapped provider or acquiring a unit;
this enforces the existing resident-owner requirement without lazy initialization.
All fixed value members initialize explicitly and token comparison is fieldwise.
The initial physical witness target used RGBA while its accepted video pipeline
requires BGRA: only the fixture target format was corrected. Production texture
format/shader semantics were unchanged. Generated terminal subprocesses model
precommit retained and postcommit consumed dispositions and mandatory destructor
failure; they do not claim physical device-loss injection.

## Accepted outcome

Fixed source-reference ownership and exact-generation transactional retirement
are independently accepted. Two-generation CPU/physical controls prove bounded
admission, exact unit/alias multiplicity, receiver-safe invalidation, failure
rollback/retry, once-only shutdown and declared fatal disposition. All six
complete builds and canonical nonretail suites pass 275/275 with clean complete
logs; focused, strict host LSan, generated/established Vulkan, six serial LAN,
ledger/header/diff and frozen-hash checks pass.
[Evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2b0a_source_references.md).
Commit boundary: `delivery: M22 08P0C2B0A retire bounded source references`.
B0B owns selected-stage state and composes this reference owner next.
