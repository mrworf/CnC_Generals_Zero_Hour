# M22 plan 01 slice 08F0A: update-owned two-phase new-game lifecycle

## Goal

Restore the original ownership boundary between a new-game request and its
second, update-owned construction phase.  A generated scenario must request a
new game, allow the original `GameLogic::update` owner to dispatch the second
phase, and return through the existing reset/teardown lifecycle with no live
owned resources.

## Scope

Replace the Linux probe's direct second `startNewGame` call with the original
update-owned transition.  Add a generated-only witness for accepted dispatch,
missing owner rejection, reset/retry, two fresh generations, and zero
ownership.  Keep retail inputs read-only and do not run their parser/provider
route in this slice.

## Non-scope

Do not admit the retail selector, load a map parser/provider, construct a
scenario, capture Recording output, or change default production behavior.
Those belong respectively to 08F0, 08F1, 08F, and 08.

## Validation

Require focused GCC/Clang and canonical sanitizer witnesses, source
identity/provider/ledger checks, full six-configuration non-GPU/non-LAN
matrix, strict host leaks, proportional Vulkan, and serial LAN before the
independent commit.

## Commit boundary

One independent commit: `delivery: M22 08F0A restore update-owned new game`.
