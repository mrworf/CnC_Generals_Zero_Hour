# N2 worktree inventory and documentation checkpoint

Review parent: f44b668adea375546d391d75827e296b4bbd1701.
User request: sort changed/untracked files and commit as appropriate.
Existing milestone transaction: N2 plan01/slice03; no replacement plan.

The initial nonignored inventory contains 16 modified and three untracked files.
All belong to the unfinished shared condition/animation producer batch. No
unrelated nonignored files were found. No files were deleted, moved, reverted,
stashed or imported from recovery. Retail links/targets and upstream dependency
sources were not changed by this review.

## Committed documentation only

- `docs/original-engine-formats.md`: verified original `nukeCurrentRender`
  pause reset and root-transform preservation; same-model reuse does not invoke
  that recreation path. This is a source finding, not native acceptance.
- `delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01_slice_03.md`:
  existing coherent producer continuation and its pending validation contract.
- This inventory and `workflow/delivery/state.yaml`: exact unfinished work and
  resume point. Earlier accepted checkpoint provenance remains unchanged.

## Unfinished implementation: 14 modified, three untracked

Modified headers under `GeneralsMD/Code/GameEngine/Include/`:

- `Common/NativeW3DPose.h`
- `Common/W3DModelDrawData.h`
- `GameClient/ClientRandomValue.h`
- `GameClient/Display.h`
- `GameClient/NativeHeadlessDisplay.h`
- `GameClient/NativeModelAnimationClock.h`
- `GameClient/NativeModelDrawCurrentQueries.h`
- `GameClient/NativeModelDrawQueries.h`

Modified source/build/tests:

- `GeneralsMD/Code/GameEngine/Source/Common/RandomValue.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/System/NativeW3DPose.cpp`
- `GeneralsMD/Code/GameEngine/Source/Common/System/W3DModelDrawData.cpp`
- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Drawable/Draw/W3DModelDraw.cpp`
- `cmake/OriginalRuntime.cmake`
- `tests/original/drawable_queries.cpp`

Untracked implementation, retained in place:

- `GeneralsMD/Code/GameEngine/Include/Common/W3DModelAnimationFlags.h`
- `GeneralsMD/Code/GameEngine/Include/GameClient/NativeModelDrawAnimationProducer.h`
- `GeneralsMD/Code/GameEngine/Source/GameClient/NativeModelDrawAnimationProducer.cpp`

These are one coupled source-state/RNG/pose owner and its callers/fixtures, not
independently accepted helpers. They must be committed together only after the
existing plan's required validation passes; this documentation checkpoint does
not absorb the implementation or mark slice03/N2 complete.

## Evidence and exact resume point

The existing GCC coupled build completed successfully. The subsequent three
producer families finished in 5.61 seconds: functional and transitions passed;
selection failed with `weighted idle reaches retained completion`. Logs:
`/tmp/n2-condition-producer-coupled-build.log` and
`/tmp/n2-condition-producer-coupled.log`. No CMake/Ninja/CTest job was running
at inventory time. `git diff --check` passed.

Source review identifies a fixture precondition problem to resolve and verify,
not a reason to change production completion rules: RANDOMSTART can select the
final frame before `producerStep`; that method checks retained completion and
performs idle reselection before advancing. A multiplier applied to the prior
selection therefore does not guarantee that the next selection is complete.
Keep exact original weighted-index rejection sampling, RNG cursor, and start
precedence checks. Establish deterministic retained completion without bypassing
the behavior being asserted, then validate selection and its negative cases.

The producer also still needs the planned cold/seeded allocation-failure/retry
coverage, complete ownership-cohort warm-up, and frozen related GCC plus GCC and
Clang sanitizer validation. Previous raw-frame checkpoint sanitizer results do
not validate these newer dirty files. Specialized concrete19 effects/callbacks,
full native startup and loaded simulation remain pending. No milestone, retail,
visual-parity or full-suite acceptance is claimed.
