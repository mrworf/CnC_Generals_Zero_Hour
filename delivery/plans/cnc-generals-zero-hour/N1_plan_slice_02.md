# N1 slice 02 — capacity and lifecycle qualification

## Goal and scope

The stock renderer handles source-derived scene demand and repeated lifetimes
without omitted draws, accumulating ownership or unwanted simulation progress.
Depends on slice 01's accepted public APIs and physical semantic tests.
No proprietary input or original world integration is necessary for these fixtures.

## State, interfaces and ownership

Build a generated scene workload census from original terrain/material/camera
source consumers, then drive loading→ready→draw→replace/cancel→resize→shutdown.
Use game-owned resource tracking, public capabilities/stats and public completion
semantics. Persist source holders until release and readback destinations until
ready. Never equate a release callback with GPU completion or force private arena
shrink. No GPU transaction rollback contract is introduced.

## Surfaces and implementation choices

Extend the generated fixtures, source requirement/census manifest, runner and
evidence. Select resource batching and public capacity settings from the observed
complete workload; preserve independent maximum dimensions/cardinality/bytes.
Game-owned allocations may be bounded; backend diagnostics must remain truthful
estimates when upstream provides estimates. No engine/thread authority changes.

## Validation and acceptance

Test repeated generations/context teardown and mixed/all-active frames; compare
expected draw counts and physical outputs, not just handle counts. Exercise
invalid capacities, canceled preparations, stale/destroyed handles at our own
admission boundary, and corrected retries without issuing invalid upstream calls.
Require GCC/Clang sanitizer ownership checks, Vulkan validation, stable resource
use after completed lifetimes, and meaningful workload timing/memory evidence.
Run the full N1 suite on frozen source and record all unexecuted checks explicitly.

An upstream public capability gap stops qualification and triggers architecture
reassessment. A watchdog timeout remains incomplete execution. Commit this slice
and mark N1 accepted only when the complete governing gate passes.
