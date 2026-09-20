# M10 delivery plan: effects, water, and WWShade command generation

## Goal

Close M10 by giving every source-required effects family a checked legacy-to-GLSL/pipeline mapping and an asset-free recorder path that proves render-pass ordering, state preservation, bounded caching, and deterministic lifetime cleanup.

## Authority and constraints

- Milestone contract: `delivery/milestones/cnc-generals-zero-hour/M10-effects.md`.
- Product authority: `docs/zero-hour-linux-port-plan.md`, especially M10 and renderer sections.
- Direct providers M4 and M7 are complete at transaction start `a314721c3ce122792cad50a92c603a495139d3d6`.
- The retail corpus is read-only. Durable corpus evidence contains logical names and expectations only, never retail bytes or host paths.
- No GPU, display, retail-data access, or network is required by these slices; real-pixel validation remains M14.

## Delivery slices

1. [Slice 01: checked effects mapping and offline GLSL closure](milestone_10_plan_01_slice_01.md)
2. [Slice 02: deterministic effects command recorder](milestone_10_plan_01_slice_02.md)

The slices are dependency ordered: the recorder consumes the checked mapping and compiled shader table delivered by slice 01.

## Milestone validation

After both slices, configure, build, and run `renderer-contract` for all four canonical presets:

```sh
cmake --preset <preset>
cmake --build --preset <preset>
ctest --preset <preset> -L renderer-contract --output-on-failure
```

Acceptance requires every manifest entry to resolve to compiled stage sources, every scoped effect to produce a valid normalized command sequence, unknown effects to name their logical asset and material, render-target and multipass order to be explicit, and teardown/cache bounds to pass.

## Non-scope

Hardware pixels and validation layers (M14), world geometry behavior (M9), retail redistribution, and changes to gameplay simulation are excluded.

## Commit record

- Slice 01: `552341db0bb415fef87598e7224166e5780a3503`
- Slice 02: `25654e19f9b35e9ea9880a59b7f8099a5c613104`
