# M22 plan 01 slice 08P0C2D0B: public bgfx transaction aggregate

## Goal, dependency and boundary

After D0A and before B0/B, accept both idle-preparation and in-frame modes
on the Linux full-draw public bgfx route. B0/B delayed-source admission,
C2C preparation and C2D source draw
depend on this real capability, not Recording-only success. SDL and other
devices remain explicitly unsupported/fail-closed unless source runtime
reachability later requires them. No private Vulkan/bgfx cancellation API,
tree advancement/factory, retail input or broad backend rewrite. This file is
an evidence-only aggregate after [D0B1](milestone_22_plan_01_slice_08p0c2d0b1.md)
candidate/COW resource lifetime, [D0B2A](milestone_22_plan_01_slice_08p0c2d0b2a.md)
bounded public native reservation, [D0B2B](milestone_22_plan_01_slice_08p0c2d0b2b.md)
deferred frame journal and [D0B2](milestone_22_plan_01_slice_08p0c2d0b2.md)
reservation/journal aggregate. No production merge occurs in this aggregate.

## Admission, deferred commands and native commit

Current bgfx begin touches/clears a native view and consumes order; draw
validates/allocates wrappers then immediately submits; end only destroys the
framebuffer. In the opt-in transaction, retain immutable bounded source-order
command/resource data without native view touch/clear/submit/present until
commit. Validate the entire generation/attachment/load/initialization,
shader/viewport/clear/index/layout/count/resource and finite-capacity contract
before irreversible native commands. Prepare every fallible native wrapper
and resource first; publication after the native commit point cannot fail.
An independently required prerequisite gets a plan-only split before edits,
never a partial-frame exception or weakened test.

Abort before commit removes candidates and restores accepted shadow bytes,
initialization, view order and identities; clean retry emits once. Stale/
removed providers and capacity fail before mutation. The admitted immutable
batch uses only pinned public bgfx operations and owned references, no source
registry/state access or fallible resource creation after submit begins.
Release candidate/retired native resources in bounded completion-safe order.
Idle mode permits only admitted bounded resource/byte/marker preparation;
never allocate/touch/consume pass/view/viewport/clear/draw/present. Candidate
native resources and accepted shadow bytes retire/rollback without a frame.
Frame mode alone admits its complete ordered deferred pass sequence. Reject
cross-mode operations, mode changes, nested entry and existing active ordinary
passes before mutation. Preserve ordinary nontransaction passes and accepted
resource identities in both modes. A failed transaction cannot
mark targets accepted or present partial pixels; prior accepted contents stay
unchanged. Do not claim rollback after irreversible submission or disable
Khronos validation. Authorization is not applicable to generated tests.

## Surfaces, tests and acceptance

Expected surfaces: public bgfx transaction owner, narrow OriginalGpuEdge
capability admission/commit/abort, contract docs and generated renderer/
physical witnesses. Preserve the unrelated renderer edit by isolated paths/
hunks. Cover complete commit, partial command/resource and later staged-draw
faults, stale/removal, command/byte/view bounds+1, destruction/reset and retry
without duplicate draw/present/view consumption across two generations.
Physical Vulkan pixels prove accepted baseline survives rejected batches,
retry changes declared pixels once, exact clear/draw order and immediate
native/device resources. Use graphical host escalation and validation layers;
reject Validation Error/VUID categories, no emulation substitute. Persist
focused commands before edits. Run both toolchains/focused sanitizers, six
builds/canonical nonretail suites (sanitizer detect_leaks=0), exact serial host
LSan (`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), required physical
Vulkan and LAN 4/4 all six, ledger/diff. Commit one coherent slice:
`delivery: M22 08P0C2D0B revalidate bgfx transactions`.

Add independent idle-mode physical resource/byte controls, no-view/pass/draw
negative controls and prior accepted pixels on idle rejection; frame controls
cannot substitute for idle source-preparation admission or vice versa.

## Read-only native owner audit and resolved boundaries

The audited pinned public API is `build/bgfx-toolchain/source/bgfx/include/bgfx/bgfx.h`.
`copy`/`makeRef` return Memory consumed by a later bgfx operation; `release` is
private. D0B1 therefore does not accumulate unconsumed native Memory for abort.
It stages full uploaded mip bytes in bounded CPU storage and feeds native
candidate textures before any accepted native publication. Full ordinary mip
uploads retain their source bytes for later COW; unchanged known mips survive
replacement. No private free/cancel API or global allocator change is allowed.

Render-target extent is immutable in the public device descriptor. Resize is
a fresh candidate handle plus exact retirement, never an in-place descriptor
mutation. Accepted GPU-written color/depth/stencil pixels are not a CPU upload
shadow: transaction `upload_texture` to a render target rejects before any
mutation. The verified original TextureLoader/shroud route creates sampled,
non-render-target 2D textures, so this fail-closed transaction-only restriction
does not omit its required upload route. Ordinary nontransaction uploads stay
unchanged. Render-target creation/destruction remain journaled; attachment
writes belong only to the fully admitted D0B2B frame commit.

Native candidate destruction is queued through public bgfx destroy, without
frame advancement as an abort/reset shortcut. Preserve handles needed by any
frozen draw/FBO until their queued use is completion-safe; retire baseline
resources only after acceptance, in program/shader and FBO/texture-safe order.
Framebuffer attachments remain borrowed (`destroyTextures=false`), while
their FBO wrapper is transaction-owned. Idle admission/abort/commit must not
call frame, wait_idle, readback, claim/release window or mutate view state.
Reject these side routes during a live journal; device destruction first
cancels it and then follows the existing shipping completion/shutdown path.

Frame presentation is at most one final frame command. Query extent and
prepare presentation vertices/program/bindings before commit; suspension is
an admitted frame-only completion, not permission for idle advancement.
Resize records an immutable window/extent generation and is emitted only
after full admission. No callback, transient allocation, wrapper creation or
resource lookup remains after the first accepted-target touch. D0B2A reserves
the resource-command space for resize/retirement and frame completion too.

`EncoderImpl::submit` calls `bindStateIndexCached`, which can allocate TinySTL
hash nodes and lazy FrameArena blocks. `setUniform` can grow UniformBuffer;
resource destroy/resize can grow CommandBuffer. TinySTL's map has no public
reserve method. Public Init draw/uniform reservations do not cover these
paths, and the pinned Init code caps minimum resource command storage at
64 KiB. D0B2A is therefore a required narrow public runtime extension, not an
unresolved investigation gate in the journal plan. It proves allocation-free
API-thread replay within admitted bounds, including debug uniform tracking,
binding collision handling, and frame-completion paths. Native driver/device
loss and fatal process OOM remain the ordinary bgfx fatal boundary; they are
not reclassified as recoverable journal rejection. Every supported recoverable
allocation/provider/capacity failure precedes accepted-resource touch.

Post-D0B1 read-only allocation inventory is complete in D0B2A. Its narrow
public native primitive performs synchronous admit/reserve/replay of a complete
immutable manifest while holding API/frame ownership; no exposed reservation
token admits intervening native mutation. A owns native deep-copy/admission,
all reached encoder/depth/debug/frame/buffer reservation and allocation-free
replay. B owns device journal construction/order and generation consumption.
Worker/backend allocations retain the ordinary device-loss/error boundary and
physical Vulkan validation, never a claimed reversible API-thread result.
Cached view-frequency uniforms, active debug text, dynamic-buffer retirement,
single-threaded/concurrent encoders and unsupported command categories fail
closed before any accepted-target touch; ordinary behavior remains unchanged.

Reuse unchanged D0B2B final-source six-build/canonical, focused and strict
host LSan, physical Vulkan, serial LAN, ledger/diff evidence after reviewing
both idle and frame composition. Commit only aggregate artifacts.

Accepted after independent idle/frame ownership composition review; no
executable change. B2 aggregate `b4e0559b566e3d7570d0d351c549bd5e013b3448`
preserves B2B's unchanged final-source baseline and clean six canonical suites
274/274. Current B1 idle physical controls and B2B frame controls independently
pass on both native/both sanitized toolchains, not substituted by Recording.
[Aggregate evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2d0b_bgfx_transaction_aggregate.md)
records exact reference multiplicity, mode separation and native limitations.
D0 aggregate is next; source-stage/tree/factory/retail remain closed.
