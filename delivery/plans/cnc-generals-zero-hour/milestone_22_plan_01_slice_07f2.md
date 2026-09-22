# M22 plan 01 slice 07F2: original base terrain map/shroud binding

## Goal and observable outcome

Requires accepted 07F0 and 07F1. A canonical
`BaseHeightMapRenderObjClass` owner created under the accepted original display
and Recording edge owns its source `W3DShroud`, binds one logical
`WorldHeightMap` ref, initializes the map-derived shroud through the original
partition-cell decision, records bounded dimensions/minimum/maximum height,
resets, frees and re-enters without retaining either owner.

## Scope and non-scope

Implement only the Linux CPU branch of the original base terrain owner; do not
change class layout or the native branch. This child supplies map/shroud
ownership needed before the derived terrain mesh exists. Derived
`HeightMapRenderObjClass` vertex/index buffers, shoreline/extra-pass tiles,
lighting, collision, active shroud projection, terrain render/pixels,
`W3DTerrainVisual::load`, retail maps and production-default factory selection
remain typed pending. A base-owner success cannot count as a mesh or frame.

## Entry, state, errors and surfaces

The source probe establishes production-compatible `W3DDisplay` scene/assets
under a Recording `OriginalGpuEdge`, constructs a minimal test-derived original
base terrain owner, and parses the generated read-only logical map. The owner
binds only when width/height match the accepted map and extra visual-pass
preprocessing is disabled. Null/mismatched maps, duplicate binding and visual
preprocessing reject transactionally. `reset` clears shroud data but preserves
the bound map as in source; `freeMapResources` releases both binding and shroud
grid, and a second init succeeds. Destruction retires the source shroud owner
and all refs before display/edge teardown.

Expected surfaces are canonical `BaseHeightMap.cpp`, a focused source
probe/test, full-draw registration, dependency ledger, plan/index and evidence.
No retail input or symlink content is accessed or changed.

## Tests and acceptance

Positive tests cover source shroud construction, matching map ref ownership,
height extrema, reset-preserved map, explicit free, re-entry and two fresh
processes. Negative tests cover missing edge/display, null or mismatched map,
duplicate init, visual preprocessing and base render/mesh methods, with stable
refs and allocation counts after each failure. Run focused GCC/Clang,
identity/provider-removal/ledger controls, exact five non-LAN suites with
leak-capable GCC/Clang ASan+UBSan/LSan, and the established LAN split. Recording
commands must remain empty; no physical pixel gate applies to this CPU owner.

## Commit boundary

One commit: `delivery: M22 slice 07F2 bind base terrain map data`.

## Result

Complete. The canonical base terrain owner now owns its source shroud, binds
and releases one logical-map ref, derives extrema and shroud dimensions, and
supports reset/free/re-entry without emitting a draw. The focused probe keeps
the accepted production default unchanged: a zero partition cell size rejects,
then a probe-local representative `MAP_XY_FACTOR` setting demonstrates the
source prerequisite and is restored before teardown. Null, dimension mismatch,
duplicate binding and visual-preprocess inputs remain transactional failures.
Evidence: [milestone_22_slice_07f2.md](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07f2.md).
