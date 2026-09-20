# M20 plan 01 slice 01: source dependency-boundary investigation

## Goal and outcome

Determine whether the exact M20 contract can execute preserved original `GameMain -> CreateGameEngine -> GameEngine::init/execute` behavior before M21–M23. The observable outcome is a source-backed executable boundary or a precise upstream dependency defect.

## Scope

Inspect original initialization order, abstract factories and concrete provider implementations; test whether an M20 stop point can retain original semantics. Record any dependency-cycle blocker. Production changes are permitted only if the preserved lifecycle can be built without substituting later providers.

## Non-scope

Do not create a reduced parallel `GameEngine`, compile original filenames around replacement semantics, or add success-only null module/object registries. Do not implement M21–M23 under M20.

## Dependencies and ordering

Requires accepted M19 support and existing M4 data APIs. This is the first M20 slice and supplies the source-owned lifecycle used by slices 02–03.

## Entry point and behavior

Investigation established that original `GameEngine::init` initializes `TheModuleFactory` at source line 542 and `TheThingFactory` at line 577 only after file/data, CSF, science, terrain, audio and concrete function-lexicon providers. The production Win32 factory supplies W3D implementations. The base `ModuleFactory.cpp` directly includes 202 GameLogic module headers and three GameClient module headers; `W3DModuleFactory.cpp` adds 20 W3D draw-module headers. Those are M21/M22 providers, while audio/media is M23.

## Data/state transitions

No runtime state transition was added because the only bounded implementation attempted would have replaced rather than preserved original transitions. The worktree was restored before commit.

## Authorization

Not applicable: this is a local process lifecycle. Host paths and retail content are not consumed in this slice.

## Validation and error handling

The decision test is architectural: if required original factories depend on later milestones and no source-semantic boundary exists, classify `UPSTREAM_REQUIRED`. That condition is met. After replanning, runtime positive/negative tests remain mandatory.

## Expected surfaces

This governing plan, the three slice plans and the M20 blocker evidence only. No production surfaces remain modified.

## Validation commands

- Inspect original `GameEngine.cpp`, `ModuleFactory.cpp`, Win32/W3D factory implementations and current source classification.
- Verify `git diff` contains no production implementation changes.

## Acceptance and commit boundary

Acceptance for this investigation is a reproducible dependency finding and exact upstream handoff, with rejected synthetic code removed. Status: blocked (`UPSTREAM_REQUIRED`). Evidence: `evidence/qa/cnc-generals-zero-hour/M20-original-lifecycle-blocker.md`.
