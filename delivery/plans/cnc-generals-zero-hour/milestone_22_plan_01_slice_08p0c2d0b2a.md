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

Introduce explicit finite native budgets and an exact context/frame/sequence
token, single API-thread owner, no nested/concurrent encoder overlap. Admit
only supported native command categories before any accepted-target touch.
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
to the current native frame and token; reject stale/wrong-owner/duplicate finish
and misuse before mutation. Cancel reservation before replay restores only
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

## Surfaces, proof and acceptance

Scope a separately reviewed pinned runtime patch and public extension/header,
offline bootstrap/patch identity checks, CMake imported-runtime verification,
public dependency ledger/contract docs and isolated generated low-level
`test_bgfx_submission_reservation.cpp`. Preserve the existing shaderc patch,
source revisions/licenses, offline/no-fetch behavior and no-system-fallback
contract. Never hand-edit the pinned checkout without the corresponding
durable reviewed patch and exact bootstrap verification.

CPU controls prove budget arithmetic and public token/category behavior.
Physical two-generation controls cover maximal admitted varied bindings,
hash collision/equality, uniform stream/arrays, clear/views, resource retirement,
window resize/suspension and frame finish. Instrument API-thread native
allocator traffic: after successful reservation and before replay completion,
every supported replay path allocates zero times, including debug tracking.
Distinguish worker/driver allocations from the API-thread replay guarantee;
do not hide an API allocation in a worker classification. Inject each admission
storage allocation failure and all bounds+1/overlap/stale/category errors;
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
