# M22 plan 01 slice 08F1: original map-override parser/provider closure

## Goal

Deliver the smallest source-faithful prerequisite for 08F: the original
`GameLogic::loadMapINI` sequence accepts a bounded, project-owned map override
through its native parser and retains original optional-file behavior.  It is
tested independently from retail scene execution; separately provisioned
read-only retail input is observed only through a fixed redacted stage result.

## Scope

Include the original map-INI parser/provider ownership required by the reached
first parse call, map cache/game-text/display preconditions, optional map text
and smart asset purge ordering where actually invoked, missing/malformed parse
rejection, reset/retry and two-generation removal.  Generated fixture data
contains no copied retail material and does not establish retail visual or
frame fidelity.

Exclude consumer scene continuation, terrain loading, renderer frame capture,
private paths/names/hashes/bytes, screenshots, labels and raw parser output.
08B's early route stop remains unchanged until 08F accepts the completed
source-owned construction route.

## Validation

Positive: generated original map-override input parses under the original
method and proves ordered optional text/display handoffs; a missing optional
input remains accepted.  Negative: malformed required map override, absent
required provider, stale map cache/game text/display publication and two
fresh-generation retry fail closed or return to zero ownership.  The retail
audit reports only a fixed numbered reached stage and input-metadata invariant.
Run focused GCC/Clang/sanitizer and source identity/provider/ledger checks,
then the required full configured matrix, strict host leaks, Vulkan and serial
LAN before committing.

## Commit boundary

One independent commit: `delivery: M22 08F1 close map override parser`.
