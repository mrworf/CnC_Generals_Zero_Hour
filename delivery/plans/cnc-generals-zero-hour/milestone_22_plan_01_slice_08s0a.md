# M22 plan 01 slice 08S0A: strong bounded asset-import/cache transaction

Status: planned; ready after the plan-only checkpoint.
Plan transaction parent: `89df8bb309ad8cd4f6d409cdc27d645f11d9f782`.

## Outcome, dependencies and scope

Depends on accepted 01 (native loader graph), 05B1/05B2A/05B2B1 (file/image/
texture semantics), 05B2B2B1 (material/mapper), 08L3A0 (borrowed-file owner),
08Q0R1/08Q0 and 08R0A composite/lifecycle owners.
Provide one explicit bounded Linux source-owner import attempt spanning a native
W3D file, nested loads and normal texture descriptors. A failed attempt releases
all local candidates and preserves the accepted graph exactly for clean retry.
[S0](milestone_22_plan_01_slice_08s0.md) is the complete public owner inventory
and common acceptance contract. B consumes A; active08 waits on S0 aggregate.

No display adapter, GameClient preload traversal, render/Create kind admission,
frame/present, eager texture pixels/upload, Q0 recolor changes, audio dispatch,
retail input, generic container redesign, fields/virtuals/layout/serialization or
Windows success-path change. Authorization is existing source-owned generated
input only; no authentication applies. Ordinary non-attempt loader semantics
remain accepted slice01 behavior, not a new universal atomic import policy.

Expected surfaces: owner-local W3DAssetManager/WW3DAssetManager import capability;
native assetmgr/proto, HTree/HAnim managers/readers, admitted prototype definitions
and mesh load/context/material/texture helpers; narrow Linux-only friend access
where existing private cache/container state requires it. Add separate import
tests/fixtures, CMake identity/removal registration and exact ledger rows. Do not
change generic container APIs or absorb active08 ModelDraw/Shadow/engine hunks.

## Entry, publication and rollback

An owner-scoped internal attempt wraps the actual native W3D loader/cache calls.
Validate exact asset manager/singleton, FileFactory/FileSystem and existing Edge
identity/generation/idle conditions before mutation; absent device is allowed only
for proven metadata-only work. Reject overlapping/foreign/stale attempts without
poisoning an existing owner. Nested admitted loads join that owner, with fixed
64-depth/65536-node and 64MiB candidate-storage budgets, checked arithmetic and
cycle rejection before recursive work. Existing native per-container maxima,
including 16000 hierarchy slots and Q0's 65536 prototype slots, remain exact.

Bound filename/suffix and every GameFileClass lookup intermediate before legacy
copies; preserve source lookup order, case/key semantics and read-only behavior.
Guard FileClass Return_File and Close across allocation/read/parse/commit throws.
Preserve native false/error contracts for missing/duplicate/malformed/unsupported
loads, but withdraw all new root-attempt state before returning or throwing.
Complete unsupported/malformed chunk preflight rejects late-invalid mixed input
before accepted registry mutation. Never silently construct an unsupported kind.

Build/protect complete candidates for prototype vector/hash/order, HTree pointer/
count/hash, HAnim refs/hash and missing-animation table, normal TextureHash keys/
entries/buckets and touched-existing texture filter metadata. Source Load_Texture
may mutate a reused descriptor: snapshot that exact typed tuple before mutation,
restore allocation-free on failure, preserve successful source semantics.
No accepted sibling pointer/capacity/bucket/key/ref/order changes on a fault.
Nonthrowing final publication occurs only after all parse, allocation, key-copy,
rehash/growth, ref acquisition and admission work completes; no callbacks there.
Generic Insert/Re_Hash/Resize and accepted Q0 colored-cache behavior stay unchanged.

Guard every admitted cold parser candidate, not just published registries:
Mesh/Model geometry/cull/pass/context/temporary name/material/mapper/texture refs;
HModel/HLOD/Collection/DistLOD/Null/Box/Aggregate/Ring/Sphere/Dazzle definitions,
Aggregate child/name storage, Ring/Sphere primitive-channel key arrays and
Dazzle strings/read-only type lookup; HTree pivots; raw/compressed motion/bit channels and morph pose/key/
pivot arrays plus nested hierarchy/animation loads. Never rely on a most-derived
destructor after constructor failure. Explicit guards/catches retain native
successful payload/order/version/optional-member behavior. These are cold
definitions only: no RenderObj Create path is opened. Alternate USE_WWSHADE/SHD
loaders and procedural/cube/volume texture creation remain fail-closed before
construction, with explicit negatives; ordinary registered definitions are not
left behind an unresolved admission decision.
Any independent owner beyond the inventory stops for architecture authority.

Candidate failure keeps accepted source bytes, refs, registry identities/capacity/
order, normal and Q0 caches, native resources and mapper RNG exact. Import does
not advance animation, GameClient/GameLogic/audio RNG or emit sound/FX. Generated
fault seams are owner-local once-only friend capabilities, never retail/env
selectors or serialized/public ABI. Reset/free/provider removal must preserve
accepted sibling refs and cannot call stale-generation providers.

## Tests and exact validation

Register three A-owned IDs/target: `original_w3d_asset_import`,
`original_w3d_asset_import_identity`, `original_w3d_asset_import_provider_removal`,
`original_w3d_asset_import_tests`. Use separate generated input/source ownership,
not active08 trial helpers. Cover cold and cache-hit imports, native .w3d naming,
all admitted prototype/hierarchy/raw/compressed/morph and mesh/material branches,
shared aliases and pre-existing siblings. Assert exact payload/ref/cache/order and
lazy nonresident textures. Missing/open/read/truncated/late malformed/duplicate/
unknown/unsupported failures and exact capacity/bound+1/overflow/depth/cycle cases
must leave no new registry/ref/file/resource effects.

Inject every local construction/read/admission/key-copy/vector-growth/hash/ref/
publication boundary, including nested import and failure after earlier valid
chunks; retry through the same public source loader. Cover existing texture-filter
rollback, HTree full capacity, HAnim Add_Ref/hash failure, prototype chain/vector
failure, normal texture candidate failure and no Q0 sibling changes. Cover
provider removal, no-device metadata success versus physical work rejection,
busy/stale/foreign owner, two generations/reset/teardown and exact raw/DMA baseline.
Assert all explicitly unsupported profile kinds reject before construction.

Focused build:
`cmake --build build/<preset> --target original_w3d_asset_import_tests original_w3d_cpu_graph_tests original_w3d_house_color_tests original_w3d_prop_owner_tests -j4`.
Exact eleven-control focus, after registering the three A-owned IDs:
`ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build/<preset> -R '^(original_w3d_asset_import|original_w3d_asset_import_identity|original_w3d_asset_import_provider_removal|original_w3d_cpu_graph|original_w3d_material_abi_isolation|original_w3d_abi|original_w3d_texture_decisions|original_w3d_presentation_identity|original_w3d_presentation_provider_removal|original_w3d_house_color|original_w3d_prop_owner)$' --output-on-failure -j1 -V`.
Verify exact 11 registration/count; new IDs are implementation outputs, not
claimed run. Four focused configurations, strict same11 both sanitizers,
six complete builds/serial canonicals, established native Vulkan/minimal/LAN,
full log category audits, ledger/diff/staged checks follow S0. No new A pixel claim.

## Preconditions, readiness and commit

M22-S0-01 accepted providers above; PRE-002 tools/presets and committed generators.
M22-S0-02 import implementation/new witnesses are A outputs, not entry gates.
Readiness: source/index/accepted commit inspection, `cmake --list-presets`,
existing eight focus IDs/targets verified; no warm cache/private corpus required.
PRE-012/PRE-016 host facilities apply only at final acceptance, freshly checked.
Acceptance: every declared branch/fault/bound/retry/provider/teardown passes,
ordinary loader/ABI/Windows behavior remains unchanged and all final gates clean.
Commit only A-owned source/test/registration/ledger/plan/evidence/index hunks:
`delivery: M22 08S0A transact bounded asset imports`.
Preserve active08/renderer edits; B remains closed until A acceptance/commit.
