# M29 slice 02 — affected producer and source-inventory reconciliation

## Goal and observable outcome

The original W3D camera-scoped clear shape is expressible through the source-facing CPU edge and is verified without retail content; every required legacy renderer category has an explicit bgfx-public, retained CPU/shader or fail-closed disposition.

## Scope and non-scope

Connect only settled command intent and update mapping/ledger and affected CPU tests. Do not deliver complete retail scenes or a physical device. Do not reset M2/M7–M10 history.

## Dependencies and ordering

Requires slice 01 clear operation. Original `WW3D::Render` applies camera viewport and `DX8Wrapper::Clear` in an active frame; M22 will complete its actual production draw route.

## Entry point and behavior

`OriginalGpuEdge` translates active camera rectangle and original independent clear flags/values to the new device command. If source timing or dimensions are invalid, it propagates a source-context error before any clear reaches the device. CPU recording tests assert exact command order around representative draws. Inventory checker fails if a source category is unknown or multiply classified; ledger names changed boundary and preserves already accepted CPU behavior.

## Data/state, authorization and errors

Source edge keeps the current active frame/viewport and target generation, resets on pass end/abort, and never silently clears an entire target. No user authorization applies. Invalid active state, unsupported source format or unhandled category fails closed with traceable error.

## Surfaces

`src/original_runtime/original_gpu_edge.{h,cpp}`, relevant original CPU tests, `docs/renderer/legacy-api-mapping.tsv`, renderer contract and source dependency ledger, inventory tests/tool if an actual mapping gap is found.

## Tests and validation

- Positive: source-shaped camera viewport clear after first draw and before second, distinct color/depth/stencil flags, unaffected UI/world/effects recorder snapshots.
- Negative: no active frame, invalid viewport/generation, unsupported format/category; no partial clear.
- Run renderer inventory, original source/CPU tests, renderer recorder/UI/world/effects focused CTest, public-header scan and relevant Python tests.

## Acceptance and commit

Meets M29 M2 and M7–M10 affected-category preservation with source-to-command traceability. One reviewable commit includes this plan, code/tests/docs and focused evidence.

## Delivered evidence

[Focused source and inventory evidence](../../evidence/cnc-generals-zero-hour/milestone_29_slice_02.md). Commit identity is the commit containing this slice file; the transaction handoff records its exact SHA without a self-referential Git field.
