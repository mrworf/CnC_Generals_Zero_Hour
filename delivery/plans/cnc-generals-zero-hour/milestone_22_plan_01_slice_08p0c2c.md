# M22 plan 01 slice 08P0C2C: immutable outside-frame tree preparation

Status: implementation-ready refinement after accepted C2B `749067b`;
plan-only checkpoint precedes all production edits. No new prerequisite/slice.
Transaction parent is the accepted C2B payload; the governing index's unique
plan commit subject identifies this checkpoint in Git history.

## Goal, dependency and boundary

After C2A program, D0's idle/frame capabilities, B0's complete delayed-source
transaction and C2B shroud binding,
prepare one exact-owner immutable tree
render phase outside an active source frame. C1's resource retirement/update
requires `source_buffers_retirable()`, while native DoTrees is inside Flush.
Preparation must bridge that real lifetime boundary, not call C1 during draw.
C2D owns in-frame scene integration; no triangles, projected decals, physical
factory or retail admission here. Authorization is not applicable.

## Accepted phase and retry state

Audit source display/view update → Begin_Render → scene/Flush → End_Render.
Select the shipping preparation hook at the existing source boundary, after
the accepted camera/shroud update and before Begin_Render. Prepare exact
generation/epoch, camera, program/stage/constants, accepted atlas/VB/IB and
counts; acquire only necessary references. Admit the exact backend's bounded
frame transaction before C1 acceptance; unsupported devices reject without
tree/RNG/FX mutation. All fallible new preparation work
must finish before irreversible C1/FX acceptance. Do not invent another scene
or registry; retain original source owners and the C1 transaction.

After C1 state/geometry/RNG/FX commits, the prepared phase is immutable and
owns an explicit retry identity until draw succeeds or is canceled. In-frame
render failure cannot undo immediate effects: retain the accepted phase and
retry exactly its buffers/constants without advancing sway, collision,
topple/sink, GameClient RNG or FX. Do not use a gameplay-frame number alone as
a render-attempt identity. A successful completion allows exactly the next
source preparation; pause retains source behavior. Hidden/zero-tree states
are explicit, not successful missing-provider substitutions.

Preparation failure leaves prior accepted phase/resources/tree identity/RNG
and pending FX intact. Reset/removal/epoch change cancels a pending phase
safely; stale device/owner references reject without crossing generations.
References release once and reverse, outside active source frame retirement.
The approved existing-owner phase representation and admission ordering below
resolve this investigation. Scope freeze prohibits any further independent
owner or split without an explicit architecture checkpoint.

## Resolved lifecycle and atomic candidate composition

The shipping hook is `W3DDisplay::draw`, after existing shroud content render
and `updateViews`, before `WW3D::Begin_Render`. Camera preparation must use
the actual camera's native D3D projection/view algebra outside a pass rather
than reading stale global transforms from the previous frame or calling the
in-pass `CameraClass::Apply`. Preserve C2B's content and stage1 authored
semantics; tree program's subsequent UV-index0/1 and transform-disable
requirements belong to the immutable tree snapshot, not to premature global
source-stage mutation. No B0/frame-journal overlap is required or permitted.

Existing C1 `updateTreeVisibleFrame` commits candidate registry/geometry/RNG,
retires replaced resources, then dispatches admitted FX. FX may reset/remove
the terrain, so calling capture/program preparation after C1 returns cannot
be atomic. Build and fully admit the immutable candidate during C1's
prepublication boundary: exact candidate sway/constants/vertex/index/atlas,
accepted shroud content identity/epoch, camera and render-target formats,
source tree shader/stage/filter/material semantics, exact ranges and bounded
program/uniform resources. All allocation, validation, upload, ref/lease
acquisition and failure injection complete before C1 publication or any FX.
Integrate this with the existing terrain registry/Edge ownership, not a new
provider, proxy scene, factory or test-only shipping route.

Extend the existing Edge preparation to consume a complete admitted immutable
snapshot and return phase-owned resources, independent of mutable global
`physical_`, source revision and pending stages. Ordinary generic preparation
must retain its current contract and cannot invalidate the phase. Exact
resource pins survive replacement/old mapping retirement; source address or
reused native/logical handle equality alone is insufficient. Candidate failure
releases only candidate units, preserves prior accepted phase/program/resource
identity and pending effects, and retries without source reset.

With the candidate fully built, perform a real bounded frame transaction
begin/abort probe against the exact backend, target generation and declared
command/resource/byte/view bounds before C1 publication. Rejection or abort
failure cannot advance C1/RNG/FX. The successful probe proves compatible
bounded admission at preparation time, not future availability. Never retain
a live device journal across C1 publication/old-resource retirement: abort
could otherwise restore old native units whose Edge mappings were retired.
C2D opens its actual draw journal later; begin failure preserves this same
accepted immutable phase for retry. SDL/other unsupported devices fail closed.

Publish an already-owned phase/lifecycle object atomically with C1 through
no-throw assignments; no allocation/lookup/validation remains after that
point. A local owned lifecycle reference survives possible FX-triggered
registry erasure. Reset/removal cancels through that already-owned lifecycle
state; no registry access or owner dereference follows FX dispatch. Successful
draw/completion or explicit lifecycle cancellation are the only phase release
points, exactly once and outside active-pass retirement. Pending phase reuse
cannot call C1, query mutable simulation state again, consume RNG, replay FX,
replace immutable constants/buffers or change its monotonic retry identity.
Identity exhaustion rejects before mutation; stale generation/epoch/owner
rejects or cancels without crossing generations. No gameplay frame number is
used as the sole identity. Explicit empty/hidden phase handling preserves
source pause/cull behavior without substituting missing providers.

## Exact focused command contract

Add a narrow generated public-display-route fixture
`original_w3d_tree_preparation` using the existing full probe, generated rigid
tree asset producer and logical/visual map providers; do not expand the heavy
64-type/4000-instance fixture merely to gain this coverage.

`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_tree_program_tests original_w3d_source_reference_tests original_w3d_stage_transaction_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_tree_preparation|original_w3d_tree_program|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_source_reference|original_w3d_stage_transaction|renderer_recording_transaction|renderer_bgfx_transaction_resource)$' --output-on-failure`

Run this exact eight-control set on GCC/Clang native and both sanitizers;
strict host LSan selects the same eight with `ASAN_OPTIONS=detect_leaks=1`,
no UBSan override. The new fixture proves real public preparation/no triangles,
immutable same-phase retry after later global generic stage/program mutation,
next-success progression, source pause/hidden/empty, all candidate shader/
pipeline/uniform/create/upload/pin/publication and probe-admission faults,
FX-triggered terrain removal, reset/epoch/stale generation, exact resource
counts and no C1/RNG/FX re-advance across two generations. Retain all existing
fixture assertions. Physical tree/shroud and transaction controls remain the
established exact host Vulkan selectors; no new in-frame draw is admitted.

## Surfaces and acceptance

Expected surfaces: CPU terrain exact-owner state, source display/view lifecycle
hook, source program/shroud admission and generated public-route witness;
native Windows/retail object layout remains unchanged. Keep C2D scene/draw and
C3/C4 admissions closed. Tests prove public source preparation, immutable
same-phase retry/no C1/RNG/FX advance, next-success progression, pause/hidden,
empty state, reset/removal, stale generation, all preparation faults and
two-generation clean residuals. No test-only shipping hook or forced phase.
Persist focused commands before implementation. Run both toolchains/focused
sanitizers and six complete builds/canonical suites (`-LE 'gpu|lan|retail'`,
sanitizer `ASAN_OPTIONS=detect_leaks=0`), exact serial strict host LSan
(`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), established physical Vulkan,
serial LAN 4/4 all six, ledger and diff checks. Commit one coherent slice:
`delivery: M22 08P0C2C prepare immutable tree render phase`.
