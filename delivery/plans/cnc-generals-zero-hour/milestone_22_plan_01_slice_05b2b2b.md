# Superseded M22 plan 01 slice 05B2B2B: original material and shader state

This former single slice is now three dependency-safe commits:
[05B2B2B1](milestone_22_plan_01_slice_05b2b2b1.md) for source mapper,
material and delayed DX8Wrapper ownership,
[05B2B2B2](milestone_22_plan_01_slice_05b2b2b2.md) for source shader and
combiner decisions, and
[05B2B2B3A](milestone_22_plan_01_slice_05b2b2b3a.md) for canonical FVF layout, then [05B2B2B3B](milestone_22_plan_01_slice_05b2b2b3b.md) for physical public
GPU shader/pipeline lowering. Together they retain all acceptance below.
No standalone 05B2B2B commit is permitted.

## Outcome and dependencies

Requires accepted 05B2B2A. Original `VertexMaterialClass::Apply`,
`ShaderClass::Apply` and reached `DX8Wrapper` pending-state methods own
material, blend, depth, cull, fog, combiner and supported WWShade path
choices. A narrowly typed public contract translates their exact
source-issued values to Recording and SDL_GPU Vulkan-capable pipeline,
uniform and stage state, preserving source order. No authored pass graph,
draw, generic substitute shader or retail frame is claimed until 06–09.

## Canonical source and translation

Preserve original source method branches/field defaults, immediate versus
delayed state application, material null/reset and alpha override. The
source's fixed-function shader and material decisions precede the device
edge; only the edge maps their outputs to supported public blend/depth/
stencil/cull/fog/combiner/shader features. If a legacy operation cannot be
represented honestly by SDL_GPU, fail closed with its source operation and
reachability documented; no silent no-op, generated material or packed
generic pipeline. Pending bindings are applied only by the actual original
interleaved category pass in 06. Do not reorder original `DX8Wrapper`
calls to precompute a pass graph. WWShade optional/disabled macro branch
selection must be evidenced without claiming a compiled-out pass.

## Acceptance and negatives

Owned W3D material and shader fixtures and direct original-class state
fixtures exercise exact source fields, method order, distinct blend/depth/
cull/fog/combiner and WWShade reached decisions, alpha and null/reset
routes with Recording parity and capability-bound SDL_GPU Vulkan checks.
Explicitly test unsupported material/shader/combiner/stage modes, absent
required owner, injected state-translation/pipeline-creation failures,
reset/recreation and bounded teardown/retry with zero source/device owners
and no dummy draw. Physical pass-time bind failure belongs slice 06.
Read-only retail aggregate classifies reached families without private
artifacts. Canonical provider-removal, no mixed schema/full ABI objects,
ledger freshness, GCC/Clang full suites, focused ASan+UBSan/LeakSanitizer,
source classification and `git diff --check` pass. Slice 06 must later
exercise original category Render in full source order before any complete
frame claim; 07–09 remain mandatory.

## Commit boundary

One independently validated commit:
`delivery: M22 slice 05B2B2B translate original material state`.
