# M22 plan 01 slice 07E0D1: original empty bib cleanup

## Outcome and dependency

Requires accepted 07E0B original no-map terrain composition. The original Linux `W3DTerrainVisual::removeAllBibs` must preserve the native no-owner/no-map cleanup behavior when `InGameUI::destroyPlacementIcons` calls it with zero icons during reset and destruction. This is a prerequisite to production-order original factory startup, not bib rendering support.

## Boundary and failure

An uninitialized visual with no terrain render owner returns without mutation, following the native null-owner branch. A published original visual with the accepted `HeightMapRenderObjClass`, no loaded map, and no created bibs also returns idempotently. Any mismatched/published owner, loaded map or active bib creation mode remains fail-closed. All add/remove/highlight bib methods remain typed pending; this slice creates no bib storage, map data or draw calls. Native Windows branch and class layout do not change.

## Acceptance

The initialized GameClient test calls the real `InGameUI::placeBuildAvailable(NULL,NULL)` cleanup with original visual published, repeats `removeAllBibs` and verifies owner refs/no source draw. It rejects unsupported bib creation and mismatched publication while preserving positive empty frame and teardown. Add source/physical controls, five rebuilt non-GPU suites including leak-capable GCC/Clang sanitizers, repeated host Vulkan, identity/ledger checks, and one independent evidence/commit. The pending 07E0E startup edits stay unstaged.

## Result and commit boundary

The source-order control checks that original `GameClient::~GameClient` deletes UI before terrain visual before display, and that `InGameUI::~InGameUI` reaches placement/bib cleanup. The runtime fixture invokes that real cleanup with the no-map original owner and rejects mismatched publication and active bib creation. A separate pending E0E production-order diagnostic returned from actual original GameClient teardown without a bib exception; it remains diagnostic, not D1's acceptance dependency, because its GPU-device allocation policy is pending. The exact D1-only tree passes five 194/194 full suites, repeated physical validation, clean ledger and `git diff --check`. Evidence: [07E0D1](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07e0d1.md). This commit excludes the E0E factory and unrelated renderer diagnostic.
