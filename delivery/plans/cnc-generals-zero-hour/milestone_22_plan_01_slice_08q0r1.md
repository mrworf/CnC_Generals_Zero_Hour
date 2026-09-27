# M22 plan 01 slice 08Q0R1: strong cloned-render graph construction

Status: complete; final-source acceptance passed.
Evidence: [strong clone graph](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08q0r1_clone_graph.md).
Plan transaction parent: `ab4deb411e6dab48999018a98676349688335b37`.

## Outcome, dependencies and boundary

Depends on accepted M22 slices 01 (original mesh/HLOD graph), 05B2B2B1
(original material/mapper semantics) and 08P0B1 (exact model reference ownership).
Provide strong construction/unwind for the transitive graph reached by
`MeshClass::Make_Unique` / `new MeshModelClass(copy)` before Q0 custom recolor
and prototype publication consumes it. Q0 depends on this correction; slice08
depends transitively. Preserve all currently unstaged Q0/08 trial changes and
the unrelated renderer diagnostic. R1 has a separate exact implementation commit.

No parser, image/remap/BOX, texture upload/cache/prototype publication, scene/frame,
retail admission, allocator redesign or generic vector/hash/ShareBuffer redesign.
No existing fields, class layout, virtuals or serialization changes. Linux-only
source branches keep Windows successful behavior unchanged. Generated inputs
are public/project-owned; no authentication or retail permission is applicable.

## Complete read-only transitive ownership audit

FACT: inspected public WW3D2 mesh.cpp, meshmdl.cpp, meshmdlio.cpp,
meshgeometry.cpp, meshmatdesc.cpp, matinfo.cpp, vertmaterial.cpp/.h,
mapper.cpp/.h, aabtree.cpp, WWLib sharebuf.h, Vector.H, wwstring.cpp/.h,
RANDOM.H and ref-count helpers. The Q0 generated mesh has a vertex material,
screen mapper and texture; remapper construction is mandatory, not hypothetical.

| Owner / publication order | Fallible boundary | Existing cleanup gap / R1 responsibility | Successful semantics preserved |
|---|---|---|---|
| MeshClass::Make_Unique | Allocate/copy new MeshModel before REF_PTR_SET(Model) | Keep old Model identity/refcount until complete clone; release complete candidate on rejection | Shared-model early return and exact replacement |
| MeshModel(copy) | Base geometry copy; default/optional alternate descriptors; MaterialInfo clone; MaterialRemapper and descriptor remap | Keep raw descriptor/MatInfo members null/unpublished while locally guarded candidates complete; nonthrowing final publication | CurMatDesc selects copied default; optional alternate copied; GapFiller stays null as authored copy ctor |
| MeshGeometry(copy) | Add_Refs shared geometry, then optional CullTree allocation and assignment | Constructor failure must release each shared ref and guarded tree; no dependency on most-derived destructor | Geometry buffers remain shared; cull tree copied and rebound to new mesh |
| Newly created AABTree candidate | Node array, then poly-index array | Guard complete default object across assignment; its Reset/destructor cleans partial arrays. Direct copy constructor also needs partial-construction cleanup if used by this closure | Node/index order and mesh rebinding unchanged; ordinary assignment untouched |
| MeshMatDesc(copy) | Shared colors/UV/texture/material refs; independent per-stage texture arrays, per-pass material and shader arrays | Constructor catch withdraws all initialized refs/arrays before rethrow; source never mutated | Fixed pass/stage order, UV/color sharing and independent pointer arrays unchanged |
| ShareBuffer / TexBuffer / MatBuffer copies | One array allocation, followed by scalar/pointer assignment and nonthrowing Add_Ref loop | No additional template fix: allocation occurs before ref acquisitions; element copies in this closure cannot throw | Exact element order and one ref per nonnull pointer |
| MaterialInfo(copy) | Each VertexMaterial::Clone, local pointer storage/Add, then each texture Add_Ref/storage/Add | Build complete bounded node/vector candidates offside; guard uninserted node and all inserted refs; publish storage/counts nonthrowingly | Material clones and shared texture identities/order/ref multiplicity |
| VertexMaterial::Clone | Default material allocation, then Name assignment and mapper clones via operator= | Guard raw default object until assignment completes; a failed helper must release it | Authored assignment semantics, including CRCDirty, retained on success |
| VertexMaterial(copy) | Name member construction, mapper clones in stage order, final raw material allocation | Initialize every mapper slot before fallible work; guard clones/material locally until publication | Authored copy CRCDirty=true, UV/color/light/material/name/unique fields |
| Mapper::Clone families | Allocate concrete mapper; copy scalar/vector/matrix state; authored reset/sync reads | Caller guards acquired clones; all inspected concrete copy bodies have no nested heap ownership | Animated copies retain authored reset/time behavior, not a frozen surrogate |
| RandomTextureMapper copy | Consumes three values of mapper.cpp-local rand4 | An owner-local clone-attempt checkpoint restores this stream on graph failure; success preserves authored consumption. Q0 may retain this internal guard until its own publication | No GameClient, GameLogic or audio stream mutation; no random API/layout change |
| MaterialRemapper | Add_Refs src/dest, allocate vertex-material table then texture table | Build both arrays under guards before publishing refs/arrays; partial construction owns nothing unguarded | Exact paired source/destination order and cached last-hit lookup |
| MaterialRemapper::Remap_Mesh | Set material/texture pointers; when current/alternate source uses arrays absent from copied default, allocate those destination arrays through authored setters | Remap only private candidate descriptors; preflight the whole lookup graph and reserve missing-array bytes, then guard every remap allocation/Add boundary; missing entries reject, never publish partial model | Authored pass/stage/vertex/polygon order, single-vs-array choices and alternate-to-default array creation |

No fallible copy helper outside this cloned-render ownership graph was identified.
MeshClass/RenderObj copy before Make_Unique copies scalar state then Add_Refs its
shared model without later fallible work. Generic MeshModel assignment,
alternate-description installation, model loading/cull-tree building and Scale
are not redesigned; Q0 retains its separate enclosing candidate ownership.
An additional independent owner requires an architecture checkpoint, not widening.

## Construction, bounds and failure contract

Use owner-local RAII or explicit constructor catch cleanup, not cleanup setters
or rendering unregister calls on an incompletely constructed most-derived model.
Do not invoke generic Resize on accepted state as reservation. MaterialInfo may
use narrow Linux-only template friendship (no fields/layout changes) to publish
the complete offside pointer-vector storage nonthrowingly; generic Add/Resize
remain unchanged. Bound candidate allocations using validated source counts and
checked byte products before allocation; reject negative/inconsistent counts,
overflow and the existing 64MiB owner budget without source/ref/native mutation.
Fixed descriptor pass/stage maxima remain the authored constants. No extra
successful count limit is invented for otherwise supported native graphs.

The internal clone-attempt/fault capability has no retail/environment selector,
serialized state or public virtual surface. Generated-only friend access injects
once-only allocation faults before every interior allocation, clone and Add
boundary. Nested construction joins the same owner-scoped attempt; no external
callbacks occur during publication. Every throw releases exact local refs/arrays
in reverse ownership order and restores mapper random state. Existing Model,
geometry/material/texture/mapper identities, bytes, refs, cache/prototype graph and
native resources remain unchanged. Success commits once; normal retry is identical.

## Generated witnesses and acceptance

Register `original_w3d_clone_graph`, `original_w3d_clone_graph_identity` and
`original_w3d_clone_graph_provider_removal` using a separate generated test source,
not Q0's uncommitted house-color witness. Drive actual MeshClass::Make_Unique,
MaterialInfo/VertexMaterial clone, descriptors and optional cull trees. Cover:

- Actual single-material/texture/screen-mapper graph and mixed multi-material,
  two-stage arrays, shared aliases, alternate descriptors and nonnull cull tree.
  Explicitly cover current-alternate arrays absent from copied default so native
  destination-array creation succeeds and every partial remap still unwinds.
- Every interior fault ordinal, including second array/vector allocation and
  failure after acquired mapper/material/texture refs; immediate raw allocation,
  source/ref/cache/prototype/native residual baseline, successful retry and total
  zero teardown across two generations.
- Empty lists/null optional members; fixed pass/stage boundaries, malformed count
  and checked-byte overflow rejection before effects; exact alias multiplicity.
- All supported mapper clone families and authored reset semantics; random-mapper
  failed attempt followed by equivalent-generation success preserves sequence.
- Source geometry/material/texture bytes and identities unchanged, new material
  and mapper identities distinct, geometry refs shared and cull-tree owner rebound.
- Exact provider symbols/link-map, Linux class-layout consistency, Windows source
  branch unchanged; minimal/headless consumers still link and execute.

Exact nine-control focus, after registering its three new IDs:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_clone_graph|original_w3d_clone_graph_identity|original_w3d_clone_graph_provider_removal|original_w3d_cpu_graph|original_w3d_material_abi_isolation|original_w3d_abi|original_w3d_texture_decisions|original_w3d_presentation_identity|original_w3d_presentation_provider_removal)$' --output-on-failure -j1`.
Run on GCC/Clang native and both sanitizers; verify all nine IDs are selected.
Both strict host LSan runs use exactly `ASAN_OPTIONS=detect_leaks=1` and this focus,
no UBSan override. Freeze then run six complete builds, six serial canonical
nonretail suites `-LE 'gpu|lan|retail'` (sanitizers detect_leaks=0), established
native Vulkan source controls, serial LAN4/4 all six, complete log category scans,
ledger, owned diff and staged-path audits. Q0 physical recolor is not R1 acceptance.

## Preconditions and readiness checks

- M22-QCR-01: accepted original mesh/material/model-ref providers above; inspected
  governing index/evidence, no reopening their accepted behavior.
- M22-QCR-02: construction guards/witness are R1 implementation outputs, not
  entry prerequisites; Q0 cannot resume production until R1 acceptance.
- PRE-002: four committed native presets and established GCC/Clang sanitizer
  configurations; `cmake --list-presets` lists all four native presets.
- PRE-012/PRE-016: existing host Vulkan/validation session required only at the
  physical acceptance boundary; fresh verification required, never presumed.
- PRE-008: retail is not used by R1 and remains read-only at later slice08.

Read-only source audit and `ctest -N` confirm the six existing focus IDs;
the three new IDs are R1-owned registration work. Clean-checkout simulation:
accepted providers → R1 construction and generated witnesses → Q0 color owner
→ slice08. No forward dependency, cycle, extra tool, service or new M0 is needed.

## Commit boundary

One exact behavior commit after required gates:
`delivery: M22 08Q0R1 construct cloned render graphs strongly`.
Own only constructor/helper sources/headers, internal clone guard, independent
witness/registration, exact ledger rows and plan/evidence/status hunks. Existing
Q0 source/CMake/vector changes must remain unstaged except reliably separable
R1-owned hunks; preserve active08 and renderer diagnostic entirely.
