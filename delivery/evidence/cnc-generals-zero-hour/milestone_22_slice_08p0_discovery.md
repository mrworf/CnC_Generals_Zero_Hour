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

C2A is now independently accepted against C2A0: exact source packed slots,
272-byte constant record and tree-only generated UV transport retain generic
rejection. Native toppled nonzero sway selection and physical nontrivial atlas/
position-varying unswayed shroud controls pass, with six canonical suites
269/269 and focused/strict LSan/host gates. [Exact evidence](milestone_22_slice_08p0c2a.md)
does not admit source shroud binding, scene draw or physical factory/retail.

Post-A read-only shroud admission audit confirms an additional complete owner:
CPU DX8 texture refs/dirty bits change before fallible markers; source stage
maps/transforms can allocate, edge revision advances before recording, and
delayed filtering can replace the accepted sampler. SourceStateSnapshot lacks
refs/pending edge state; RenderStateStruct lacks stage maps/texture transforms
and its ordinary-setter restore is fallible. Neither is rollback.

Persist the dependency-safe reorder A→D0A→D0B→D0 aggregate→B0→B→C→D before
production. D0 supplies explicit bounded idle preparation and in-frame modes
on Recording and bgfx; idle mode forbids pass/view/viewport/clear/draw/present,
SDL/other devices fail closed. B0 composes device journals with complete source
texture/ref/maps/transform/dirty/revision/filter/sampler ownership and retry;
B supplies only exact native shroud semantics. No trial wiring, private input
or production edit is included in this plan-only checkpoint.
Checkpoint against accepted A commit `8df9c18b34bef2e23613b8655bccfd8b97555aef`
by `delivery: M22 order idle and frame transaction prerequisites`.

D0A is independently accepted: optional bounded idle/frame capability,
Recording checkpoint/commit/rollback, exact token ownership, retired candidate
handles and monotonic consumed faults. Attached-target diagnostic allocation
is poisoned before formatting and explicitly fault-tested. Refrozen complete
builds/canonical suites pass 270/270 all six, focused 3/3 in native/sanitizer
toolchains, strict host LSan 2/2 both, established physical controls and serial
LAN 4/4 all six. [Exact evidence](milestone_22_slice_08p0c2d0a_recording_transaction.md)
preserves physical/source/factory closure; D0B is dependency-next.

D0B1 is independently accepted: bounded idle bgfx candidate/COW resource
publication with exact per-create native ownership units, delayed ordinary
retirement and no native finish/frame calls. Refrozen rejected nested/foreign
admission preserves the original owner; two-generation physical controls
cover aliases, formats/mips, partial allocation/publication failure and retry.
Complete builds/canonical suites pass 271/271 all six, strict host LSan 2/2
both, generated native/sanitizer Vulkan controls, established physical checks
and serial LAN 4/4 all six. [Exact evidence](milestone_22_slice_08p0c2d0b1_bgfx_resource_transaction.md)
keeps frame/source/factory closed. D0B2A's reviewed bounded public native
submission reservation precedes D0B2B's complete deferred frame journal.

D0B2A implementation freezes one synchronous public immutable native manifest,
ordered lifetime validation, checked storage reservation and allocation-free
API-thread replay. Final generated two-generation Release and BX debug-enabled
physical proofs pass exact collision/full-equality sampling, ordinary prefix,
immutable payload snapshot, late rejection pixel preservation, maximal uniform
and arena extents, exact native window resize, frame-only source suspension and
native ownership multiplicity. Unpublished nontrivial UniformBuffer candidates
use explicit constructor/destructor ownership; no global allocator change is made.
These are focused checkpoint results, not final acceptance. Six complete builds,
canonical suites and host gates must finish before the child is marked accepted.

The final read-only encoded-state audit found reserved-bit checks alone did not
exclude native table bound+1 values. A's whole-manifest correction validates all
state/sampler gap bits and encoded depth/blend/equation/cull/topology/stencil,
independent target descriptors, default sampler flags/stages and swapchain/clear
encodings before any reservation allocation. Maxima and mixed late-invalid
generated controls preserve receipt/pixels with zero allocation. Corrected CPU
and native Release/Debug physical proof pass; all prior acceptance gates are
superseded and the complete matrix restarts from the recorded refrozen hashes.

D0B2A completes corrected-source six builds/canonical selections 272/272,
focused/strict-LSan/generated Release+Debug+sanitized Vulkan, established host
controls and six LAN selections. Whole sanitizer-log audit isolates historical
non-bgfx presentation null-FileSystem and texture-decision invalid address/profile
enum categories; unchanged original/Recording executables cannot reach A.
Explicit parent classification permits the non-regressing native child closeout,
not a global sanitizer-clean or M22 waiver claim. Before D0B2B, persist/deliver
independent presentation-service and complete filter/profile admission corrective
owners, then require affected clean sanitizer controls and a fresh complete matrix.
Shipping FileSystem ordering is already correct; filter rejection must precede
all table access and min/mag/mip/U/V stage mutation. Exact A evidence records
counts, locations and qualification; renderer diagnostic remains excluded.

Corrective D0B2R1 now admits real generated-root FileSystem/local service
ownership before GlobalData, rejects absent/overlapping providers without
construction, and removes services after GlobalData on success/failure.
Two-generation exact global/pool baselines and the unchanged presentation route
pass. Six complete builds/canonical suites finish 272/272, focused/strict-LSan,
established Vulkan and six LAN gates pass. Both complete sanitizer logs prove
the presentation null-service category eliminated; only separately tracked
unchanged R2 address/profile enum categories remain. R2 is mandatory next and
must complete a clean canonical matrix before B2B. No shipping CRC/startup,
native transaction, tree/factory or retail behavior changed; renderer excluded.

Corrective D0B2R2 now uses platform-consistent defined unsigned filter/profile/
address representations with unchanged enum/class/member layout and untouched
Windows declarations. Whole-tuple admission precedes fresh texture Init/access/
selection and direct provider/table/stage effects; default indices reject before
lookup. Two-generation generated controls prove no rejected tuple state/resource/
provider consumption, exact accepted identities/defaults and clean retry.
Both native/both sanitizer focused and strict host LSan controls pass. All six
complete builds/canonical suites finish 272/272; broad complete-log audits have
zero sanitizer findings and prove both historical R1/R2 categories gone without
suppression. ABI, established Vulkan, generated native resource/reservation,
existing texture physical, six LAN and ledger/header/diff gates pass. This closes
the dependency-first corrective matrix before B2B; it does not open the tree
factory or accept the deferred frame journal. Renderer remains excluded.

D0B2B now independently accepts complete bounded immutable frame capture on
the public bgfx owner. Exact native ownership-unit leases and frozen aligned
payloads survive source removal/COW/generation reuse; conflicting aliases and
nonrepresentable coordinates reject before shadow/candidate publication.
Final present is unique, CompleteFrame is last, and synchronous A admission
precedes all accepted target touches. Recoverable reservation rejection retries
the same journal once; unsupported external encoder overlap requires abort and
an ordinary readiness boundary before a fresh attempt, never a live reset.
Typed candidates retire only at ordinary/wait/shutdown boundaries, preserving
native reference multiplicity. The edge forwards exact device/token ownership;
B0 still supplies complete engine stage/ref/map/filter rollback.
Separate self-contained generated shader fixtures preserve production relative
paths and exact shipping/install closure. Refrozen focused 8/8 all four,
strict host LSan 4/4 both, twelve generated physical controls, six complete
builds/canonicals 274/274 with clean complete-log audits, established physical
Vulkan and six serial LAN 4/4 all pass. Exact hashes/ledger/header/diff hold;
unrelated renderer diagnostic remains excluded. B2/B/D0 evidence-only aggregates
follow before B0, exact shroud, immutable tree preparation and scene integration.

D0B2, D0B and D0 evidence-only aggregates now review exact public native
reservation/journal, independent idle/frame native ownership and required
Recording/Linux full-draw capability composition in that order. No executable
surface changes; unchanged B2B final-source six274/274 clean canonical suites
and focused/strict/physical/LAN gates remain valid. SDL/other devices fail closed.
The source edge owns only optional device/token forwarding here: complete
delayed texture refs/maps/transforms/revision/filter/samplers remain B0 next.
Tree preparation/draw/factory and retail admission remain deferred.

B0A independently closes fixed bounded source-reference lifetime: exact64 units,
generation/sequence/count tokens, allocation-free finish/cancel, transactional
native cleanup before terminal source metadata detach/pooled release, alias
multiplicity and receiver-pinned invalidation. Failed checkpoint/destroy/commit
keeps queue and publication for exact retry; mandatory final cleanup cannot
return with abandoned pins. Generated fatal subprocesses model retained versus
consumed terminal disposition, not physical device loss. The first physical
sample mismatch was fixture-only RGBA target versus existing BGRA video pipeline;
production format/shader semantics did not change. All six full builds/canonical
275/275 clean-log suites, focused, strict host LSan, generated/established Vulkan
and six serial LAN4/4 pass. B0B stage transaction is next, then B0 aggregate;
no shroud/tree/factory or retail admission is opened by this reference owner.

B0B now independently closes resident selected-stage tuple/ref/map/transform/
LastAccessed/revision/filter/sampler atomicity. Preflight precedes pins/journal;
selected-only application preserves pending shader/material and other stages.
Grouped all-survivor A cancellation is allocation-free only when every exact
identity retains more refs than queued units; equality/mixed terminal retry
remains transactional. Forty-eight commit-ready generated failure controls,
stale-provider entry guards and two-generation lifecycle cleanup prove exact
rollback/retry without loader/RNG/frame consumption. Six complete builds and
six clean canonical276/276 suites, focused both native/sanitizer, exact strict
host LSan, A/B generated and established Vulkan, six serial LAN4/4 and identity
checks pass. Scope freeze is durable: newly required architecture owners require
an explicit checkpoint, not another autonomous split. B0's evidence-only
aggregate follows separately; exact shroud/tree/factory/retail remain closed.

B0 independently reviews accepted A/B source lifetime and complete selected-stage
atomicity as an evidence-only aggregate. A's grouped survivor/terminal boundary,
exact-generation ordinary cleanup/retry and explicit fatal disposition compose
with B's preflight, no-throw native/source publication and lifecycle cancellation.
Unchanged B0B final-source six276/276 clean canonical suites and focused/strict/
physical/LAN/identity gates remain valid. No executable changes or reruns; exact
shroud semantics are next under the approved critical-path scope freeze.
