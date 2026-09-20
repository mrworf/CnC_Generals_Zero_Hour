# W3D startup architecture reconciliation

## Scope

Targeted source review at `853842b654261c08dafc8b5131e3b64d69cbc653` of the M20 plan 02 retail blocker. No retail traversal, runtime execution, source edits or new acceptance claims. Governing architecture is `docs/zero-hour-runtime-closure-reconciliation.md`; RC-012 records the resolved engineering direction.

## Executive Summary

The missing provider is real, but it is not a new product decision or a reason to move rendering acceptance before startup. The source architecture permits separation of original schema methods from rendering code. Keep the full retail gate and make that implementation boundary explicit. Readiness means permission to implement it, not proof that the port is complete.

## What Is Good

- Original `W3DModuleFactory.cpp:62–80` identifies the complete 19-provider extension; this gives a concrete completeness oracle instead of learning one name per retail failure.
- RC-001/002/004 already allocate transitive startup providers to integration and permit narrow shared-method refactoring. Preserving those rules prevents a dependency cycle with later rendering acceptance.
- The historical blocker reports the failed retail gate and removed experimental edits rather than claiming acceptance from owned fixtures.

## What Is Bad Or Risky

- `src/original_runtime/linux_game_engine.cpp:624` constructs only the base ModuleFactory. `ThingTemplate.cpp:601–603` immediately dereferences its ModuleData result. A missing registry entry therefore causes a crash rather than a controlled startup error.
- Registry macros connect both data and instance construction. `W3DDefaultDraw.h` uses the standard macro, while `W3DModelDraw.h:281–327` defines substantial typed data, field parsing and lifetime behavior. Registering names with generic data or simply compiling a registry header cannot preserve those contracts.
- `W3DDefaultDraw.cpp:42–49` includes WW3D rendering, scene and display headers. The recorded compile attempt reaches `dx8fvf.h` and `d3d8.h`. This is a compile-boundary problem even without hardware acquisition; Linux Vulkan availability does not supply legacy Direct3D headers.
- The retained BIG adapter has only bounded partial validation according to the blocker report. Earlier provider acceptance and fixture startup do not establish its malformed-input safety or retail closure.

## What Should Change

Apply RC-012: preserve the original full registry and concrete data methods in a shared CPU schema closure; make unavailable instance creation explicitly fail closed only where unreachable by the required startup profile. Port any actually reached original instance behavior before startup acceptance. Add source-identity, complete registration/schema, unknown-provider and cleanup tests, then all original integration gates. Track deferred draw operations against their real consumers rather than filenames or milestone labels. The compiler owns grouping and dependency allocation.

## What I Would Not Change Yet

Do not reopen accepted M26–M28, install an excluded D3D SDK, redesign the renderer, weaken the retail gate, or make M22/M23 prerequisites of M20. None follows from the observed schema failure. Do not claim that this source review exhaustively discovers future porting issues; the evolving provider ledger and integration tests remain necessary.

## Overall Opinion

Implementation-ready within the existing product/backend direction. The blocker exposed an incomplete provider and insufficiently concrete boundary, not a demonstrated backend incompatibility. Proceed to dependency-aware compilation and readiness; preserve implementation history and require new governing-plan adoption before resuming delivery.
