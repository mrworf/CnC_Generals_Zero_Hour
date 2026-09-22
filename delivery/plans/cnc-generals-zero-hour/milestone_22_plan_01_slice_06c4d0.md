# M22 plan 01 slice 06C4D0: original material-pass abort ownership

## Outcome and boundary

Requires accepted 06C4C. Linux source `WW3D::Render` abort invalidates original FVF containers after physical upload/draw failures. Their queued visible and delayed `MatPassTaskClass` records each hold a mesh and pass reference; original container destructors do not retire those lists, so retries accumulate refs and scene teardown leaves stale owners. Retire only these abandoned source tasks during Linux container invalidation, without changing successful flush order, class layout or Windows behavior. This is an independent C4D prerequisite, not complete C4 acceptance.

## Entry, negatives and verification

Use generated source-only material passes through an original scene and inject a dynamic APT upload failure and a procedural draw failure after a successful positive draw. Assert abort closes the source frame, invalidation returns mesh/pass refcounts to scene-owned values, and a fresh frame rebuilds each pass without old-task duplication. Cover immediate and delayed queues, successful positive flush and repeated generations. Negative controls must fail before the fix. Run focused GCC/Clang Debug and sanitized source-only tests, full four asset-free suites, source identity/ABI/provider/ledger checks; one isolated plan/code/test/evidence commit. Do not change retail or production shader configuration.
