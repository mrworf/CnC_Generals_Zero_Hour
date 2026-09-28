# M22 plan 01 slice 08S0: native asset-preload aggregate

Status: complete; evidence-only aggregate of accepted 08S0A/B.
Plan transaction parent: `89df8bb309ad8cd4f6d409cdc27d645f11d9f782`.
Aggregate transaction parent: `0be5d49eb47f8d58937914988503141110d23f8a`.

## Outcome and dependency graph

Accepted original file/bootstrap, mesh/material/texture, 08Q0R1/08Q0 and 08R0
providers precede [08S0A](milestone_22_plan_01_slice_08s0a.md) strong bounded
asset-import/cache ownership → [08S0B](milestone_22_plan_01_slice_08s0b.md)
native display preload and temporary GameClient Drawable lifetime → evidence-only
08S0 → [08](milestone_22_plan_01_slice_08.md) → 09. No further split without a
new architecture checkpoint. Existing accepted slices remain closed.

The redacted continuation reaches the unchanged CPU
`W3DDisplay::preloadModelAssets` pending-device guard after object/player
construction, before camera/recorder continuation. The public stack is
`GameLogic::startNewGame` → `GameClient::preloadAssets` →
`Drawable::preloadAssets` → `W3DModelDraw`/module data/model condition preload
→ display preload. This is not an R0 prop-frame regression. Fixed categories
only: reach/setup/teardown accepted; scene/frame absent; corpus unchanged.

Preserve active08 executable/test/ledger/plan trial hunks and the unrelated
renderer diagnostic unstaged. Retail corpus, original symlink and input choices
stay read-only/private; no names, roots, bytes, hashes, raw logs or images enter
this packet. A/B use generated owned inputs only. No credentials or new service
are required; physical execution uses the existing approved host boundary.

## Complete bounded public-source owner inventory

FACT: inspected display preload CPU/native branches, GameLogic/GameClient and
Drawable/module preload traversal, W3DAssetManager/WW3DAssetManager,
W3DFileSystem/GameFileClass, prototype loaders/definitions, mesh/material load
contexts, HTree/HAnim managers and raw/compressed/morph animation readers,
TextureClass/Load_Texture/TextureHash, Q0 cache and R0 clone/lifecycle contracts.

| Boundary | Source-success semantics / current fallibility | Corrective owner |
|---|---|---|
| Native model preload | Append literal `.w3d` to the supplied model string; W3DAssetManager strips the last extension for its existing-prototype check, then uses native file resolution. No new first-dot rewrite or case/key change. Missing file returns false; the native void preload ignores it. | B preserves best-effort missing preload, not successful required-scene-resource admission; A bounds names/path intermediates before unsafe legacy copies. |
| Native texture preload | Default regular `Get_Texture`, one caller Add_Ref and one Release_Ref. It creates/retains a lazy descriptor, not pixels/residency; no Init, upload, LastAccessed or draw. Missing image is not discovered at descriptor preload. | A owns strong normal texture-cache insertion/metadata rollback; B preserves exact default/multiplicity/no-eager-load behavior. Actual required texture use still fails through accepted loader/draw contracts. |
| File factory / file handle | Exact borrowed or display-owned factory/provider; Get_File, availability, read-only Open, ChunkLoad, Close, Return_File. Filename overload has no Return_File guard if nested load throws. GameFileClass closes its opened engine File on destruction. | A retains exact provider identity, guards Return_File/Close across every exit and bounds each authored lookup candidate without changing lookup order. |
| Whole W3D file | Chunk order publishes hierarchy, raw/compressed/morph animation and registered prototype payloads incrementally. A later malformed/duplicate/unsupported chunk or allocation can leave earlier entries accepted under ordinary source semantics. | A explicit scoped import attempt: all candidate publication is withheld/withdrawn on failure; ordinary non-attempt Load_3D_Assets keeps accepted slice01 intermediate-publication semantics. |
| Prototype registry | Raw loader candidates; Add_Prototype links the native hash chain before fallible vector Add/growth. Definitions/load contexts and wrapper construction can throw before an owner is published. | A complete bounded candidate registry/storage graph and raw-candidate guards; preserve prior capacity/growth, entries, pointers, hash chains and order exactly; nonthrowing publication only after complete admission. |
| Prototype conditional branches | The compiled ordinary loader set is Mesh, HModel, Collection, Box, HLOD, DistLOD, Aggregate, Null, Dazzle, Ring and Sphere. Ring/Sphere import owns only definition/channel arrays; Dazzle imports strings and performs a read-only existing type lookup. R0 clone admission is not proof of cold parser unwind. | A admits and guards these cold definitions, including Aggregate child-definition/name storage and Ring/Sphere primitive-channel key arrays. It does not call their RenderObj Create paths. Alternate USE_WWSHADE/SHD profile loaders and procedural/cube/volume texture creation are explicitly unsupported by this Linux attempt and reject before construction. Existing R0 render-kind limits remain unchanged. |
| Mesh/material load | Mesh owns new Model; MeshModel raw MeshLoadContext, temporary name, geometry/cull generation, default/alternate passes, VertexMaterial/mapper nodes, texture refs and context-vector Add. Error-return cleanup does not cover arbitrary throws; Load_Texture can change a cached sibling's filter tuple. | A local context/node/ref guards plus exact touched-existing texture metadata rollback; preserve source geometry/material/mapper/filter success and alias multiplicity. Q0R1 copy semantics are consumed, not reopened. |
| HTree registry | New tree Load_W3D pivots; fixed 16000-slot TreePtr; pointer/count published before TreeHash key/insertion. | A validates capacity/bound+1 and partial pivots, stages complete pointer/hash metadata; no count/hash half-publication or type0-style alias; exact sibling order and tree identities. |
| HAnim registry | Raw/morph/compressed load creates refs/channels/arrays; Add_Anim Add_Refs before native hash Add. Local channel helpers can throw before parent attachment. | A owns node/channel guards and exact hash/ref publication; restore accepted refs/buckets/order on failure. No animation advance or audio dispatch. |
| Morph nested imports | Load_W3D resolves hierarchy and named pose animations via Get_HTree/Get_HAnim; load-on-demand may recurse and record missing-animation entries/AssetStatus reports. | A nested imports join one bounded root attempt, with exact provider, depth/node/cycle limits; missing-cache metadata is staged/restored too. Import/text parsing convenience APIs not on this binary route remain closed. |
| Texture registry | Normal Get_Texture constructs raw descriptor then generic TextureHash.Insert, which can allocate/copy/rehash; source Load_Texture applies min/mip/address metadata to reused descriptors. | A owner-local strong candidate insertion and typed metadata checkpoint; ordinary generic hash/vector APIs unchanged; no Q0 colored-cache redesign or procedural/cube/volume admission. |
| GameClient preload traversal | Loaded Drawable modules in native order; KINDOF_PRELOAD or preloadEverything temporary Drawables; debris model list, optional control-bar scheme images, particle textures, then authored fixed texture list. | B keeps source order and all existing producer identities; no new UI/media/audio/render admission. Successful prior per-request imports may remain accepted; no invented whole-map preload transaction. |
| Temporary Drawable | newDrawable already has strong constructor rollback. Once returned, draw->preloadAssets may throw before destroyDrawable, leaving registered module/scene/shadow/tree ownership. | B idle preflight and exact scope ownership: every throw retires only the temporary Drawable through accepted lifecycle, no gameplay hooks or sibling reset. No throwing destructor/cleanup masking primary error. |
| Reset/provider removal | GameClient releases Drawables before display scenes/assets; display Free_Assets removes prototypes, animations, trees, then textures, and file factory after WW3D shutdown. | A/B two generations, idempotent retry, borrowed-vs-owned provider removal, no stale generation/native replay; total initialized allocator/resource baseline at final teardown. |

Raw/compressed/morph parser branches and nested material/texture/cache effects
belong to A's one coherent import graph, not another owner or a display shim.
The generated packet covers the finite ordinary cold-loader set above; alternate
shade-profile and texture-creation limits are explicit negative controls, not an
unresolved implementation-ready investigation gate. A newly mandatory independent
owner requires a checkpoint.

## Common final-source acceptance

Six existing configurations: `linux-gcc-debug`, `linux-clang-debug`,
`linux-gcc-release`, `linux-clang-release`, `linux-gcc-sanitized`,
`linux-clang-sanitized`. Configure each to refresh source overlays, then complete
build: `cmake -S . -B build/<preset>` and `cmake --build build/<preset> -j4`.
Each behavior child freezes public source/test/selected executable hashes and
runs its exact focus on GCC/Clang debug and both sanitizers. Strict host focus
uses exactly `ASAN_OPTIONS=detect_leaks=1`, no strict UBSan override.

Six serial canonical suites:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -LE 'gpu|lan|retail' --output-on-failure -j1 -V`.
Audit complete logs for sanitizer/runtime/validation categories, not exit alone;
serialize heavy sanitizer work and retain existing wrapper deadlines/assertions.
Fresh established native Vulkan controls remain GCC display-owner/factory-
bootstrap/factory-map and Clang display-owner/factory-map, using the unchanged
registered wrappers with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` and
`python3 tools/run_validation_clean.py`, existing graphical host escalation.
Exact established Vulkan commands: GCC
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3 tools/run_validation_clean.py ctest --test-dir build/linux-gcc-debug -R '^(original_w3d_display_owner_bgfx|original_w3d_factory_bootstrap|original_w3d_factory_map)$' --output-on-failure -j1 -V`;
Clang uses `build/linux-clang-debug` and
`'^(original_w3d_display_owner_bgfx|original_w3d_factory_map)$'`.
Serial LAN4/4 in all six:
`ctest --test-dir build/<preset> -L lan --output-on-failure -j1`.
Retain minimal/headless linkage/runtime absence controls on both native toolchains.
Exact eight-control minimal focus:
`ctest --test-dir build/<native-preset> -R '^(original_headless_update|original_headless_update_identity|original_headless_update_provider_removal|original_lifecycle_subsystem|original_lifecycle_subsystem_identity|original_lifecycle_spine_objects|original_lifecycle_enum_abi|original_lifecycle_provider_removal)$' --output-on-failure -j1 -V`.
Run `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`,
exact owned/staged diff/whitespace/privacy/identity checks.

A proves import/cache/resource rollback without claiming pixels. B additionally
drives preloaded aliases through the accepted source draw with its generated
physical route on all four focused configurations. Source-owned refs/bytes/
registries/capacity/order/filter metadata return exactly on each failure; live
Edge/native resources match baseline. Accepted Recording monotonic diagnostics
are checked separately for exact bounded candidate deltas, not universally reset.
Fresh equivalent device/Edge and process shutdown return the initialized raw/DMA
baseline exactly. No global reset may hide a failed attempt or destroy siblings.

After B commit, S0 closes separately with evidence-only plans/index/evidence,
verifies accepted A/B commit identity and unchanged B executable hashes, and
reuses B final gates without rerun. Then the existing bounded redacted slice08
retail probe resumes; it is a later gate, not generated A/B acceptance. Exact
runtime roots and existing selections remain private. A different failure stops
for classification/architecture before behavioral edits.

## Preconditions, readiness and commit

M22-S0-01 accepted providers and PRE-002 tools are verified by source/index,
presets and test registration. M22-S0-02 A and M22-S0-03 B are child-owned
outputs, not their own entry prerequisites. PRE-012/PRE-016 physical host and
strict/LAN facilities are freshly verified at acceptance; PRE-008 retail is used
only after the aggregate. No new package/service/credential or M0.
The supplement in the existing readiness manifest/report records the acyclic
clean-checkout order. The original plan-only packet did not claim acceptance;
the separately accepted children now close that boundary.
Aggregate subject: `delivery: M22 08S0 close native asset preload`.

## Accepted closure

08S0A is accepted at `f237f73aadf90da5dbaf944159481452cbf69559`; 08S0B
is accepted at `0be5d49eb47f8d58937914988503141110d23f8a`. Their ancestry,
completed plan states and exact final executable/test hashes are verified.
This aggregate changes no executable or witness and reuses B's six complete
builds, exact14 native/sanitizer/strict focus, physical four, minimal8 both,
established Vulkan3/2, LAN4 all six and six fresh canonical suites295/295.
Complete category audits are clean; the approved preload-wrapper120-second
correction and preserved build/native/minimal identity are documented in B.
The same thirteen active08/renderer paths remain unstaged and unchanged. Resume
only the existing bounded redacted probe after this evidence-only commit;
retail and slice09 acceptance remain pending. The
[aggregate evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08s0_native_preload.md)
records the accepted child identities and unchanged gate provenance.
