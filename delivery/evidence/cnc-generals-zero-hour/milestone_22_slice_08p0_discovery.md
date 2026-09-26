# M22 slice 08P0 discovery: original tree draw provider

After accepted 08N0, a corrected read-only retail continuation crossed all
six fixed post-map phases and entered ordinary authored object construction.
Ten `W3DModelDraw::onObjectCreated` entries and exits balanced. The first
failing edge was `ModuleFactory::newModule`: draw-provider type 1, public
schema-only provider ordinal 8 of nine, `W3DTreeDraw`. The factory correctly
throws `ERROR_INVALID_D3D` for its null physical create proc. No Recording
scene frame was published; teardown completed. No active bib producer was
reached. Only this fixed source category is retained; no retail selector,
root, logical name, byte, hash, raw output, stack or label is retained.

The source graph is not a one-line registration change. Full-instance
`W3DModuleFactory` retains a schema-only `W3DTreeDrawModuleData` parser and
null physical proc. `W3DTreeDraw` itself has an empty constructor and draw
method; its first transform sets `m_treeAdded` before asking
`TheTerrainRenderObject->addTree`, and its destructor does not call
`removeTree`. The accepted CPU-only `BaseHeightMapRenderObjClass` initializes
`m_treeBuffer=NULL`, while the Windows tree buffer constructor allocates raw
Direct3D resources. Source tree type admission resolves a model mesh, bounds
and texture; the buffer owns bounded tree/type/partition arrays, atlas tiles,
vertex/index buffers and a projected-shadow pointer. Terrain pass drawing
performs cull, sway/topple updates, atlas/shroud/lighting binding, triangles
and optional decal queue/flush. Native `addTree` returns `void`; full type
capacity and missing/non-mesh asset branches return ambiguous `0`. Enabling
the create proc now would silently omit or retain trees and could cross raw
device calls.

The dependency closure is separated by real owners: 08P0A atomically owns
tree type/instance and terrain lifetime without a frame; 08P0B prepares
model/atlas/mesh GPU-edge resources without factory admission; 08P0C records
the source tree pass and only then admits the physical module. 08P0 is the
generated aggregate. Retail selector/borrow-unwind belongs to slice 08.
All trial production wiring and diagnostics were removed before this
plan-only checkpoint; unrelated renderer diagnostic remains unstaged.

## Post-C1 C2 source-owner audit

Accepted C1 ends at committed source tree geometry/state and consumed optional
FX, not terrain triangles. Canonical `Shaders/Trees.nvv` interprets XYZNDUV1
normal slots as sway-index/darkening/base-Z, uses height-relative displacement,
scales diffuse, retains atlas UV and derives shroud UV from unswayed position.
Generic world/lighting shaders are incompatible. `Trees.nvp` is unused and its
install is under `#if 0`. Native tree draw supplies c4–7, c8–18, c32/c33 and
detail-alpha/depth state. Linux `setShroudTex` is absent; its native owner
separately binds exact terrain shroud, camera-space inverse-view/origin/scale,
color MODULATE/alpha SELECTARG2 before the tree program's UV/disable sequence.

C1 update requires source_buffers_retirable outside an active frame. Native
DoTrees is in Flush after mesh/occluded/shader work and before stencil shadow,
static/water and final particles. Linux source display/view Begin/End and
current Customized_Render ordering need an explicit immutable preparation
phase so render retry cannot advance C1 or replay irreversible FX.

Read-only rollback audit finds no public checkpoint. Recording begin/draw/end
append commands and mutate view/index/target state; edge abort merely ends a
pass. bgfx begins with view touch/clear and submits each draw immediately;
end releases framebuffer only. SDL end submits its command buffer. Those are
not command rollback. Dependency-first D0A owns bounded optional public
capability and deterministic Recording checkpoint; D0B owns real deferred
public-bgfx transactions, all fallible admission/resources before native
commit. Unsupported SDL/other devices remain fail-closed. Both capabilities
precede outside-frame preparation and inside-frame draw. Preserve ordinary
pass behavior, no private cancellation or fixture mutation, and prove partial
commands/resources removed with retry-emits-once and physical validation.

The persisted plan-only order is C2A→C2B→D0A→D0B→D0 aggregate→C2C→C2D→C2
aggregate. C3 optional tree decals, C4 factory and 08/09 retail/visual acceptance
remain unchanged and closed. No production edits or retail probing occurred
in this refinement; only public source facts are recorded.

## C2A trial localization and C2A0 prerequisite

Canonical tree load/update retains encoded sway type across topple; only slot0
selects zero c8, while1–10 select sampled waves. The uncommitted A trial's
native two-generation owner/Recording/fault/retry controls passed. Its physical
standalone initially omitted shipping process-service synchronization before
bgfx workers; adding that real fixture precondition removed allocator/thread
corruption without allocator edits. Exact untextured physical sway, darkening,
alpha and composite controls then passed. These are localization, not final A
acceptance, and no retail input was used.

The textured control exposed a distinct public-bgfx origin defect. Compiled
original_applied_3.frag reflects stage_ops128, stage_args160, alpha96, fog80/64
in a block beginning0. Runtime subtracts the first live field64 instead of the
actual block origin, reading stage control from the wrong bytes. The generated
physical symptom was338 accepted pixels with constant green100:100, no shroud
variation; the tree vertex block has first live field0 and is unaffected.
Persist C2A0→A before backend correction, remove all A-owned trial source/test
edits while preserving plans/investigation facts and the unrelated renderer
diagnostic, accept exact dead-prefix/multiblock binding independently, then
reimplement and revalidate A. No passing prior control waives this defect.

C2A0 is independently accepted with compiler-authoritative block origins,
strict absent-versus-foreign Uniform admission, unchanged source payloads,
two-generation physical dead-prefix/multiblock controls and all six canonical
suites 268/268. [Exact evidence](milestone_22_slice_08p0c2a0.md) records the
refrozen results; the earlier A trial remains localization only. Reimplement
C2A against this owner before admitting exact tree shader semantics.
