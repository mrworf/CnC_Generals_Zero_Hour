# M22 plan 01 slice 08P0C2A0: exact reflected uniform block origins

## Goal, dependency and boundary

After accepted C1 and before C2A, correct the independently owned public-bgfx
uniform-field binding origin. Preserve source GLSL, public per-source-block
uniform ABI, generic/tree program semantics and ordinary device behavior.
No tree program, C1 advancement, scene draw, factory, retail or frame-command
transaction is admitted. D0A/B remain separate later owners. Generated local
source/build/physical tests have no additional authorization boundary.

## Confirmed source defect and exact ownership

The generated `original_applied_3.frag` envelope reflects stage_ops at byte128,
stage_args160, alpha_parameters96, fog_parameters80 and fog_color64 in its
source block. Those are absolute merged-stage offsets; its actual source block
starts at zero. BgfxGpuDevice currently subtracts the lowest live field (64)
as an inferred block base, so stage controls read byte64 instead of byte128.
The exact generated textured tree control produced 338 accepted pixels with
constant green100:100 and no shroud variation. The separate vertex program's
first live field is at zero and its sway/darkening/alpha/matrix pixels passed.
Recording is not affected. This is not an accepted shader fallback or a tree
geometry defect; prior controls lacking dead-prefix coverage cannot waive it.

Use compiler-authoritative merged-stage block origins, not the first live
field and not a source-name special case. Extract/validate exact SPIR-V root
block/member offsets against the owned source-block manifest, including blocks
with optimized-out prefixes or holes and multiple source bindings. Persist a
bounded versioned origin contract with generated/installed shader metadata.
At load, reject missing, stale, duplicate, mismatched, underflow/overflow or
out-of-block field metadata before native shader publication. Subtract only
the admitted actual source-block origin to bind the caller's unchanged source
UBO bytes. Do not compact an unused prefix or repack source fields.

Retain stage/binding identity, array/matrix/int payloads, padding and explicit
resource generations. Preserve existing shader binary offsets, descriptor
slots and uniform identifier normalization. Any further independently required
owner gets a plan-only split before production. Candidate metadata/shader
failure releases only its owned resources and leaves prior accepted programs,
buffers and target contents available; clean retry works without reset.

## Surfaces and acceptance

Expected surfaces: build-only bgfx layout metadata extraction/verification,
shader build/install inputs, bounded public-bgfx manifest/field admission and
isolated generated renderer witnesses. The unrelated renderer diagnostic is
not this slice's test surface and remains unstaged. No allocator changes;
original-engine physical fixtures must initialize shipping process services
before workers and retire devices before those services.

CPU positives prove actual block zero with dead prefix, interior holes,
nonzero later block origins, multiple bindings, integer vectors, matrices and
arrays. Negatives cover malformed/truncated/duplicate origins, unknown blocks,
stage/binding mismatch, optimized-out blocks, offset underflow/overflow,
capacity, missing/stale metadata and rejection before publication. Physical
Vulkan proves declared texture quadrants, alpha-test rejection and fog payloads
using original fixed-function fragment state plus independent multiblock
controls; compare exact source byte ranges and two-generation resource/retry
identity. Prior accepted pixels remain on metadata rejection. Do not disable
validation or substitute Recording for physical proof. Persist exact focused
commands before edits; run GCC/Clang native and sanitizer focused controls,
six complete builds/canonical nonretail suites (`-LE 'gpu|lan|retail'`,
sanitizer `ASAN_OPTIONS=detect_leaks=0`), exact serial host strict LSan
(`ASAN_OPTIONS=detect_leaks=1`, no UBSan override), established physical
Vulkan, serial LAN4/4 all six, ledger and diff on final source.
Commit independently: `delivery: M22 08P0C2A0 bind exact shader block origins`.

## Implementation admission and focused commands

Share one bounded CPU-only shader-envelope/SPIR-V decoder between build-time
metadata emission and runtime admission. A versioned `.layout` sidecar records
actual root source-block origins/extents; runtime cross-checks it against the
loaded binary and the existing source-binding manifest before native creation.
Offline closure also checks that contract. Never infer an origin from live
fields; preserve source field offsets/padding and compiled payload bytes.

Focused: `ctest --test-dir build/<preset> -R
'^(renderer_bgfx_uniform_layout|bgfx_shader_family_closure|original_w3d_shader_source|original_headless_update)$'
--output-on-failure`. Physical: isolated `renderer_bgfx_uniform_layout_tests
--gpu` under the established graphical host environment and
`VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, wrapped by
`tools/run_validation_clean.py`. Strict host LSan selects
`renderer_bgfx_uniform_layout|original_w3d_display_owner` with detect_leaks=1,
no UBSan override. Existing canonical/physical/LAN gates remain required.

## Implementation checkpoint

Compiler/runtime origin admission and generated CPU/physical controls are
accepted: six complete builds and six canonical suites pass 268/268 each,
with focused, strict host LSan, physical Vulkan, serial LAN and ledger/diff
gates on the refrozen source.
[Acceptance evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c2a0.md).
No tree program or scene/factory/retail admission is included in this slice.
