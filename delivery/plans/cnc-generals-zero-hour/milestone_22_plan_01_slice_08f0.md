# M22 plan 01 slice 08F0: selector-gated post-reset source transition

## Goal

Close the narrow transition between accepted 08D's active-water-reset boundary
and 08F's post-parser source construction.  The Linux engine must preserve the
08D fail-closed stop for ordinary active-water reset selection, while the
separate explicit generated test scene selector admits the existing original
new-game route through native `loadMapINI` completion and stops before terrain
load.

## Scope

Include one conjunctive selector check in the Linux engine, a fixed redacted
post-parser marker before terrain load, and an owned generated mission/skirmish
control. Positive evidence covers two independent consumer generations
reaching the same parser boundary. Negative evidence covers absent scene
selector, missing config/reset selector, device failure, provider removal,
reset/retry, and final zero ownership.

Exclude post-parser terrain load and map-specific client construction, retail names, paths,
bytes, hashes, visual/Recording/audio fidelity, screenshots, raw process
output, and any default-production behavior change.  08F1 owns the parser
provider closure; 08F owns the later source construction.

## Prerequisite discovered during implementation

At discovery, 08F1's generated parser/provider closure did not implement the
live `W3DDisplay::doSmartAssetPurgeAndPreload` handoff. The Linux display
method then threw a typed pending error, while the original method reads an
optional usage list and always purges assets with that exclusion list. A
selector-local no-op would skip a source ownership transition.
[08F0B](milestone_22_plan_01_slice_08f0b.md) completed that prerequisite
before this slice resumed.

The generated-only probe also showed that the accepted 08D reset leaves the
active water object detached.  The first F0A update tick reaches its update
and pre-map draw, and F0A's temporary draw scene collides with the live display
scene.  Resume 08F0 with exact selector-gated owner checks; do not reuse a
second scene or claim a frame before terrain load.  These trial changes were
removed pending the prerequisite.  See the
[discovery evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f0_discovery.md).

## Resumed implementation decision

Use `ZH_M22_GENERATED_SCENE_ROUTE` only with the existing bounded scenario,
original factory, Recording device, and 08D config/reset selectors. It does
not authorize a retail provider. Extend the accepted F0A update-owned second
phase only for that conjunction. Its temporary draw-owner wrapper borrows the
already-published display scene/assets without alias replacement or deletion.
After 08D detachment, only that exact selector may skip the detached water
update and pre-map frame; source reset and all other owner guards remain
authoritative. The original parser and smart-purge method run, then a fixed
marker and typed stop occur before terrain load. Any additional missing source
owner requires a separate prerequisite rather than a local no-op.

## Validation

Run only the generated configured mission/skirmish controls; do not invoke
the retail audit or traverse a retail provider. Require the exact accepted 08D
early stop when the scene selector is absent and the fixed post-parser marker
only when all selectors are present. Prove ordinary completion without those
selectors, missing selector/provider/device rejection, no terrain load, and
zero rollback owners/resources.
Run focused GCC/Clang and sanitizer checks, identity/provider/ledger, full six
configured non-GPU/non-LAN suites, strict host leaks, proportional host Vulkan,
and serial LAN before committing.

## Commit boundary

One independent commit: `delivery: M22 08F0 admit scene selector boundary`.

## Execution record

The generated-only selector now admits the F0A update-owned second phase after
08D reset while borrowing the live display assets/scene. The exact pre-map
tick keeps the detached water owner and issues no frame. Source
`GameLogic::loadMapINI` and 08F0B smart purge complete, then a fixed marker
and typed exception stop before terrain load. The [08F0 evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f0.md)
records two mission and two skirmish generations, negative controls, strict
leaks, six full suites, host Vulkan, serial LAN, and zero owner/resource
rollback. 08F post-parser construction remains pending as a separate slice.
