# M22: original retail scene rendering on Vulkan

This plan governs exactly one milestone transaction.

## Outcome

The production Linux original-engine path renders representative campaign and skirmish scenes from the user-owned retail corpus through the original W3D, WWShade, and GameClient producers onto the public SDL_GPU Vulkan backend. The same producer path is observable through the recording device, fails closed for unsupported state or malformed/missing assets, survives resize/device-resource recreation, and returns all owned renderer resources to zero at teardown.

## Delivery-goal context

- Goal status record: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Active packet and revision: `delivery/milestones/cnc-generals-zero-hour/status.yaml`, `sha256:21ce030a34f700bbcf7773983454f0b741f384956f1ff6568d7fc6782bc5bfe9`
- Source transaction: `cb567c223beab5d63fa2f13d66718e670271d3d0`
- Planning transaction: `cd2389e40aa89b1c2d1e02be638d084a95438f2d` (closeout `75da0520c91b14bbea21d3d58c99e3eb3905aa3`)
- Fixed goal scope: `M26, M27, M28, M20, M21, M22, M23, M24, M25, M15, M16, M17, M18` (context only)
- Current milestone: `M22`
- Resume mode: new
- Transaction-start HEAD: `0c227ac924b163d9f0caf0e703e5610ed37fd9b2`
- Pre-existing dirty paths: none

## Governing contracts

- PRD requirements: `docs/zero-hour-linux-port-plan.md` §§1–6 and §10; `docs/zero-hour-source-engine-migration.md` SE-005 and SE-010.
- Milestone acceptance: `delivery/milestones/cnc-generals-zero-hour/M22-original-rendering.md` in full.
- Architecture decisions: `docs/zero-hour-runtime-closure-reconciliation.md` RC-002, RC-003, RC-004, RC-007, RC-010, and especially RC-012.
- Renderer contracts: `docs/renderer/renderer-contract.md` and `docs/renderer/legacy-api-mapping.tsv`.
- Dependency and source identity: `docs/original-runtime-dependency-ledger.tsv`.
- Accepted providers: `evidence/qa/cnc-generals-zero-hour/M20-plan03-original-lifecycle-acceptance.md`, `evidence/qa/cnc-generals-zero-hour/M21-original-simulation.md`, and `evidence/qa/cnc-generals-zero-hour/M14-gpu-acceptance.md`.
- External gates: PRE-008 retail roots plus freshly verified PRE-012/PRE-016 (RTX 4070, NVIDIA 610.57.04, Vulkan 1.4.341, `VK_LAYER_KHRONOS_validation` 1.4.357, Wayland session).
- Repository instructions: no `AGENTS.md` is present; the retail symlink and all user-owned corpus content are read-only.

## Current-state findings

- M20 provides the canonical 19-entry original W3D schema and production factory identity. M21 enabled ten original concrete draw classes only for CPU construction/state/preload/destruction; their physical model, bone, shadow, and draw operations remain explicit unavailable-device edges.
- The dependency ledger has two M22-owned rows: original `DX8Wrapper` physical device/drawing semantics and all deferred original W3D draw-instance constructors/vtables and their render consumers.
- M14 proves the public SDL_GPU Vulkan backend and validation harness with generated/component scenes. M28's `OriginalCpuPresentation.cpp` is an independently accepted resource extraction, not the full original producer and not retail-scene acceptance.
- `LinuxDisplay`, `LinuxView`, and `LinuxTerrainVisual` currently record/preload CPU state without acquiring a device or drawing. They are the Linux factory boundary through which the original GameClient must receive the M22 presentation adapter; they must not become a parallel authoritative scene implementation.
- A rejected implementation probe demonstrated that calling `W3DModelDraw` and then supplying adapter-owned geometry/material data is not an original renderer closure. No part of that uncommitted probe is retained. Geometry, hierarchy, animation, texture identity, and `ShaderClass` state must first come from the original WW3D object graph.
- The selected M21 retail campaign/skirmish pair reaches ten concrete GameClient draw classes. Its minimal upstream WW3D CPU closure begins at `W3DAssetManager`/`WW3DAssetManager`, `ChunkLoadClass`, the registered prototype loaders, mesh/HLOD/hierarchy/animation loaders, texture/material/mapper/`ShaderClass` state, and their render-object ownership graph. Only source files required by compile/link reachability and runtime witnesses are added; the unrelated remainder of the 244-file WW3D2 directory is not presumed in scope.
- Slice 01 restored the original WW3D CPU graph (`8e8e12b`). A full-behavior compile of all ten concrete draw translation units then exposed original `W3DDisplay` scene/asset state, `W3DShadowManager` selection/lifetime, and terrain-track bind/edge/unbind as mandatory linked CPU owners. Their source TUs also contain Win32/DX8 physical operations, so original CPU presentation ownership must close before subclass runtime acceptance. A compile-only GameClient draw target is not acceptance.
- `ZH_W3D_SCHEMA_ONLY`, `ZH_W3D_HEADLESS_INSTANCE` and `BRUTAL_TIMING_HACK` alter source bodies and, for the latter two, class layout/`RenderObjClass` declarations. M20/M21 schema tests and M22 production therefore need mutually exclusive executable configurations of the same canonical sources, verified against link-map and ABI selection; mixing those object definitions is forbidden.
- Existing project-owned renderer contracts already cover explicit resources, immutable pipeline state, uploads, render passes, presentation, and public Vulkan-backed resize/recreation. M22 must bind original source state into that contract rather than reintroducing Direct3D or private Vulkan calls.
- The ordinary four presets intentionally remain asset- and device-independent. Retail/GPU checks require separate options and must not leak paths, filenames chosen from the private corpus, bytes, or hashes into committed output.

## Decisions

1. Preserve `W3DModuleFactory` and all concrete `ModuleData` identities from M20. M22 removes a fail-closed operation only when the actual rendering consumer is implemented and tested; it does not add another registry, generic draw class, or reduced schema.
2. The adapter boundary is the existing public `renderer::GpuDevice` contract. Original W3D/WWShade/GameClient code remains the producer of model selection, transforms, animation/material state, camera, terrain, fog/shroud, lighting, shadows, particles, water, and effects. Adapter code translates those reached legacy states and resources into explicit device commands; it may not synthesize a replacement scene.
3. Recording and Vulkan runs invoke the same original producer entry point. Recording is the deterministic semantic witness and negative-control surface; hardware is the device, validation, resize, presentation, and visual-evidence gate.
4. Retail logical input names are supplied privately by the gate and read through the existing VFS. Tests commit only logical/aggregate results and project-owned screenshots or derived visual observations permitted by the product contract; no retail bytes, hashes, or host paths are retained.
5. Unsupported required legacy state, missing/malformed required assets, stale resources, and omitted original providers are hard failures. No placeholder texture/model, generated scene, ignored state, generic/no-op draw module, or silent successful null behavior can satisfy acceptance.
6. Eleven slices are the dependency-safe sequence after the source-order audit: accepted original WW3D assets, GameClient ownership and draw overrides; bounded FVF/visibility CPU gate; original buffer/category ownership; mesh entry and canonical WW3D state; first physical translation; original interleaved complete pass graph; GameClient integration; retail recording; then validation-enabled Vulkan acceptance. An unavailable device edge during CPU slices never establishes successful pass scheduling or a frame.

## Scope

### Included

- Original DX8Wrapper/WWShade state and resource translation needed by reached M22 rendering consumers.
- Every ledger-deferred W3D draw operation reached by the selected campaign and skirmish scene families, including all newly reachable CPU dependencies.
- Original model/HLOD/animation/texture, terrain, camera, object, lighting, fog/shroud, shadow, particle, water, and effects producers required by representative real scenes.
- Recording-device semantic/ownership evidence, source identity and provider-removal controls, missing/malformed/unsupported negative controls, and clean reset/re-entry.
- SDL_GPU Vulkan presentation on the verified RTX, explicit Khronos validation-layer scanning, visual evidence, resize/recreation, wait-idle, and bounded teardown.
- Four canonical preset builds/full asset-free CTest suites, focused ASan/UBSan, installed/arbitrary-CWD and dependency-ledger freshness checks.

### Excluded and deferred

- Menus, controls, interactive UI, music/audio/video flows (M23).
- Complete playable sessions and gameplay acceptance (M15), save/replay (M24), and LAN (M25/M16).
- ARM64, base Generals, tools, online services, private Vulkan calls, Direct3D dependencies, and redistribution of retail content.
- Replacing original producers with M14 generated scenes, M28 fixture parsers, or other toy/proxy scene implementations.

## Slice index

| Slice | Plan | Outcome | Dependencies | Status | Commit | Evidence |
|---|---|---|---|---|---|---|
| 01 | [milestone_22_plan_01_slice_01.md](milestone_22_plan_01_slice_01.md) | Original WW3D loaders construct the reached CPU asset, render-object, material, hierarchy, and animation graph from W3D resources with exact ownership/failure behavior. | M20, M21 | complete | `8e8e12b` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_01.md) |
| 02 | [milestone_22_plan_01_slice_02.md](milestone_22_plan_01_slice_02.md) | Original GameClient display, scene, asset, shadow and track CPU owners close the draw-class link graph with a typed unavailable GPU edge and unmixed ABI. | slice 01 | complete | `6cda594` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_02.md) |
| 03 | [milestone_22_plan_01_slice_03.md](milestone_22_plan_01_slice_03.md) | All ten original concrete GameClient draw classes preserve source-owned hierarchy, animation, bone/tread/supply/rider and dependency behavior. | slice 02 | complete | `c9d9be6` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_03.md) |
| 04A | [milestone_22_plan_01_slice_04a.md](milestone_22_plan_01_slice_04a.md) | Original FVF CPU layout and original MeshClass hidden/visible gate, explicitly not pass scheduling. | slice 03 | complete | slice 04A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_04a.md) |
| 04B | [milestone_22_plan_01_slice_04b.md](milestone_22_plan_01_slice_04b.md) | Original vertex/index CPU ownership, upload metadata and source-owned mesh/category registration before physical submission. | slice 04A | complete | slice 04B commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_04b.md) |
| 04C | [milestone_22_plan_01_slice_04c.md](milestone_22_plan_01_slice_04c.md) | Original mesh visibility/frustum/sort/overrides and category entry decisions plus canonical WW3D state stop at first typed physical edge; no completed pass claim. | slice 04B | complete | slice 04C commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_04c.md) |
| 05 | [milestone_22_plan_01_slice_05.md](milestone_22_plan_01_slice_05.md) | Translate first source-issued physical DX8 resource/state/draw calls through `GpuDevice`; no synthetic precomputed pass graph. | slice 04C, M14 | pending | | |
| 06 | [milestone_22_plan_01_slice_06.md](milestone_22_plan_01_slice_06.md) | Full interleaved original mesh/WWShade/WW3D material, rigid/skin/decal/static/sorting pass graph records owned fixture frames in authored call order. | slice 05 | pending | | |
| 07 | [milestone_22_plan_01_slice_07.md](milestone_22_plan_01_slice_07.md) | Original GameClient display/scene, 2D, terrain, track, shroud and selected shadow/effect routes integrate and switch production to full behavior. | slice 06 | pending | | |
| 08 | [milestone_22_plan_01_slice_08.md](milestone_22_plan_01_slice_08.md) | Original campaign/skirmish consumers load and record complete retail scene families with failure/reset and provider-removal evidence. | slice 07, PRE-008 | pending | | |
| 09 | [milestone_22_plan_01_slice_09.md](milestone_22_plan_01_slice_09.md) | Same retail scenes present on validation-enabled SDL_GPU Vulkan, survive resize/recreation, yield reviewed visuals and pass cumulative acceptance. | slice 08, PRE-012, PRE-016 | pending | | |

## Cross-slice concerns

- Compatibility and migration: keep the 19 original registry names, concrete data types, tags, defaults, inherited fields, and runtime class identities. Preserve left-handed coordinates, 0..1 depth, clockwise winding, top-left texture origin, ARGB8/premultiplied-alpha, fog, bias, and WWShade multipass semantics.
- Authorization and security: retail roots are read-only; outputs are isolated under temporary/XDG locations. Evidence must redact private paths and must not store retail bytes, filenames selected from the private corpus, or hashes.
- Invalidation and lifecycle effects: renderer reset must invalidate device resources without invalidating original simulation authority. Destruction is reverse, exactly once, bounded, and safe after partial failure, resize, reset, or device recreation.
- Audit and observability: recording markers/commands identify the original producer and logical scene family, not merely adapter activity. Validation output is combined and rejected on `Validation Error` or `VUID-` even if the executable exits zero.
- Performance and scale: selected scenes are bounded but retain real retail terrain/object/resource counts; no timeout increase or scene reduction hides quadratic behavior or leaks.
- Environment or external services: Vulkan and a graphical session are required only for slice 09; read-only retail material is required in 08–09. Ordinary tests stay display/GPU/retail independent.

## Milestone completion gate

- Configure and build `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`; run the complete asset-free CTest suite in each preset, with repository-required UDP exceptions run under the established local-socket procedure.
- Run focused original-rendering positive/negative, source-identity, provider-removal, dependency-ledger, and lifecycle tests in GCC Debug and Clang Debug ASan+UBSan.
- Run installed-form owned fixtures from arbitrary CWD with isolated XDG state.
- Run read-only retail campaign and skirmish recording gates and verify the corpus metadata is unchanged.
- Run the same retail scenes on SDL_GPU Vulkan with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`; fail on any `Validation Error` or `VUID-` output.
- Capture/review the permitted visual evidence for every required scene family and both dimensions of resize/resource recreation; verify no placeholders or missing required assets.
- Verify zero live original allocations/workers/device resources after normal and injected-failure teardown, `git diff --check`, source classification, provider-removal, and dependency-ledger freshness.

## Rollback and recovery

Each slice is independently revertible. Slices 01–04C add only original CPU producer behavior and do not claim a renderer. Slice 05 establishes first translated physical commands; 06 closes the original interleaved pass graph; 07 integrates production GameClient traversal. Slice 08 records retail scenes without requiring hardware. Slice 09's device-gated tests remain opt-in. Reverting never writes or migrates retail or XDG user data. A partial producer/resource failure unwinds adapter resources and original draw instances before returning an actionable error.

## Execution notes

- Planning phase completed before production changes at transaction-start HEAD `0c227ac924b163d9f0caf0e703e5610ed37fd9b2`; worktree was clean.
- M14/M28 generated or fixture scenes may remain regression coverage but are explicitly non-acceptance for M22.
- Replan checkpoint: an uncommitted adapter-owned resource probe was rejected and removed before this revision. The original three slices map to new slices 03, 04, and 05; new prerequisite slices 01 and 02 close the previously hidden producer dependencies.
- Post-slice-01 replan checkpoint: canonical full draw TUs compile, but their link probe discovers original `W3DDisplay`, `W3DShadowManager`, and terrain-track CPU owners absent from the earlier boundary. Original slice 02 becomes slice 03; previous slices 03–05 become 04–06. New slice 02 owns that precise source closure, avoiding a dependency cycle in which subclass behavior would otherwise depend on the later device-translation slice. Only plans are committed at this checkpoint; the compile probe remains uncommitted and non-acceptance.

## Post-slice-03 replan

The real original draw-module witness revealed that `MeshClass::Render` in the
current CPU-only configuration throws *before* original frustum/sort/alpha,
shadow/additional-pass and polygon task scheduling. Its authored continuation
uses `MeshModelClass::Register_For_Rendering`,
`DX8PolygonRendererClass`, `DX8FVFCategoryContainer` and shader/material
routes. `WW3D::Flush` in original `ww3d.cpp` orders DX8 mesh, WWShade,
static-sort, and sorting-renderer flushes; the currently linked
`ww3d_cpu_state.cpp` covers static defaults only. Source-local extraction
must eliminate duplicate static definitions when original `ww3d.cpp` joins
the Linux build. Original `dx8wrapper.cpp` includes `<D3dx8core.h>` and physical
Direct3D calls. None of these paths is proven by mere HLOD/child construction.
Original pass scheduling must execute before a physical GpuDevice translation
can faithfully record it. Thus prior slice 04 becomes slices 04/05, and its
GameClient full-scene dependency becomes slice 06; previous retail recording
and Vulkan acceptance become slices 07/08. Accepted 01–03 remain unchanged.
Only original source owners may schedule pass/geometry/material behavior;
adapters terminate at the physical GPU/OS boundary. This replan is committed
before any new production edit. Exact new source TUs are still gated by
reached compile/link/runtime evidence, not entire legacy inventory.

## Post-slice-03 bounded CPU replan

The original mesh pass depends on canonical FVF layout, original vertex/index
buffer ownership and original DX8 mesh/category registration before the
`WW3D::Flush` pass scheduler can be characterized honestly. Slice 04 is
therefore split into 04A/04B/04C, each with a separately testable source
boundary. 04A's hidden/visible gate is only a bounded CPU step; it cannot
establish original mesh pass scheduling. 04B's CPU ownership likewise cannot
claim a rendered frame. 04C must prove original pass ordering and typed
physical-edge failure before 05 can translate it. No surrogate GPU SDK,
proxy geometry/material authority, duplicate source definitions or mixed
M20-schema/M22-full ABI is permitted. Slices 05–08 retain their original
acceptance and require 04C transitively. This replan is committed before
additional production changes.

## Post-slice-04B source-order correction

The 04C source audit discovered a real dependency cycle in the previous
wording: original `DX8TextureCategoryClass::Render` calls physical
`DX8Wrapper::Set_Texture`, `Set_Material` and `Set_Shader` before its later
alpha/material/per-mesh decisions; original skin rendering creates the
dynamic vertex-buffer access before completing skin geometry; and original
`WW3D::Flush` calls the physical mesh flush before WWShade, static-sort and
sorting flush. Consequently it is impossible to run the *entire* authored
ordered pass graph before first physical translation without reordering
original source or inserting a surrogate scheduler. Accepted 01–04B stay
unchanged. Slice 04C now owns independently testable mesh/category CPU entry
decisions, canonical WW3D state and typed physical failure; 05 translates
first source-issued physical commands; new 06 completes original interleaved
mesh/WWShade/WW3D pass scheduling using that translated edge. Prior 06–08
become 07–09 (GameClient, retail recording, Vulkan respectively). Each
boundary preserves the final M22 acceptance; no generated substitute or
partial frame may count as a successful retail scene. `USE_WWSHADE` remains
authored configuration, and either enabled family encounters or the disabled
branch and regular shader/material semantics require evidence. This replan
is committed as plans only before further production implementation.

## Deferred follow-ups

- Interactive UI/media and user input remain M23.
- Full playable-session evidence remains M15.
