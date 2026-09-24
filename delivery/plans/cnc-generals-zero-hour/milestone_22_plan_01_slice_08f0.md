# M22 plan 01 slice 08F0: selector-gated post-reset source transition

## Goal

Close the narrow transition between accepted 08D's active-water-reset boundary
and 08F's post-parser source construction.  The Linux engine must preserve the
08D fail-closed stop for ordinary active-water reset selection, while the
separate explicit test scene selector admits the existing original new-game
route only as far as its first native map-parser boundary.

## Scope

Include one conjunctive selector check in the Linux engine, fixed redacted
stage markers from the existing original source, and an owned audit control.
Positive evidence covers two independent consumer generations reaching the
same numbered parser boundary.  Negative evidence covers absent scene selector,
missing config/reset selector, device failure, provider removal, reset/retry,
and final zero ownership.

Exclude map-parser success, terrain/client construction, retail names, paths,
bytes, hashes, visual/Recording/audio fidelity, screenshots, raw process
output, and any default-production behavior change.  08F1 owns the parser
provider closure; 08F owns the later source construction.

## Prerequisite discovered during implementation

08F1's generated parser/provider closure did not implement the live
`W3DDisplay::doSmartAssetPurgeAndPreload` handoff.  The Linux display method
still throws a typed pending error, while the original method reads an
optional usage list and always purges assets with that exclusion list.  A
selector-local no-op would skip a source ownership transition.  Complete
[08F0B](milestone_22_plan_01_slice_08f0b.md) before resuming this slice.

The generated-only probe also showed that the accepted 08D reset leaves the
active water object detached.  The first F0A update tick reaches its update
and pre-map draw, and F0A's temporary draw scene collides with the live display
scene.  Resume 08F0 with exact selector-gated owner checks; do not reuse a
second scene or claim a frame before terrain load.  These trial changes were
removed pending the prerequisite.  See the
[discovery evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08f0_discovery.md).

## Validation

Run the existing redacted configured audit without entering a retail scene,
and two generated selector controls;
require the exact accepted 08D early stop when the scene selector is absent and
the fixed first parser-boundary marker only when all selectors are present.
Run focused GCC/Clang and sanitizer checks, identity/provider/ledger, full six
configured non-GPU/non-LAN suites, strict host leaks, proportional host Vulkan,
and serial LAN before committing.

## Commit boundary

One independent commit: `delivery: M22 08F0 admit scene selector boundary`.
