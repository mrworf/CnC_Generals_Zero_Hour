# M22 plan 01 slice 06C: original sorting/static/WWShade complete frame

Dependency-safe delivery decomposition: 06C1 restores original sort snapshot/refcounts and CPU queue/pool ordering to a typed first physical edge; 06C2 restores physical sorting VB/IB, sorted nonpool/pool draw and error rollback; 06C3 restores original WW3D camera/static-sort/sort frame scheduling and device boundary; 06C4 audits authored WWShade configuration/families and proves this entire original interleave on Recording and validation-layer Vulkan. Each is independently tested and separately committed; only 06C4 accepts the unchanged aggregate below. No duplicate sorter, generated scenes or unearned full-06C claim.

## Dependency and behavior

Requires accepted 06B. Restore reached `SortingRendererClass::Insert_Triangles/Flush`, original static sorting and `WW3D::Render`/`Flush` scheduling, exact enabled/disabled `USE_WWSHADE` compilation branches. When enabled, original WWShade SHDMESH and reached legacy source shader-family routes must register, select and render; when disabled, negative branch witness and original regular material/effect scheduler still supply reached semantics. Preserve original interleaving and blend/sort depth/texture/material decisions. Read-only retail aggregates identify the enabled required shader-family closure without storing private identifiers, bytes or hashes. Do not replace original producers with recorder-owned geometry/material or test-only proxy passes.

## Acceptance

Owned multi-pass W3D fixtures produce complete source-owned rigid/skin/decal/procedural/static/sorting/WWShade-selected Recording frames in exact authored source order with positive/negative shader-family coverage, valid public draw descriptors and bounded teardown/recreation. Reject unsupported required family, missing asset, wrong sort/order, injection at stage/bind/draw/flush, stale generation and partial frame; prove source reset/retry. Preserve full 06 aggregate acceptance; no GameClient or retail-frame claim. ABI/link, identity, provider-removal, full GCC/Clang suites, focused sanitizers and ledger pass before one 06C commit. Slice 07 starts only after this gate.
