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

## Post-08F0 resumption design (not implemented)

The accepted 08F0 selector remains a post-parser stop. A separate explicit
`ZH_M22_GENERATED_CONSTRUCTION_ROUTE` selector, valid only with the 08F0
generated scene/Recording/config/reset conjunction, admits the existing
original terrain-load call. The next source stage is determined by generated
mission/skirmish controls; no retail provider is opened. Any missing owner
reached after terrain load is a prerequisite rather than a selector-local
bypass. Evidence uses fixed stage categories and aggregate ownership only.
The generated-only probe exposed the missing disabled-water-grid query in
source `Radar::newMap`; [08F2](milestone_22_plan_01_slice_08f2.md) must be
completed first. The probe edits were removed at the discovery checkpoint.
The map-loaded `removeAllBibs` cleanup is a separate, selector-scoped 08F
handoff: the generated fixture has no bib producer and every creation API is
still fail-closed, so empty-list cleanup can be admitted with exact owner
checks without implementing an active bib owner or skipping a populated list.

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
