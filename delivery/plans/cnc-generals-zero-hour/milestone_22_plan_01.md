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
6. The twenty slices indexed below are the dependency-safe sequence after the source-order, texture-provider and dynamic FVF-layout audits: accepted original WW3D assets, GameClient ownership and draw overrides; bounded FVF/visibility CPU gate; original buffer/category ownership; mesh entry and canonical WW3D state; public 16-bit index contract and original buffer physical entry; canonical TGA/DDS image providers; original texture format/bitmap/loader CPU decisions; original texture loader/material/shader physical entry; canonical physical FVF layout; separate applied-state mapping and GPU shader lowering; complete interleaved pass graph; GameClient integration; retail recording; then validation-enabled Vulkan acceptance. An unavailable device edge during CPU slices never establishes successful pass scheduling or a frame.

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
| 05A | [milestone_22_plan_01_slice_05a.md](milestone_22_plan_01_slice_05a.md) | Public 16-bit index/offset/base contract and source-issued original vertex/index buffer physical commands, typed at later texture edge. | slice 04C, M14 | complete | slice 05A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05a.md) |
| 05B1 | [milestone_22_plan_01_slice_05b1.md](milestone_22_plan_01_slice_05b1.md) | Canonical original Targa and DDS providers parse and decode reached owned/retail image families with bounded failure behavior; no texture/frame claim. | slice 05A | complete | slice 05B1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b1.md) |
| 05B2A | [milestone_22_plan_01_slice_05b2a.md](milestone_22_plan_01_slice_05b2a.md) | Original WW3D format/bitmap and TextureLoader selection, reduction, mip and pixel decisions stop at first typed texture-device edge. | slice 05B1 | complete | slice 05B2A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2a.md) |
| 05B2B1 | [milestone_22_plan_01_slice_05b2b1.md](milestone_22_plan_01_slice_05b2b1.md) | Original TextureLoadTask Lock/Load/Unlock owns decoded mip bytes, source-issued GPU create/upload and texture lifetime/failure controls. | slice 05B2A | complete | `4ec52aa` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b1.md) |
| 05B2B2A | [milestone_22_plan_01_slice_05b2b2a.md](milestone_22_plan_01_slice_05b2b2a.md) | Original TextureClass/TextureFilter source decisions and source-ordered pending stage/sampler/texture state at the device edge. | slice 05B2B1 | complete | `fbde148` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2a.md) |
| 05B2B2B1 | [milestone_22_plan_01_slice_05b2b2b1.md](milestone_22_plan_01_slice_05b2b2b1.md) | Original mapper/VertexMaterial and DX8Wrapper source ref-counted delayed material/UV state. | slice 05B2B2A | complete | `b70a134` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b1.md) |
| 05B2B2B2 | [milestone_22_plan_01_slice_05b2b2b2.md](milestone_22_plan_01_slice_05b2b2b2.md) | Original ShaderClass blend/alpha/fog/depth/cull and two-stage combiner/capability decisions. | slice 05B2B2B1 | complete | `6c68e11` | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b2.md) |
| 05B2B2B3A | [milestone_22_plan_01_slice_05b2b2b3a.md](milestone_22_plan_01_slice_05b2b2b3a.md) | Public Recording/SDL_GPU canonical dynamic FVF layout, indexed Vulkan evidence and unsupported-format rejection. | slice 05B2B2B2 | complete | slice B3A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b3a.md) |
| 05B2B2B3B1 | [milestone_22_plan_01_slice_05b2b2b3b1.md](milestone_22_plan_01_slice_05b2b2b3b1.md) | Original applied DX8 state snapshot, exact semantic mapping and unsupported negatives before physical resource creation. | slice 05B2B2B3A | complete | slice B3B1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b3b1.md) |
| 05B2B2B3B2A | [milestone_22_plan_01_slice_05b2b2b3b2a.md](milestone_22_plan_01_slice_05b2b2b3b2a.md) | Source-issued Recording pipeline/uniform/stage binding descriptor, lifetime, error/replay and unsupported-state checks. | slice 05B2B2B3B1 | complete | slice B3B2A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b3b2a.md) |
| 05B2B2B3B2B | [milestone_22_plan_01_slice_05b2b2b3b2b.md](milestone_22_plan_01_slice_05b2b2b3b2b.md) | GLSL/SPIR-V public SDL_GPU lowering, exact source shader pixels under validation, bounded recreation. | slice 05B2B2B3B2A | complete | slice B3B2B commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_05b2b2b3b2b.md) |
| 06A1 | [milestone_22_plan_01_slice_06a1.md](milestone_22_plan_01_slice_06a1.md) | Original DX8Wrapper buffer/index/base and draw methods issue exact device-edge indexed operations; direct method witness, no category claim. | slice 05B2B2B3B2B | complete | slice 06A1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a1.md) |
| 06A2 | [milestone_22_plan_01_slice_06a2.md](milestone_22_plan_01_slice_06a2.md) | Original rigid/category/polygon single source body issues unlit material/texture/world/alpha interleaving through A1 physical draw. | slice 06A1 | complete | slice 06A2 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a2.md) |
| 06A3 | [milestone_22_plan_01_slice_06a3.md](milestone_22_plan_01_slice_06a3.md) | Aggregate original category LightEnvironment, source FVF and lit GPU acceptance; delivered by 06A3A→B1→B2→C. | slice 06A2 | complete | aggregate 06A3C | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3c.md) |
| 06A3A | [milestone_22_plan_01_slice_06a3a.md](milestone_22_plan_01_slice_06a3a.md) | Original source-owned ambient/directional/point-light expansion and bounded physical snapshot with typed lit edge. | slice 06A2 | complete | slice 06A3A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3a.md) |
| 06A3B | [milestone_22_plan_01_slice_06a3b.md](milestone_22_plan_01_slice_06a3b.md) | Aggregate exact source-lit state and GPU Vulkan pixels, delivered by B1→B2. | slice 06A3A | complete | aggregate 06A3B2 | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3b2.md) |
| 06A3B1 | [milestone_22_plan_01_slice_06a3b1.md](milestone_22_plan_01_slice_06a3b1.md) | Canonical source lighting selector/default and semantic snapshot mapping, physical lit draw typed unavailable. | slice 06A3A | complete | slice 06A3B1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3b1.md) |
| 06A3B2 | [milestone_22_plan_01_slice_06a3b2.md](milestone_22_plan_01_slice_06a3b2.md) | Exact lit shader/uniform GPU lowering and validation-enabled Vulkan pixels. | slice 06A3B1 | complete | slice 06A3B2 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3b2.md) |
| 06A3C | [milestone_22_plan_01_slice_06a3c.md](milestone_22_plan_01_slice_06a3c.md) | Original category-issued lit frame, failures/recreation/teardown and full rigid aggregate acceptance. | slice 06A3B2 | complete | slice 06A3C commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06a3c.md) |
| 06B | [milestone_22_plan_01_slice_06b.md](milestone_22_plan_01_slice_06b.md) | Aggregate original skin, additional/procedural/cull-volume and decal pass owners; delivered by 06B1→B2→B3→B4. | slice 06A3C | pending | aggregate 06B4 | |
| 06B1 | [milestone_22_plan_01_slice_06b1.md](milestone_22_plan_01_slice_06b1.md) | Canonical original dynamic VB/IB access CPU storage, lock, discard/no-overwrite, offset, reset and bounded physical upload; no skin claim. | slice 06A3C | complete | slice 06B1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b1.md) |
| 06B2 | [milestone_22_plan_01_slice_06b2.md](milestone_22_plan_01_slice_06b2.md) | Aggregate exact skinned FVF source-color semantics and original HLOD skin/category draw, delivered by 06B2A→B. | slice 06B1 | complete | aggregate 06B2B | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b2b.md) |
| 06B2A | [milestone_22_plan_01_slice_06b2a.md](milestone_22_plan_01_slice_06b2a.md) | Exact original dynamic FVF 0x252 normal/COLOR1/UV2 shader input and source-selected lit material color terms. | slice 06B1 | complete | slice 06B2A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b2a.md) |
| 06B2B | [milestone_22_plan_01_slice_06b2b.md](milestone_22_plan_01_slice_06b2b.md) | Original HLOD-owned skin HTree deformation, dynamic category draw, physical pixels and no-draw/retry. | slice 06B2A | complete | slice 06B2B commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b2b.md) |
| 06B3 | [milestone_22_plan_01_slice_06b3.md](milestone_22_plan_01_slice_06b3.md) | Aggregate original skin/rigid additional pass, delayed scheduler and cull-volume dynamic index routes, delivered by 06B3A→B→C. | slice 06B2 | complete for owned source routes; retail reachability 08 | aggregate 06B3C | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b3c.md) |
| 06B3A | [milestone_22_plan_01_slice_06b3a.md](milestone_22_plan_01_slice_06b3a.md) | Original MaterialPass install and MeshClass immediate skin/rigid non-cull pass with per-instance override restoration. | slice 06B2 | complete | slice 06B3A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b3a.md) |
| 06B3B | [milestone_22_plan_01_slice_06b3b.md](milestone_22_plan_01_slice_06b3b.md) | Original rigid delayed scheduling/list timing and physical additional pass after base. | slice 06B3A | complete | slice 06B3B commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b3b.md) |
| 06B3C | [milestone_22_plan_01_slice_06b3c.md](milestone_22_plan_01_slice_06b3c.md) | Original cull-volume APT and dynamic index branch with negative rollback and aggregate pass acceptance. | slice 06B3B | complete | slice 06B3C commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b3c.md) |
| 06B4 | [milestone_22_plan_01_slice_06b4.md](milestone_22_plan_01_slice_06b4.md) | Aggregate original decal system/mesh generation, bounded depth bias, original physical rigid/skin decal and full 06B frame delivered by 06B4A→B→C→D1→D2. | slice 06B3 | pending | aggregate 06B4D2 | |
| 06B4A | [milestone_22_plan_01_slice_06b4a.md](milestone_22_plan_01_slice_06b4a.md) | Canonical original projector/decal system and rigid/skin CPU decal owners, APT/clipping/UV with typed first device edge. | slice 06B3 | complete for owned CPU/decal creation routes; physical queue remains 06B4B-D | slice 06B4A commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b4a.md) |
| 06B4B | [milestone_22_plan_01_slice_06b4b.md](milestone_22_plan_01_slice_06b4b.md) | Original source `D3DRS_ZBIAS` through bounded public pipeline depth bias and validation-layer depth pixels. | slice 06B4A | complete for source 0/8; wireframe 7 typed and reachability audit 07/08 | slice 06B4B commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b4b.md) |
| 06B4C | [milestone_22_plan_01_slice_06b4c.md](milestone_22_plan_01_slice_06b4c.md) | Original rigid/skin decal physical dynamic vertex/index/material/texture draw with source-owned clipping and failure/retry. | slice 06B4B | complete for direct original owner routes; canonical Flush queue remains 06B4D | slice 06B4C commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b4c.md) |
| 06B4D | [milestone_22_plan_01_slice_06b4d.md](milestone_22_plan_01_slice_06b4d.md) | Aggregate original Flush decal queue/ZBIAS begin/reset, skin/additional/delayed/APT/decal interleave, rollback and 06B acceptance via D1→D2. | slice 06B4C | pending | aggregate 06B4D2 | |
| 06B4D1 | [milestone_22_plan_01_slice_06b4d1.md](milestone_22_plan_01_slice_06b4d1.md) | Original Flush queue and source ZBIAS bracket, physical failure abort/reset/requeue, no aggregate claim. | slice 06B4C | complete for original queued rigid/HLOD skin decals; aggregate remains 06B4D2 | slice 06B4D1 commit | [evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_06b4d1.md) |
| 06B4D2 | [milestone_22_plan_01_slice_06b4d2.md](milestone_22_plan_01_slice_06b4d2.md) | Same-Flush complete owned scene with original category, skin, additional/cull, delayed, both decal owners, validation pixels, 06B aggregate. | slice 06B4D1 | pending | | |
| 06C | [milestone_22_plan_01_slice_06c.md](milestone_22_plan_01_slice_06c.md) | Original sorting/static passes, enabled WWShade behavior and complete owned interleaved fixture frames with rollback/teardown. | slice 06B | pending | | |
| 07 | [milestone_22_plan_01_slice_07.md](milestone_22_plan_01_slice_07.md) | Original GameClient display/scene, 2D, terrain, track, shroud and selected shadow/effect routes integrate and switch production to full behavior. | slice 06C | pending | | |
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

Each slice is independently revertible. Slices 01–04C add only original CPU producer behavior and do not claim a renderer. Slice 05A establishes original vertex/index physical commands, 05B1 proves original image providers, 05B2A preserves original image/format/mip selection before the first device operation, 05B2B1 restores image resource/upload behavior, 05B2B2A restores source-issued texture stage/filter choices, and 05B2B2B1/B2 restore source mapper/material/delayed state and shader/combiner decisions. B3A proves original FVF physical layouts; B3B1 maps exact original applied state, B3B2A verifies source-issued Recording physical binding descriptors and B3B2B executes the bounded shader on SDL_GPU Vulkan. Slice 06 closes the interleaved pass graph and applies pending state only at actual original draw/pass entry; 07 integrates production GameClient traversal. Slice 08 records retail scenes without requiring hardware. Slice 09's device-gated tests remain opt-in. Reverting never writes or migrates retail or XDG user data. A partial producer/resource failure unwinds adapter resources and original draw instances before returning an actionable error.

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

## Post-slice-04C original texture-loader split

At the first category physical edge, original rigid meshes upload the
source-owned vertex/index buffers before texture/material commands. Original
W3D geometry indices are 16-bit and use per-draw first-index/base-vertex;
M14's public `DrawDesc` had only a count, while SDL_GPU hardcoded 32-bit
indices and zero offsets. That narrow GpuDevice capability must be corrected
without rebasing geometry. The accepted CPU texture provider presently owns
identity/filter metadata only: `TextureClass::Init`/`Apply` are typed
unavailable and the original native `TextureLoader` supplies image pixels.
A buffer-only first-edge witness therefore cannot claim shader, sampler,
material or pixel translation. Previous 05 splits into 05A (public index
contract and exact original buffer uploads) and 05B (original image loader
and texture/material/shader physical entry); original source call order and
the final 06–09 acceptance remain unchanged. The complete revised packet is
committed as plans only before 05A is accepted. No fake pixels or fallback
texture may stand in for the original loader.

## Post-slice-05A canonical image-provider split

The original `TextureClass::Init` delegates foreground and background load
requests to `TextureLoader`; its reached provider closure includes original
`Targa` image decoding and `DDSFileClass` DXT mip/block parsing. The existing
CPU configuration retains only `TextureClass` identity and filter metadata;
there is no valid pixel source to upload until those original providers work
on Linux. The image formats and missing/malformed/unsupported cases have an
independently testable CPU boundary. Thus the former pending slice 05B is
superseded by 05B1 (canonical original Targa/DDS providers, no device frame)
then 05B2 (original loader and source-issued physical texture/material/shader
commands). Accepted 01–05A are unchanged, and downstream 06–09 retain their
full acceptance. `TARGA.CPP`, `ddsfile.cpp`, and `textureloader.cpp` remain
canonical providers, without a duplicate adapter-owned decode or invented
fallback pixels. This is a plan-only checkpoint before provider acceptance.

## Post-slice-05B1 source-owned format/bitmap/loader boundary

Canonical `TextureLoader::Get_Texture_Information` consults original
thumbnail metadata or original DDS/Targa providers, then WW3D requested
reduction and minimum texture dimension; its compressed/uncompressed begin
methods call original `Get_WW3D_Format`/`Get_Valid_Texture_Format`, device
texture size normalization and mip policy *before* original
`DX8Wrapper::_Create_DX8_Texture`. Original `BitmapHandlerClass::Copy_Image`
owns conversion/mip output before physical upload. `ww3dformat.cpp` also
contains DX8 capability lookups; `textureloader.cpp` owns a large native
thread/surface state machine. A first device-creation witness cannot prove
that original producer behavior without restoring those reached CPU methods
first. Former pending 05B2 is superseded by 05B2A (original format/bitmap/
loader CPU decisions and typed physical edge), then 05B2B (first source-issued
physical texture lifecycle/material/shader). Preserve the authored order and
normal 05B2 outcomes/negatives; no image bytes, mip choice, material, or
shader state may originate in an adapter. Accepted 01–05B1 and downstream
06–09 are unchanged. This checkpoint is committed as plans only before
production changes.

## Post-slice-05B2A physical texture/stage boundary

Accepted 05B2A proves original metadata and `Begin_Compressed_Load`/
`Begin_Uncompressed_Load` before typed creation. The next reached original
methods are `TextureLoadTaskClass::Lock_Surfaces`, `Load_Compressed_Mipmap`/
`Load_Uncompressed_Mipmap`, and `Unlock_Surfaces`; those methods produce the
actual DDS/Targa decoded mip bytes, whereas the separate `TextureClass::Apply`
and DX8 renderer material/shader calls require stage binding that the
current public `GpuDevice` does not yet expose. One bounded slice cannot
accept the source-owned mip output and the later stage consumer as a single
independently testable transition. The former pending 05B2B is superseded
by 05B2B1 (source-owned mip/decode, device-edge staging/create/upload and
lifetime) then 05B2B2 (source-issued filter/stage/material/shader state
through a narrowly extended public Recording/SDL contract). Staging must
not select pixels; no dummy draw or pass may force stage binding. Both
slices together retain all original 05B2B acceptance, including failure
controls, and 06 now depends on 05B2B2. Accepted 01–05B2A and remaining
07–09 are unchanged. Commit these plans alone before production edits.

The source-order trace found `TextureLoadTaskClass::Apply_Missing_Texture`
publishes an authored `MissingTexture` image for absent ordinary textures.
05B2B1 therefore preserves that original optional path; the adapter may not
invent fallback pixels. Required-resource failure is instead enforced by
the owning scenario load/reset/retry boundary in 08, without changing the
original loader's intermediate publication semantics.

## Post-slice-05B2B1 source-stage split

Accepted `4ec52aa` restores source-owned pixel publication, but original
`DX8TextureCategoryClass::Render` interleaves texture stages, material,
shader, mesh transforms, alpha override and the actual draw. Its Linux
branch still stops before that category body. `TextureClass::Apply` and
`TextureFilterClass::Apply` are independently testable stage/sampler
decisions, while original `VertexMaterialClass::Apply`, `ShaderClass::Apply`
and delayed `DX8Wrapper` states require a separate typed fixed-function
translation. The former 05B2B2 becomes 05B2B2A (source texture/filter,
stage/sampler pending state) and 05B2B2B (source material/shader, exact
blend/depth/cull/fog/combiner choices). Original 06 then exercises the
entire interleaved category body and applies those pending states at its
actual pass/draw, not at a fabricated earlier pass. Both new slices retain
all prior positive/negative 05B2B2 gates and no retail or full-frame claim;
06 now depends on 05B2B2B. The original source and ABI selection do not
fork. Commit this plan-only revision before production edits.

## Post-slice-05B2B2A material/mapper/shader closure

Accepted `fbde148` preserves original stage/filter state without a pass.
Source inspection reveals that `VertexMaterialClass::Apply` invokes original
mapper `Apply` methods (including reached screen/linear-offset W3D fixture
families), whose Linux `mapper.cpp` DX8Wrapper shim currently throws;
`ShaderClass::Apply` has a separate long caps-dependent blend/alpha/fog/
two-stage combiner/depth/cull path, also typed unavailable on Linux.
Original `DX8Wrapper::Set_Texture/Material/Shader` retain references and
mark delayed changes that `Apply_Render_State_Changes` consumes at the
original draw. SDL_GPU does not supply a fixed-function pipeline, so
physical shader/pipeline lowering is another independent operation.
Former 05B2B2B becomes B1 original mapper/material and ref-counted delayed
state, B2 original shader/capability/combiner decisions, B3 Recording/SDL_GPU
shader/pipeline/uniform lowering. Required shader variants are selected by
read-only retail family evidence; an unsupported required variant is a
contract escalation, never a skipped frame or substitute generic shader.
06 must still execute category Render in interleaved source order, and
depends on B3. Acceptance of A and the ultimate M22 contract are unchanged.
Commit this plan-only change before B1 production edits.

## Post-slice-05B2B2B2 dynamic-layout closure

Accepted B1/B2 retained original vertex/material/shader producers but the
public SDL_GPU `world_mesh` layout still hardcodes position/normal/UV. The
canonical `DX8FVFCategoryContainer::Define_FVF` uses `FVFInfoClass` offsets
and stride selected from optional normal/diffuse/UV counts. Reinterpreting
those source buffers through `world_mesh` corrupts the original geometry.
Therefore the former B3 is superseded by B3A, a public dynamic vertex-layout
contract with Recording/SDL parity, unsupported-format rejection and an
actual indexed validation-layer Vulkan layout test, followed by B3B, exact
shader/pipeline/uniform lowering of original pending state. Do not repack
geometry, change original FVF selection or invent private Vulkan commands.
The layout probe is not a source-driven pass, which remains 06; retail
scene/visual acceptance remains 08/09. This plan-only revision precedes
either B3 implementation, preserving all previously accepted contracts.

## Post-slice-05B2B2B3A category light/world ordering

Accepted `2c26632` proves canonical FVF physical layouts and indexed Vulkan
pixels, not an original category draw. Source inspection of
`DX8TextureCategoryClass::Render` shows its currently typed-unavailable
entry precedes source-owned light environment, world transform/identity,
normal normalization and actual category draw. B3B may lower original
shader/material/pending state already issued by source methods and probe
explicit original transform/light methods independently, but cannot claim
the category issued those later decisions. Slice 06 restores and tests the
complete original interleaving, including category-issued world/light
before pass/draw; 07–09 retail and visual gates are unchanged. This
plan-only clarification precedes B3B physical pipeline edits.

## Post-B3B source-state and physical-shader split

The original `DX8Wrapper` CPU branch owns a ref-counted delayed state with
distinct render, texture-stage, material and transform maps. Its source
`ShaderClass::Apply` and `VertexMaterialClass::Apply` must finish before any
public GPU shader/pipeline can truthfully represent those decisions. Former
B3B is superseded by B3B1, exact read-only applied-state snapshot and
semantic mapping/negative controls with no GPU resource, then B3B2, public
Recording/SDL_GPU shader/pipeline/uniform execution and validation-layer
pixels. Both retain the B3B acceptance together; category-issued world/light
and actual source draw remain 06, retail/visual gates remain 08/09. This
plan-only revision precedes further shader translation implementation. Owned
original W3D material fixtures emit lighting-enabled state even before the
category can emit its later light environment. B3B1 therefore preserves the
lit requirement in its semantic output; B3B2 must reject that physical
route until 06 restores source-owned category light/world decisions. An
unlit state issued by original methods supplies B3B2's physical positive.
B3B2 is further split before physical implementation: B3B2A constructs and
verifies only original source-issued Recording pipeline/uniform/stage bindings
with owner/generation/error controls; B3B2B adds exact GLSL/SPIR-V SDL_GPU
pixel semantics for required zero-, one- and two-texture stage families and
Khronos validation. Together these retain the former B3B2 acceptance;
neither substitutes for 06 original category pass/draw ordering.
The first real validation-layer probe found Vulkan VUID-07904 when a shader
declares original FVF attributes the source-selected buffer does not contain.
B3B2B therefore compiles exact bounded vertex-input variants for reached
canonical FVF families; it rejects unsupported families and does not add
dummy geometry, assume unavailable Vulkan features or disable validation.

## Deferred follow-ups

- Interactive UI/media and user input remain M23.
- Full playable-session evidence remains M15.
