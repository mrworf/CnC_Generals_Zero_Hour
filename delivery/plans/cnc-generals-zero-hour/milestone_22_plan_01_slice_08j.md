# M22 plan 01 slice 08J: detached active-water scene continuation

## Goal and observable outcome

Close the source lifecycle boundary reached when the accepted active-water
reset is followed by the native update-owned new-game phase. The reset leaves
the live water render object intentionally detached from the display scene.
Before game logic loads the next map, `GameClient::update` calls
`W3DTerrainVisual::update`; that exact pre-map state must be a valid no-op.
The later `W3DTerrainVisual::load` transaction must attach the same water
owner after terrain attachment, disable its grid, apply the accepted map
override, and unwind the attachment if any later load step fails.

This is a source-state contract, not a retail or generated scene-selector
exception. Once the owner is in the exact reset-detached/pre-map state, both
authorized callers receive the same behavior.

## Scope and non-scope

Replace the generated-selector-specific update bypass and construction-
selector-specific water reattachment with one bounded lifecycle predicate.
It accepts only the published terrain visual, terrain render object without a
map, published active water owner detached from the primary scene, active GPU
edge, ready water resources, and otherwise coherent original owners. The
pre-map update performs no mutation or device operation. Map load attaches
terrain first and that exact detached water owner second, then calls the
already accepted disabled-grid and no-river override owners in native order.

Reject missing or foreign terrain/water/display owners, pending resources,
already or incorrectly attached water, disabled or malformed water state, and
post-map use of the pre-map no-op. On file, terrain, grid, override, or later
construction failure, remove only scene links added by the current load,
release partial map resources, and leave the reset-detached owner retryable.
Repeated update before load is idempotent; retry attaches exactly once.

Do not admit the slice-08 retail selector, alter the accepted 08F0 stop, add a
new rendering family, implement river textures or active water grids, relax
the normal attached-water update guard, or use retail/private input. Existing
08F2 disabled-grid and 08F3 map-override contracts remain authoritative.

## Entry, state transitions, and permissions

Entry is the generated terrain-water probe after the accepted display detach
and terrain reset. State moves from attached active owner, through display
detach and successful source reset, to an exact detached pre-map owner.
Updates retain that state. A successful visual map load publishes terrain and
water in primary-scene order; failure returns to detached pre-map state;
teardown retires every alias and Recording resource. Authorization is not
applicable beyond the existing test-only factory/config/reset selection; the
new behavior is derived solely from live source ownership state.

## Implementation and validation

Expected production surface is `W3DTerrainVisual.cpp`; extend the existing
generated terrain-water probe and Python wrapper. Positive cases cover
repeated pre-map update, terrain-then-water attachment order, disabled-grid/
map-override order, successful map load, reset/re-entry, retry, and two device
generations. Negative cases cover each predicate member, already/foreign
attachment, pending resources, disabled/malformed water, failed terrain
construction, failed grid/override, and teardown after every failure. Verify
the accepted generated construction route still passes and its absent-selector
stop remains unchanged.

Run focused GCC and Clang tests; sanitizer focus; all six canonical nonretail
suites with `-LE 'gpu|lan|retail'` and `ASAN_OPTIONS=detect_leaks=0` for
sanitizer suites; strict host GCC and Clang leak checks; physical Vulkan;
serial LAN; dependency ledgers; and `git diff --check`. No retail corpus is
used until this prerequisite is accepted and slice 08 resumes.

## Acceptance and commit boundary

The update/load behavior is selector-independent, exact-state bounded,
transactional, retryable, and fully retired at teardown; all negative controls
fail before mutation or restore the detached retry state. One independently
reviewable implementation commit:
`delivery: M22 08J continue detached active water`.
