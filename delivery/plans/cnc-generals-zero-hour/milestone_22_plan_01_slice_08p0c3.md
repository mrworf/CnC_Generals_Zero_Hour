# M22 plan 01 slice 08P0C3: optional projected tree decals

## Goal and boundary

Status: accepted; exact projected phase ownership and rollback pass final-source
focus11 all four, strict11 both, six builds/canonical279 all six, generated
physical all four, established Vulkan and identity-proved LAN4 all six.
Implementation commit is identified by its unique subject below; no self-SHA.
Transaction parent is C2 aggregate `5004485e354c8ae6b8b15ad688944b866c30aeaf`.
Persist this plan/index checkpoint before executable edits. Preserve the
unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic unstaged.

After C2, own native optional projected tree-decal resources and frame
queue/flush on the CPU path. The native projected-shadow source is not linked
there, and the existing bounded object-decal owner requires a `RenderObj`;
neither is a substitute for tree decal admission. Resource and queued immutable
intents are one owner, inseparable from the accepted tree phase. Preserve source
eligibility, terrain-conforming projection, bounded capacity, texture/resource
lifetime and multiplicative queue/flush before tree triangles. Keep the physical
`W3DTreeDraw` factory closed until C4. No additional prerequisite/split may be
introduced without an explicit architecture checkpoint.

Stage optional resources and frame commands without disturbing accepted C1/C2
owners. A failed create, queue, flush, removal or provider replacement clears
only candidate entries and Recording state in reverse order, retains accepted
tree identities and allows retry without reset. Authorization is not
applicable; fixtures are generated and asset-free.

## Resolved source semantics and atomic publication

Native `W3DTreeBuffer::drawTrees` queues visible instances whose type has
DoShadow before advancing topple/push/sink state. Exclude only TOPPLE_FALLING
and TOPPLE_DOWN; TOPPLE_FOGGED remains eligible. Capture eligibility and source
position in source-instance order during cull before C1 advances. Do not derive
the queue from the post-C1 geometry or turn an object-less tree into a RenderObj.
Shadow-disabled/no-eligible work preserves the accepted C2 path without optional
resource creation, RNG, effects or lazy texture access.

Create/admit/pin the exact default projected-shadow resource (`shadow.tga`,
clamp U/V, no mip filtering) entirely before C1 publication. Explicitly initialize
reached CPU object-less shadow linkage/robj state; leave Windows declarations,
serialized state and source ABI unchanged. Shared source ownership and each
immutable phase pin have exact release units; candidate failure, reset/removal,
phase cancellation and successful completion retire exactly once. Do not enable
unrelated dynamic/projected/alpha/additive object-shadow modes.

Native tree size is model root bound-box Extent.X + Extent.Y, not instance scale;
the queue sets size X to that value and size Y to its negative. Preserve the
authored terrain grid, border, floor/ceil rectangle, per-axis104-vertex clipping,
height offset and native flipped/non-flipped triangle winding. Use the current
map draw origin/width clipped to extent directly for this unlisted tree queue;
never depend on `renderShadows` having a nonempty object-shadow list to update
global rectangle state. Reject nonfinite/invalid geometry and nonrepresentable
indices before publication. Retain native streaming batch limits32768 vertices,
65536 indices,16-bit indexing; admitted work must also fit existing phase/device
command/resource/byte bounds, with bound+1 rejecting without mutation.

Fully prepare the immutable source XYZDUV1, PRELIT_DIFFUSE, identity-world,
default-texture and `_PresetMultiplicativeShader` state before C1 publication.
Generic alpha object decals or a RenderObj quad cannot substitute. Publish queue,
resource, rectangle and phase atomically with C1 only after complete admission.
After C1/FX, use only phase-owned pins/state; no registry lookup or fallible lazy
resource preparation. At the accepted DoTrees slot, flush optional multiplicative
decal batches immediately before tree triangles, after the pre-tree mesh/shader
flush and before the later stencil/water/particle families.

Extend the existing C2D source/device checkpoint to preserve exact queue count,
order, identities/bytes, projected resource and draw rectangle. Every begin,
pass/stage/draw/end/present/commit failure aborts the frame and restores that exact
baseline, retaining the same immutable phase. Clean retry cannot advance C1,
consume RNG or replay FX; successful completion/cancel are the only phase-release
points. FX-triggered removal/reset cancels using already-owned phase state.

## Implementation surfaces and exact focused commands

Expected surfaces are CPU BaseHeightMap cull/prepublication and source DoTrees,
its immutable phase owner, the narrowly scoped CPU projected-shadow provider,
OriginalGpuEdge immutable resource/program/draw preparation and frame checkpoint,
generated full-probe fixture/wrapper, CMake registrations and dependency ledger.
Retain all accepted full-packet preparation controls and shadow-disabled draw
controls; generated default texture and nonflat map provide asset-free evidence.

`cmake --build build/<preset> --target zh_original_w3d_full_probe original_w3d_cpu_graph_tests original_w3d_tree_program_tests original_w3d_source_reference_tests original_w3d_stage_transaction_tests renderer_recording_transaction_tests renderer_bgfx_transaction_resource_tests renderer_bgfx_transaction_tests -j4`

`ctest --test-dir build/<preset> -R '^(original_w3d_tree_decal|original_w3d_tree_draw|original_w3d_tree_preparation|original_w3d_tree_program|original_w3d_terrain_shroud_projection|original_w3d_terrain_map_frame|original_w3d_source_reference|original_w3d_stage_transaction|renderer_recording_transaction|renderer_bgfx_transaction_resource|renderer_bgfx_transaction)$' --output-on-failure`

Run this exact eleven-control union on GCC/Clang debug and both sanitizers,
sanitizer focus with `ASAN_OPTIONS=detect_leaks=0`. Strict host LSan uses the same
eleven with exactly `ASAN_OPTIONS=detect_leaks=1`, no UBSan override.
Generated physical route:
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation ASAN_OPTIONS=detect_leaks=0 python3 tests/original_rendering/test_w3d_tree_decal.py --source-root . --executable build/<preset>/zh_original_w3d_full_probe --asset-producer build/<preset>/original_w3d_cpu_graph_tests --gpu`.
Run all four configurations with the established host graphical environment and
escalation; retain generated tree/program/shroud and established Vulkan controls.
Audit complete logs, not exit codes.

## Validation and commit

Generated fixtures cover shadow-disabled equivalence, visible/hidden/DoShadow,
upright/FALLING/DOWN/FOGGED and pre-topple eligibility, optional texture admission,
nonflat terrain projection with exact size/negative-Y/UV/winding/clipping/border,
unlisted-only current-map rectangle, multiplicative physical pixels and ordering
before tree triangles. Cover batch/phase bounds and bound+1, missing/stale/wrong
providers, shared ownership/alias multiplicity, two generations, reset/removal
and exact cancellation. Inject each resource, queue/prepublication and flush/frame
boundary; assert immediate residuals, unchanged prior pixels/commands and exact
same-phase retry without C1/RNG/FX re-advance. Run six
complete builds and canonical nonretail suites, focused strict host LSan,
physical Vulkan controls, serial LAN 4/4 all six, ledger and diff checks on
final source. Commit one slice:
`delivery: M22 08P0C3 own projected tree decals`.

Implementation and final gate provenance are recorded in
`delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c3_projected_tree_decals.md`.
The generated physical baseline uses explicit native afternoon ambient lighting
and a bright opaque exact-name Flat.tga; inherited dark/alpha4 fixture pixels
cannot provide a meaningful multiplicative comparison. The shipping source
lighting and exact decal state are unchanged. CPU/editor flip/origin setters are
not linked; the generated map-state adapter exercises those native fields
without introducing a production API or changing source ABI.
The C3 wrapper alone uses90s/process: isolated sanitizer full generations take
53.62s GCC and49.62s Clang. This bounded workload correction retains all
assertions/contents and TimeoutExpired failure; no global timeout changes.

Final coverage review found that uniform maximal patches hit the64MiB limit
before256 batches. The registered witness now alternates104-by-104 and21-by-21
patches:256 batches occupy51,491,328 bytes, while257 rejects without changing
accepted storage, rectangle, ordered intents or any vertex/index byte, source
checkpoint, RNG or GPU state. No production behavior changes. Refresh all six
builds, exact focus/strict/physical gates and six canonicals on this final test
hash. Reuse prior LAN4/4 only after executable, wrapper/interpreter and selected
CTest command/input identity is proved byte-for-byte.
