# M22 plan 01 slice 05B2B2B2: original shader state and combiners

## Outcome and dependencies

Requires accepted B1. The original `ShaderClass::Apply` source owns
blend/alpha-test/fog, primary and secondary texture-color/alpha combiner,
depth-write/test, cull and later authored state transitions. Preserve its
dirty-state, original delayed DX8Wrapper ordering and WWShade macro/path
selection, translating only source outputs to typed pending edge state.
No generic shader, physical pipeline or completed pass is claimed.

## Source capability closure

Preserve original field/default bit meanings and conditionals (including
capability-based fallback) in canonical source; the Linux device profile
must expose only public facts and explicit bounded policy. A missing
texture-op/device capability fails closed if no source-equivalent fallback
exists, recording the exact family. No invented old GPU vendor/caps, no
private Vulkan calls or silent fixed-function no-ops. Read-only retail
aggregate classifies required original shader/combiner families. Every
required family must proceed to B3/06 or trigger contract escalation;
test-only generated shaders cannot satisfy retail coverage.

## Acceptance and negatives

Owned original W3D shader/material fixtures and original-class state
fixtures witness source command order, dirty/unchanged short circuit,
distinct blend/alpha/fog/combiners/depth/cull, null stages, supported
fallback choices, WWShade enabled/disabled configuration and exact
negative unsupported required modes. Inject translation errors and prove
clean reset/retry without source/device leaks and without starting a pass.
Check provider-removal, canonical link-map ABI, ledger, GCC/Clang full
suites, sanitizers, classification and `git diff --check`. B3 and 06–09
remain mandatory for physical shader execution and scenes.

## Commit boundary

One independently validated commit:
`delivery: M22 slice 05B2B2B2 preserve original shader decisions`.
