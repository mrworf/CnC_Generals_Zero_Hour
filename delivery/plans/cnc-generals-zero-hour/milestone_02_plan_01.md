# M2: GPU-independent renderer API closure

## Outcome

The native-port graph gains a GPU-independent renderer contract, a mechanically checked classification of every legacy D3D/D3DX symbol, immutable and bounded pipeline descriptions, explicit coordinate and resize rules, and offline-compiled shaders for UI, terrain, water, points, and WWShade. SDL_GPU is the provisional backend, but this milestone opens no display or GPU device.

## Delivery-goal context

- Goal status: `workflow/delivery/state.yaml`
- Product ID: `cnc-generals-zero-hour`
- Governing contract: `delivery/milestones/cnc-generals-zero-hour/M2-renderer-closure.md`
- Product authority: `docs/zero-hour-linux-port-plan.md`, sections 6 and 8/M2
- Transaction start: `e17bfdba58ecf63ef2ba8a55c2b5402ccca10052`
- This companion transaction implements M2 only; the outer orchestrator owns milestone acceptance and status.

## Current-state findings

- M0 exposes `zh_w3d`, `zh_wwshade`, and `zh_renderer_sdl_gpu`, with explicit source lists and four canonical presets.
- M1 supplies fixed-width types and bounded codecs, but renderer targets still contain bootstrap code only.
- The legacy Zero Hour and shared Generals sources contain hundreds of D3D/D3DX identifiers, direct-device calls, state enums, formats, FVF declarations, runtime assembly paths, and WWShade code.
- Only a bootstrap vertex shader is compiled offline today; there is no engine-owned renderer API, mapping registry, or renderer-contract test label.

## Decisions

- Classify the complete source-observed D3D/D3DX symbol inventory using checked, documented mapping rules. A newly observed token without one unambiguous rule is a test failure.
- Publish only project-owned renderer types. SDL_GPU appears in the implementation mapping and backend declaration, never in engine-facing public headers.
- Express all pipeline-affecting state in immutable, comparable keys; validate resource bounds before device submission.
- Represent resize as an explicit state machine and keep coordinate/depth/color conventions as constants covered by tests.
- Compile one named vertex/fragment pair for each required shader family. A checked registry ties every effect family to source files and pipeline features.

## Slice index

| Slice | Plan | Outcome | Dependency | Status | Commit |
|---|---|---|---|---|---|
| 01 | [legacy renderer inventory](milestone_02_plan_01_slice_01.md) | Complete, fail-closed D3D/D3DX inventory and mapping table | M0 | completed | `3c2074d` |
| 02 | [engine renderer contract](milestone_02_plan_01_slice_02.md) | Descriptors, device interface, limits, pipeline keys, conventions, and resize state | 01, M1 | completed | this slice commit |
| 03 | [shader registry and offline compilation](milestone_02_plan_01_slice_03.md) | UI/terrain/water/points/WWShade shader closure across presets | 01, 02 | pending | pending |

## Cross-slice constraints

- No GPU, display, retail data, network access, Vulkan call, or private absolute path.
- No production source globbing; generated shader outputs stay in preset build trees.
- Public renderer headers remain engine-owned and contain no D3D, SDL, or Vulkan types.
- Unsupported legacy behavior is explicit and explained; no source-observed token may be silently omitted.
- M0 foundation smoke and M1 tests remain green.

## Milestone completion gate

- Configure and build all four canonical presets.
- Run `ctest --preset <preset> -L renderer-contract --output-on-failure` for all four presets.
- Run the inventory checker directly in positive and deliberately incomplete negative modes.
- Confirm all ten required shader modules compile to SPIR-V and every registry entry names generated outputs.
- Run the complete test suite in one canonical preset, plus `git diff --check` and a tracked-content private-path scan.

## Rollback and recovery

Each slice is independently useful and revertible. Reverting the shader registry does not alter the engine contract; reverting the contract requires reverting shader integration first. The legacy source tree remains unchanged.

## Execution notes

All slice plans were created before production edits and inspected together for complete milestone coverage, dependency order, positive and negative validation, and independently reviewable commit boundaries.
