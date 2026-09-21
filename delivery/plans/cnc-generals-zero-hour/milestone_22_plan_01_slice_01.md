# M22 plan 01 slice 01: original producer and device translation closure

## Goal and observable outcome

An original GameClient scene update reaches original W3D/WWShade draw instances and records its real resource, state, pass, and draw commands through `renderer::GpuDevice`. Every reached operation that was deferred in the RC-012 ledger either has implemented semantics or fails with a specific unsupported-state/asset error before a partial frame is accepted.

## Scope

- Bind the existing Linux original-engine factory to a presentation service backed by `renderer::GpuDevice` without changing original simulation authority.
- Port the original DX8Wrapper state/resource semantics and WWShade translation reached by rendering: transforms/camera, vertex/index resources, textures/samplers, material/blend/depth/cull/color/alpha/fog/light state, render targets, multipass ordering, uploads, draws, and explicit cleanup.
- Replace M21's physical fail-closed methods for every actually reached original concrete draw class with source-preserving constructor/update/draw/destructor behavior and adapter calls.
- Reconcile and update every M22-deferred dependency-ledger operation as it becomes implemented; preserve canonical M20 schema and original concrete class identity.
- Add recording positive and negative tests, failure unwind, source-identity, and provider-removal controls.

## Explicit non-scope

- Retail scene completeness, GPU presentation, screenshots, interactive UI/media, or full-session acceptance.
- A generic render module, copied schema, generated replacement scene, Direct3D compatibility layer, or private Vulkan implementation.

## Dependencies and ordering

- Requires accepted M20 schema/lifecycle, M21 simulation/concrete draw instances, and the M14 public renderer contract.
- Must complete before retail scene integration because recording establishes the semantic and failure contract used by both retail and hardware tests.

## Entry point and end-to-end behavior

The production Linux `GameEngine`/`GameClient` creates the existing original `Drawable` and concrete W3D draw modules from a project-owned map/object fixture. The original view/display traversal invokes their actual render state. A bound `RecordingGpuDevice` receives resources and ordered commands. A frame is accepted only after all required original producer operations complete and the pass closes.

## Data and state transitions

- Unbound -> presentation adapter bound -> original resources loaded -> frame state accumulated -> pass recorded -> frame complete -> reset/device-generation invalidation -> reverse cleanup -> zero resources.
- Any missing/malformed asset, unsupported required state, stale handle, incomplete pass, or injected allocation/upload failure transitions directly to failed/unwound with no successful-frame marker.

## Authorization and permission behavior

No privileged or user-data mutation applies. Fixture inputs are repository-owned and read-only during tests; outputs are temporary.

## Validation and error handling

- Positive: actual original concrete instances produce distinct resource/state/draw markers and balanced pass/resource ownership.
- Negative: missing/malformed model/texture/animation, unsupported reached legacy state, absent provider, stale resource after reset, and injected failure all reject with contextual errors and zero retained resources.
- Identity: compile/link and runtime checks name the original translation units, original concrete class types, canonical `W3DModuleFactory`, original GameClient consumer, and device adapter.
- Provider removal: omission of the original bridge/representative W3D provider/DX8 state provider must fail compile, link, or the runtime gate.

## Expected implementation surfaces

- `CMakeLists.txt`
- `docs/original-runtime-dependency-ledger.tsv`
- original W3D/WWShade/DX8Wrapper sources and headers under `GeneralsMD/Code/Libraries/Source/WWVegas/` and `GeneralsMD/Code/GameEngineDevice/`
- Linux device binding under `src/original_runtime/` and narrowly scoped public adapter headers under `include/zh/`
- `tests/original_rendering/`, identity/provider-removal tools, and owned fixture data
- `evidence/qa/cnc-generals-zero-hour/`

## Required validation commands

- Configure/build `linux-gcc-debug`; run focused `original-rendering` and existing `original-w3d`/`renderer-contract` labels.
- Run focused Clang Debug ASan+UBSan with strict halt/abort settings after behavior stabilizes.
- Run source identity, provider-removal, source-classification, ledger freshness, and `git diff --check`.

## Acceptance criteria

- Recording commands originate from actual original W3D/WWShade/GameClient instances and cover every reached deferred state/resource/draw operation.
- Canonical M20 schema/registry and M21 simulation behavior remain unchanged.
- All negative controls fail the production path; no ignored state, placeholder, no-op/generic module, alternate scene, or successful null operation exists on a reached path.
- Normal, reset, and all injected-failure paths return adapter and original ownership counts to zero.
- Focused tests and sanitizers pass and the ledger precisely records implemented versus still-unreached operations.

## Commit boundary

Commit the original producer/device translation, focused tests, ledger update, slice plan status, and slice evidence together as `delivery: M22 slice 01 bind original rendering`.
