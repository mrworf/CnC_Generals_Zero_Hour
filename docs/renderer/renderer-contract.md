# Renderer closure contract

M2 closes the renderer at the source/API boundary without creating a window or graphics device. `docs/renderer/legacy-api-mapping.tsv` classifies every renderer identifier observed in the in-scope legacy C/C++ source. `tools/renderer_inventory.py --check` is fail-closed: an unclassified or multiply classified identifier breaks the `renderer-contract` test label.

Engine-facing code uses the opaque handles and descriptors in `include/zh/renderer`. The internal `GpuDevice` interface is the sole submission boundary shared by the later recording and real implementations. Public headers contain no platform graphics types. Pipeline identity includes shaders, vertex layout, primitive topology, blend/color mask, depth/stencil, raster state, target formats, point-size behavior, premultiplied-alpha behavior, and fog. Keys are immutable after construction and support bounded cache lookup.

The contract permits at most four uniform buffers per shader stage. Blocks are explicitly 16-byte aligned and separated into frame, material, object, and effect data. Texture, sampler, render-target, upload, point-list, and pass limits fail before backend submission. Two-dimensional, cube, and three-dimensional textures are explicit. BC1/BC2/BC3 use the backend format only after its public support query; otherwise the caller decodes to RGBA8 on the CPU. BC2/BC3 mappings preserve the premultiplied-alpha distinction inherited from DXT2/4.

Coordinates are left-handed, depth is 0 through 1, clockwise vertices are front-facing, texture origin is top-left, UI positions retain the negative half-pixel offset, and packed colors are ARGB8. Fog distance is view-space. Resize is an explicit `stable -> requested -> recreating -> stable` transition; zero extents suspend presentation, failed recreation remains pending, and successful recreation advances a generation.

The checked shader registry contains UI, terrain, water, points/particles, and WWShade families. Each has explicit vertex and fragment GLSL modules compiled offline to SPIR-V. The points family writes point size; water covers projected render-target sampling; WWShade demonstrates multipass texture/projected/fog state within the four-uniform limit.

M29 additionally packages all 39 owned renderer/effects and original-FVF variants as pinned public bgfx Vulkan shader containers. The build-only, fail-closed GLSL lowering keeps the original sources authoritative while mapping up to four source blocks into one std140 set-0 stage UBO and source texture slots into bgfx's separate image/sampler binding pairs. Per-variant JSON manifests retain source block, slot and stage names for M30. The offline verifier checks complete family inventory, stage/envelope/SPIR-V validity, source-to-descriptor mapping, reflection and vertex attributes; `world.frag`'s three explicitly zero-multiplied texture placeholders are the only checked dead-descriptor exception. This is package/CPU evidence, not physical shader programs or pixels.

M2 provisionally selected SDL_GPU as the implementation backend and did not claim device support, synchronization correctness, pixels, or performance. Historical M14 acceptance covered that backend on the local Vulkan-capable x86-64 system. The later demonstrated viewport-clear gap selected the documented bgfx branch; M30 owns new physical acceptance. No private Vulkan escape hatch is permitted.

The [2026-09-21 renderer backend decision](../zero-hour-renderer-backend-migration.md) selects bgfx for the device edge after M22 found and reproduced the camera-viewport clear gap. This does not retroactively reaccept the M2/M7–M10/M14 SDL_GPU-specific mappings or pixels. Their affected evidence is revalidated through the migration gate; this contract's engine-facing opacity and source-behavior invariants remain in force.

M7 implements the GPU-independent execution contract through `RecordingGpuDevice`. Its generation-checked opaque resources, bounded immutable pipeline cache, CPU-backed dynamic uploads, pass validation, and normalized ordered snapshots make lifetime and command errors testable without a display or graphics device. DDS DXT1/2/3/4/5 and uncompressed true-color TGA inputs use fixed-width bounded parsing; DXT2/4 retain premultiplied-alpha semantics, and BC1/2/3 decode to RGBA8 when the public backend capability says the compressed format is unavailable.

`Dx8StateCache` is the temporary legacy `DX8Wrapper` boundary. It converts cached engine state into immutable `PipelineKey` instances, suppresses redundant pipeline creation, propagates recorder failures with facade context, and records explicit resize transitions. It remains backend-neutral: engine callers and public headers may not expose or call SDL_GPU or Vulkan. Hardware submission, synchronization, presentation, and pixel acceptance remain M14 work.

M22 adds the minimal original W3D indexed-draw capability to `DrawDesc`:
`index_element_size` defaults to the existing 32-bit behavior, while an
indexed draw may specify 16-bit elements, `first_index` and signed
`base_vertex`. Original `DX8PolygonRendererClass::Render` supplies those
source values from its shared category buffers; the recording backend
validates format/range and SDL_GPU binds the selected public index width
and draw offsets. Non-indexed draws reject index-only fields. This does not
rebase or repack original mesh geometry and adds no platform graphics type
to the public interface. M22 slice 05A verifies hardware parity with an
owned indexed fixture; retail-scene rendering remains later acceptance.

M22 slice 06C3B2 adds `RenderPassDesc::color_load`/`depth_load` and exact
`clear_color` RGBA (including destination alpha) / `clear_depth` to the
public pass boundary. The legacy default remains clear color
`(0.02, 0.02, 0.04, 1)` and depth `1`; existing callers retain their
behavior. `LOAD` requires a completed prior pass or whole-target upload
on that same live texture generation, fails before acquiring a GPU pass on
uninitialized attachments, and retains stored color/depth independently.
Color, alpha and depth clear values must be finite in `[0,1]`; SDL_GPU
attachment cycle is disabled for `LOAD` per the public SDL header contract.
Clears occur at full-target pass begin, never as a silent replacement for
the original scene's camera-viewport clear or while a pass is active. B3
will connect original `WW3D::Begin_Render`/`End_Render`; C3C owns the
source scene-scoped clear. Physical presentation requires a completed
color target, a claimed window for SDL_GPU, and no active pass.

M29 adds `ViewportClearDesc` as a separate, ordered device operation while a
pass is active. It carries the live color/depth handles, owner target
generation, signed origin and unsigned extent, and independent color, depth
and stencil flags/values. The recorder clips a partially overlapping rectangle
to the target, rejects a disjoint/empty rectangle and stale attachment or
generation, and preserves draw/clear/draw order without converting a clear to
a proxy draw. Selected colors and depth must be finite in `[0,1]`; stencil
requires a D24S8 attachment. A flagless clear is invalid. Full-target pass
clear/load remains the default and a LOAD of an uninitialized attachment still
fails. Accepted pass, draw and viewport-clear commands consume a bounded
ordered-view budget, reset only by successful presentation. SDL_GPU returns an
explicit unsupported error for the new operation; physical bgfx submission is
M30-owned, not implied by the recording contract.

The source-facing `OriginalGpuEdge` retains the current successful camera
viewport during a source frame, converts a selected original clear to the
ordered descriptor with current frame-target generation, and propagates
unsupported-device/invalid-state errors. This translation is exercised with
owned CPU fixtures; M22 still owns connecting the complete original
`WW3D::Render` scene producer and retail pixels. The [migration ledger](bgfx-migration-ledger.md)
records affected source-category and evidence ownership.

M22 slice 08P0C2D0A adds opt-in bounded device transactions. Existing consumers
retain unsupported defaults. `idle_preparation` admits resources, payload bytes
and markers only; frame pass/view/viewport/clear/draw/present operations reject
before mutation. `frame_commands` admits those operations from an idle device,
but cannot commit with a pass still active. Tokens bind device, sequence,
caller generation and mode; nested admission and stale/cross-owner finish
reject. Finish is no-throw, allocation-free and exactly once.

Recording admission bounds/copies existing slot tables, labels, payloads and
last-index bytes before publishing its checkpoint. Limits are 4096 commands,
4096 total checkpoint/candidate slots, 64 MiB checkpoint/operation payload
bytes and 256 views. Admission failure leaves the output token and observable
device state unchanged. Partial commands stay hidden until commit. Abort
restores prior bytes, initialization, identities, target/view/draw state and
command prefix, retaining dead candidate slots with retired generations so
retry cannot alias aborted handles. Consumed faults remain consumed; fixed
failure diagnostics are separate from the successful journal. Any failed
ordinary journal operation or its exception poisons commit while preserving
abort; rejected admission/foreign finish cannot mutate an existing owner. Ordinary
non-transactional behavior remains unchanged. Physical transaction support
belongs to D0B; no other backend support is implied by this Recording witness.

M22 D0B1 implements bgfx `idle_preparation` only; frame capability stays
unsupported until the native reservation/deferred-journal children. Admission
bounds five slot-table checkpoints, source mip shadows, candidate native
versions and pending retirements (4096 commands/resources, 64 MiB, zero views).
Resource metadata reservations and duplicate checkpoint payloads consume the
byte budget. Rejected entry preserves the output token and observable state.
Sampled non-target texture uploads COW complete known mip bytes into native
candidates; commit keeps the accepted opaque handle, abort preserves its
native bytes. RGBA8, BGRA8 and BGR5A1 retain full padded source rows and untouched
mips. Transaction uploads to GPU-owned render targets reject; target resize is
a fresh handle, not in-place descriptor mutation. Ordinary uploads stay valid.

Finish performs no native call or allocation: it publishes/restores CPU state
and transfers exact native ownership units into a pre-reserved retirement
queue. Deduplicated shader/program indices do not collapse create increments;
one unit is kept per surviving slot and every excess unit retires exactly once.
Pending units remain owned and count toward admission. An ordinary boundary
drains public native destruction; ordinary wait/shutdown completes it, never a
transaction abort/reset shortcut. Pass/view/clear/draw/present, window changes,
wait and readback reject during idle admission before mutation. Guards poison
before diagnostic allocation, and consumed faults are not restored. Aborted
candidate generations cannot alias retry; source refs/maps and frame rollback
remain separate later owners. No other backend or source/factory claim follows.

M22 D0B2A adds a reviewed opt-in public native `bgfx::submitBounded` primitive;
it does not yet enable the device's frame transaction capability. One synchronous
call snapshots the complete immutable command/payload slab, validates ordered
native lifetimes, reserves all API-thread storage, then replays without allocation.
Failure before replay restores completion-semaphore ownership, leaves commands,
views, accepted resources/pixels and caller receipt unchanged, and advances no
frame. Only success publishes context/sequence/generation and frame receipts.
Native indices/refcounts are validated here; device logical generations and
at-most-once journal consumption belong to D0B2B.

Bounds are 4096 commands/draws/resource mutation operations, 256 unique declared
views and 64 MiB checked admission storage. Supported categories are view/touch,
static indexed/nonindexed graphics draws, draw-frequency uniforms, sampled texture
bindings, exact live window resize, static resource retirement and optional
last-only frame completion. Full binding equality preserves distinct hash-colliding
values. Complete encoded-field/table extents and unknown gap bits are checked
before admission allocation, including resolved default sampler flags, both
stencil faces and independent blend target encodings. Ordinary queued work remains
intact. Source suspension uses frame completion
only; zero-size resize and changed native window identity or formats reject.
Debug text, profiling/custom callbacks, cached view uniforms, pending dynamic
retirement, single-thread rendering and overlapping encoders are unsupported.
Required next-frame bookkeeping must have exact fixed capacity. Workers/backends
retain ordinary device-loss/fatal behavior outside the reversible API-thread
boundary. SDL remains unsupported; source/tree/factory admission remains closed
until its dependency-ordered owners are accepted.
