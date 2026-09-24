# M22 plan 01 slice 08F: redacted post-map scenario-construction closure

## Goal and outcome

Close the first source-owned boundary after the accepted 08B terrain route and
08E pre-map optional load-screen dispatch: the original two-phase new-game
path must proceed from its initial request into the map-loading/scenario
construction phase under the explicit test-only Recording selector.  This is
not retail-frame acceptance.  It provides only a source-faithful, redacted
owner/stage witness needed before slice 08 can claim campaign/skirmish
Recording output.

## Scope and non-scope

Included: the original second-phase `GameLogic` transition, only the reached
map/terrain/client owner handoffs, selector validation, failed required-owner
rollback, reset/retry and two independently provisioned generations.  The
test may emit fixed numbered stages and aggregate ownership/count booleans.

Excluded: retail names, paths, archive members, bytes, hashes, screenshots,
labels, pixels, final Recording-frame claims, unproven map services, and any
generated substitute for the retail consumer.  Normal Linux startup and the
accepted 08B early-stop route remain unchanged.  Any later source rejection
becomes a new dependency rather than a permissive fallback.

## Dependencies and state

Requires accepted 08B terrain admission/reset and 08E Linux no-owner
load-screen dispatch.  The scene selector is permitted only alongside the
accepted Recording factory, route and reset selectors.  Initial request,
second-phase map work, failure cleanup and final engine teardown must not
leave published terrain, scene, display, texture, track, water, shadow,
adapter or Recording owners.

## Validation

Positive: both runtime-only consumer selections and two fresh generations
reach the same redacted source stage.  Negative: missing scene/factory/route
selector, forced device acquisition failure, missing required provider, and
reset/retry all fail closed or recover with zero aliases.  Sanitized diagnostics
must classify a source stage only; they must not print captured process output.
Run focused GCC/Clang and sanitizer checks, source identity/provider/ledger,
the configured full matrix, strict host leak checks, proportional Vulkan and
serial LAN gates as applicable before the slice commit.

## Commit boundary

One independent commit: `delivery: M22 08F close redacted scenario construction`.
