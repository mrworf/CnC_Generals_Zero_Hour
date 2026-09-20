# M14 slice 03: representative scene matrix and acceptance evidence

## Goal and observable outcome

One opt-in hardware run exercises the required UI/font/world/faction/effect/video and lifecycle matrix on the real SDL_GPU backend, emits project-owned captures/metrics, and records a truthful PRE-013 disposition with validation-layer status.

## Scope

- Drive representative UI/font, terrain, USA/China/GLA, water, particles, shadows, roads/decals, trees, stealth, WWShade, and movie presentation cases.
- Check and report coordinate handedness, depth range, winding/culling, half-pixel placement, color packing, premultiplied alpha, fog, depth bias, texture origin, and A/V presentation behavior.
- Emit deterministic project-owned image/metric outputs and a redacted acceptance report with versions, capabilities, timings, lifecycle results, validation discovery/output, deviations, and dispositions.
- When retail tests are enabled, verify the logical corpus through configured VFS roots without copying retail content or recording private paths.

## Non-scope

Mandatory second-GPU certification, backend replacement without a demonstrated public gap, release packaging, and playable-game acceptance are excluded.

## Dependencies and ordering

Requires slices 01-02 and completed provider milestone evidence. Final PRE-013 acceptance also requires `VK_LAYER_KHRONOS_validation` to be discoverable and enabled.

## Entry point and end-to-end behavior

The M14 acceptance executable runs a named scene matrix, presents/captures project-owned frames, computes stable metrics, performs lifecycle transitions, and writes a machine-readable report to an explicitly supplied output directory. A repository tool converts only redacted/stable facts into the checked-in evidence report.

## Data/state transitions

Each scene is pending -> rendered -> measured -> disposed, or failed with a named stage. The overall disposition is accepted only when every required scene/lifecycle case passes and the enabled Khronos validation stream has zero errors; otherwise it is blocked/failed with retained safe evidence.

## Authorization and permissions

No product authorization applies. Retail reads require explicit `ZH_ENABLE_RETAIL_TESTS` and caller-owned roots. Captures must contain only generated/project-owned pixels.

## Validation and recovery

- Positive: every named scene and semantic review item has a result; output contains no retail bytes/private paths; timings meet the documented functional threshold.
- Negative: absent validation layer, absent GPU/display, unwritable output, missing retail roots when opted in, incomplete scene matrix, capture mismatch, and validation messages prevent acceptance with actionable diagnostics.
- Re-running replaces only caller-designated generated output and leaves sources/retail inputs untouched.

## Expected implementation surfaces

GPU acceptance test/tool, scene fixtures, CMake test registration, and `evidence/qa/cnc-generals-zero-hour/M14-gpu-acceptance.md`.

## Validation commands

- All four: `cmake --build --preset <preset>` and `ctest --preset <preset> -L 'gpu|ui|video' --output-on-failure`.
- Opt-in GPU and retail+GPU builds as defined in the governing plan.
- `vulkaninfo --summary` and a layer discovery probe; absence of Khronos validation is recorded, never treated as success.

## Acceptance criteria

- All required scene/lifecycle/semantic rows have hardware evidence and project-owned captures/metrics.
- Checked-in evidence is path-redacted and contains no retail content.
- PRE-013 is accepted only with zero errors from an enabled Khronos validation run; otherwise the committed safe work is handed back as acceptance-blocked.

## Commit boundary

One commit for the scene matrix, output checks, and truthful evidence/blocker record: `delivery: M14 slice 03 record GPU acceptance evidence`.
