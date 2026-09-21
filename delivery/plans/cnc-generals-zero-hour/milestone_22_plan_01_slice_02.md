# M22 plan 01 slice 02: original GameClient CPU presentation ownership

## Goal and observable outcome

The original GameClient display, scene, asset-manager, shadow and terrain-track producers required by the ten retail-reached draw classes own their CPU lifecycle and decisions. A representative owned scene can acquire the original WW3D graph, attach/release original render objects, select an original shadow route and update original track edges without acquiring a physical GPU. Device operations are explicitly unavailable until slice 04.

## Scope and dependency closure

- Compile only reached original `W3DDisplay`, `W3DScene`, `W3DAssetManager`, `W3DShadowManager` and terrain-track CPU methods, plus transitive original WW3D scene/reference-counting providers. Preserve their source identity and the authored order of asset, scene, shadow and track decisions. OS/device adapters may terminate at physical resource operations; they may not implement shadow/track/scene policies or invent geometry/material.
- Canonical draw sources have two mutually exclusive build configurations: existing M20/M21 schema-only tests remain on `zh_original_config_providers`; M22 original production uses full-behavior objects without `ZH_W3D_SCHEMA_ONLY`, `ZH_W3D_HEADLESS_INSTANCE` or `BRUTAL_TIMING_HACK`. Compile both configurations from the same original source text, never link both definitions into one executable. Confirm class ABI and link-map provider selection. The production switch follows a green original runtime witness, not just a successful compile.
- Keep Direct3D headers/libraries absent from Linux binaries; where a legacy header contains only opaque device fields, forward declarations may preserve layout. Physical device methods must fail explicitly until the slice-04 `GpuDevice` translation.

## Validation and error handling

- Owned in-memory W3D fixture drives an original scene/asset-manager attach, hide/show or shadow route, track bind/edge/unbind and reverse release; assertions observe source-owned state before the device edge. Required missing original resources fail with actionable context, and a failed setup leaves no newly published scene ownership.
- Source compile/link identity and provider-removal negative tests cover original display, scene, asset-manager, shadow and track TUs. Link-map checker rejects both schema and full objects defining any of the ten classes in one binary and rejects a production fallback to the schema versions.
- Existing M20/M21 registry, headless simulation and WW3D CPU tests remain green in GCC/Clang; this CPU slice does not claim rendered frames.

## Acceptance criteria

- Original owners and CPU decisions, not boundary substitutes, produce scene attachment, shadow selection and track state. All reached required GPU calls are typed unavailable rather than no-op success.
- Full-class declarations/layout match across M22 production consumers; schema-only test binaries continue selecting their existing source configuration.

## Commit boundary

Commit original CPU presentation closure, focused positive/negative witnesses, identity/ABI gates, ledger/evidence and slice status as `delivery: M22 slice 02 restore original GameClient CPU owners`.
