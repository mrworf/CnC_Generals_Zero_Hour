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
source `Radar::newMap`; [08F2](milestone_22_plan_01_slice_08f2.md) is now
accepted. The next native source chain in `W3DTerrainVisual::load` attaches
the water object to the primary scene, calls `enableWaterGrid(FALSE)`, then
calls `updateMapOverrides`. Accepted 08D reset leaves water detached, so 08F
owns exact reattachment before the disabled setter, with failure rollback.
The independent [08F3](milestone_22_plan_01_slice_08f3.md) prerequisite now
closes `updateMapOverrides`; 08F must retain native terrain → water attach →
disable grid → map override order. If any of those calls rejects, remove both
new scene attachments, release the uncommitted loaded map and terrain
resources, and leave no published visual map. The generated fixture combines
the existing authored logical map with a flat visual blend tile, a generated
terrain texture, and a positive partition cell size; those are fixture inputs,
not production defaults. Neither prerequisite admitted the 08F construction
selector. The earlier probe edits were removed at discovery checkpoints.
The map-loaded `removeAllBibs` cleanup is a separate, selector-scoped 08F
handoff: the generated fixture has no bib producer and every creation API is
still fail-closed, so empty-list cleanup can be admitted with exact owner
checks without implementing an active bib owner or skipping a populated list.
The post-08F3 generated-only trial reached fixed terrain-loaded and
radar-complete stages, then found a separate display shroud-refresh owner
boundary. [08F4](milestone_22_plan_01_slice_08f4.md) closes the source
`PartitionManager::refreshShroudForLocalPlayer` display/terrain handoff before
08F resumes. Its child plans separate the terrain notification dependency
from display clear/per-cell dispatch. All trial production/test edits were
removed; neither 08F4 child admits the construction selector.

## Post-08F4 resumption checkpoint

08F4A and 08F4B are accepted. The generated-only diagnostic temporarily
added a separate conjunctive construction selector, retained the accepted
08F0 post-parser stop when absent, and restored native terrain attach → water
attach → disabled grid → map overrides inside the terrain-load transaction.
Rollback removes both new scene links and the uncommitted map. The generated
fixture extends the authored logical map with only flat visual blend metadata
and a generated terrain texture. The known-empty map-owned bib cleanup is
selector-scoped so a later source failure could not be masked by teardown.
The probe used fixed stage categories after radar/shroud refresh without
retail input. Its trial code was removed after the missing owner was found;
the existing default, absent construction selector and 08D/08F0 stops remain
controls.

## Post-08F4 generated discovery and 08F5 dependency

The uncommitted generated mission trial reached terrain attachment, the
source-ordered water reattachment/disabled-grid/map overrides, radar new-map,
partition shroud refresh, terrain logic new-map, radar terrain refresh,
pathfinder new-map, and permanent-observer reveal. The first missing owner was
`W3DTerrainVisual::addProp` for the generated authored no-draw prop during the
source object loop. Its CPU method remains typed pending, while native source
resolves model conditions and only dispatches to the terrain prop buffer when
a model name is present. The generated prop has no draw module/model, so the
valid native result is a no-op; active modeled props require a separate
producer and must remain fail-closed. [08F5](milestone_22_plan_01_slice_08f5.md)
owns this bounded method independently; do not delete the generated prop or
skip source object construction to advance 08F.

The generated 8x8 vertex map's playable boundary is 7x7. A 10-unit partition
cell size landed on a reciprocal-ceil rounding edge (partition 8, shroud 7);
the generated-only non-edge 12-unit cell size yields matching 6x6 grids and
passed the accepted 08F4 dispatch. This was a fixture correction, not an
accepted owner or production change. The trial's later failure returned by
normal rollback with zero published graphics owners and zero Recording
resources. All trial selector, production and test edits were removed at the
plan-only checkpoint. Resume 08F only after 08F5 accepts the no-model prop
owner, then re-probe the next exact source boundary without retail input.

08F5 is accepted as an independent no-model prop owner. Its generated
negative controls leave an active modeled prop fail-closed. Resume this 08F
plan at the separate construction selector and source-ordered water/object
route; do not treat 08F5's direct-method acceptance as construction success.

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
