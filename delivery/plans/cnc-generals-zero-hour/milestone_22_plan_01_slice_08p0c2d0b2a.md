# M22 plan 01 slice 08P0C2D0B2A: bounded public bgfx native reservation

## Goal, dependency and boundary

After D0B1 and before deferred journal D0B2B, provide a narrow opt-in public
bgfx reservation/admission extension for a bounded immutable replay. This is
not cancellation of already submitted work, a private-header consumer, a
global allocator change or a broad renderer rewrite. Ordinary bgfx and SDL
behavior stays unchanged. No tree/source/retail admission or Windows/retail
engine ABI change. Generated local work has no authorization change.

## Verified requirement and complete native boundary

Public Init cannot reserve the TinySTL binding-cache nodes; submit also reaches
lazy frame/binding arrays, debug tracking and uniform-buffer growth. Resource
destroy/resize/frame finish can grow command buffers. Parent D0B records the
audited pinned locations. Reserve bucket capacity alone is not sufficient:
the map allocates each new node. The extension must cover every API-thread
allocation reachable during admitted view/touch/bind/uniform/state/submit,
presentation resize, native retirement and final frame completion.

Introduce explicit finite native budgets and exact internal context/frame/sequence
ownership, single API-thread owner, no nested/concurrent encoder overlap. Admit
only supported native command categories before any accepted-target touch.
Admission consumes a complete immutable native manifest, not a budget-only
promise followed by arbitrary API calls. Deep-copy admitted command parameters
and uniform payloads before publishing admitted ownership; validate every category,
resource identity/refcount and source-order use/retirement before replay.
Unknown categories, bounds+1 or an invalid late command reject the whole
manifest before view/touch/submit/resize/retirement. D0B2A owns admission,
reservation and allocation-free native replay primitives; D0B2B owns the
source device journal's construction/order and invokes those primitives.
Prefer one synchronous public admit/reserve/replay call: B2B builds its entire
immutable manifest beforehand, then A deep-copies, validates, reserves and
replays while holding the native API/frame ownership boundary, before returning.
There is no exposed token interval in which intervening native mutation can
invalidate admission. Return an exact context/frame/sequence receipt only on
success; rejection preserves its caller-provided baseline. Keep render-completion
semaphore ownership local and restore it on every pre-replay rejection without
frame advancement. A second public call is not an implicit duplicate finish;
B2B's journal generation owns high-level at-most-once emission.
Prepare replacement storage before publication; allocation failure must leave
the ordinary frame/encoder command prefix and accepted resources unchanged.
Reserve full render/binding/frame-array extents, uniform stream including
opcode/alignment/end/threshold margin, resource pre/post command storage and
debug tracking. Provide bounded allocation-free binding-cache insertion with
full binding equality/collision handling rather than a hash-only identity.
Do not silently disable validation or required debug assertions. Account for
already queued ordinary work and current buffer positions; no frame/reset
shortcut obtains a fresh baseline. Clamp/overflow/bounds+1 fail admission.

Keep extension state private to its native owner, exposed by a reviewed public
API only. Native implementation may use its own internals; engine/device code
must not include bgfx private headers or call private symbols. Bind lifetime
to the current native frame and internal identity; reject overlapping owner
or misuse before mutation. Rejected reservation before replay restores only
reservation metadata; there are no submitted commands to undo. Admitted replay
uses reserved bounded storage exclusively; ordinary fallback allocation must
not be reached. An operation outside the admitted budget/category poisons
admission before irreversible execution, never midway through replay.

D0B2B supplies fully validated immutable commands and native resources before
reservation is accepted. Its replay cannot query source registries or callback
providers. Recoverable resource/capacity/CPU allocation failures occur before
commit; driver/device loss and fatal process OOM retain ordinary bgfx's fatal
boundary, not a fake recoverable rollback promise. No fallible API-thread
storage growth, transient allocation or wrapper creation remains after commit.
Worker/backend allocation is outside this API-thread rollback guarantee.
Existing device-loss/error handling and physical Vulkan validation cover that
boundary; a worker allocation is never falsely claimed reversible or counted
as an API-thread success. Single-threaded native rendering rejects this
capability because backend execution would occur on the API thread.

## Completed read-only allocation inventory and supported manifest

Pinned `bgfx_p.h` and `bgfx.cpp` establish the following required owner map:

| API-thread path | Required admission/reservation behavior |
| --- | --- |
| Encoder bind/submit/discard | Reserve bounded full-equality binding entries and render-bind arena blocks; hash collisions cannot alias distinct bindings. Preserve prior ordinary encoder/command prefix. |
| Draw-frequency uniform stream | Copy complete immutable values; reserve opcode/payload/alignment/End plus threshold margin. Replace only the admitted debug-uniform membership path with bounded exact tracking, preserving duplicate/frequency checks. |
| Render-item/depth-control arenas | Preallocate all blocks reached by admitted draws/touches and depth-control calls before publishing admission; sentinel/end bounds are included. Required device draw calls depth-control even for zero bias. |
| View/clear/state | Fixed native view arrays require no heap growth; validate identities/extents/order and do not mutate them at admission. View/touch are replay-only. |
| Resource pre/post command buffers | Reserve both existing prefixes plus resize, static-wrapper/FBO retirement, recursive shader/program/uniform destruction, delayed vertex-layout retirement and End/alignment margins. Native create/copy/upload remains pre-replay preparation. |
| UniformCache frame remap | Required full-draw uses draw-frequency uniforms, not cached view-frequency uniforms. Reject a nonempty cached-view baseline and view-frequency manifest category before mutation; prove the admitted empty remap reaches no node allocation or resize. Ordinary cached-view behavior remains unchanged. |
| Frame completion and next-frame begin | Reserve both current and next frame storage while synchronizing ownership of the completed render frame, without advancing/resetting it. Restore that synchronization ownership on canceled admission. Prevent admitted transition shrink/reallocation, include next encoder uniform begin and command End. |
| Debug text bookkeeping | Active debug-text rendering is unsupported/fail-closed; reserve the required next-frame text dimensions/metadata or establish exact existing capacity before replay. Do not disable validation/debug uniform checks globally. |
| Dynamic retirement, multiple encoders, unsupported commands | Reject dynamic-buffer pending retirement, single-threaded/concurrent encoders, compute, blit/readback, reset and unknown categories before accepted-target touch. Required full-draw uses static wrappers; no omitted required source route is accepted. |

`Context::swap` calls `freeAllHandles`, which can append a deferred
DestroyVertexLayout command, then swaps frames and starts the next encoder.
`Frame::adjustCapacity` may shrink/reallocate command or uniform storage even
after draw replay ends. Both transitions are included in the reservation,
not treated as worker-only work. `Frame::sort` and driver resource execution
are render-worker operations only for the explicitly admitted multithreaded
configuration. No API-thread allocation is reassigned to that category.

Before native edits, persist this inventory and review the immutable manifest
and unsupported-category controls. The extension is a reviewed public C++ API
on the pinned runtime; engine/tests include only public bgfx/bx headers.
Keep all native internals within their owning pinned implementation and keep
the complete durable patch/bootstrap/CMake identity checks synchronized.

## Surfaces, proof and acceptance

Scope a separately reviewed pinned runtime patch and public extension/header,
offline bootstrap/patch identity checks, CMake imported-runtime verification,
public dependency ledger/contract docs and isolated generated low-level
`test_bgfx_submission_reservation.cpp`. Preserve the existing shaderc patch,
source revisions/licenses, offline/no-fetch behavior and no-system-fallback
contract. Never hand-edit the pinned checkout without the corresponding
durable reviewed patch and exact bootstrap verification.

CPU controls prove budget arithmetic and public receipt/category behavior.
Physical two-generation controls cover maximal admitted varied bindings,
hash collision/equality, uniform stream/arrays, clear/views, resource retirement,
window resize/suspension and frame finish. Instrument API-thread native
allocator traffic: after successful reservation and before replay completion,
every supported replay path allocates zero times, including debug tracking.
Distinguish worker/driver allocations from the API-thread replay guarantee;
do not hide an API allocation in a worker classification. Inject each admission
storage allocation failure and all bounds+1/overlap/identity/category errors;
assert no view/touch/submit/present/resource-byte changes, clean retry and zero
residual. Guard against unplanned native frame advancement and preserve
ordinary commands queued before reservation. No journal is opened by this child.

Focused commands before edits:

`cmake --build build/<preset> --target renderer_bgfx_submission_reservation_tests -j4`

`ctest --test-dir build/<preset> -R '^(renderer_bgfx_submission_reservation|renderer_bgfx_transaction_resource|renderer_bgfx_device_contract)$' --output-on-failure`

Run both native/sanitizer focused configurations (`ASAN_OPTIONS=detect_leaks=0`),
exact serial host strict LSan with detect_leaks=1/no UBSan override selecting
the new reservation/resource CPU tests. Physical test
`renderer_bgfx_submission_reservation_gpu` uses the existing validation-clean
wrapper and graphical host escalation on both native toolchains, with required
established physical controls. Run all six complete builds/canonical nonretail
suites, serial host LAN 4/4 all six, bootstrap/patch/header/ledger/diff checks.
Do not declare acceptance if any supported commit replay can still allocate;
admission fails closed until the bounded native owner is proved complete.
Commit one child: `delivery: M22 08P0C2D0B2A reserve bounded bgfx submissions`.
