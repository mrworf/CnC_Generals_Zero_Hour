# N2 state-particle work-in-progress checkpoint

User-requested worktree consolidation, parent
632c54d415bf6868b936bd0893a439bed3438236. This commit preserves unfinished
work; it is **not** an accepted behavior slice, milestone, or delivery handoff.
Resolve its commit with `git log -1 --format=%H -- evidence/qa/N2-state-particle-wip.md`.

## Inventory and ownership

The initial worktree contained 11 modified files and one untracked file, all
belonging to the existing N2 plan01/slice03 state-particle batch:

- Eight original game headers/sources: NativeModelDrawAnimationProducer,
  NativeModelDrawConditionBatch, NativeParticleSystemAdmission and ParticleSys.
- `cmake/OriginalRuntime.cmake` and `tests/original/drawable_queries.cpp`.
- New `tests/original/GeneratedParticleInput.h`, which authors synthetic particle
  INI test input. It is required test source, not a generated build artifact or
  copied retail asset.
- `delivery/plans/cnc-generals-zero-hour/milestone_02_plan_01_slice_03.md`,
  containing the pre-existing governing batch plan.

These form one coupled checkpoint and must not be mistaken for independent
completed slices. This review adds only this evidence, a plan checkpoint note,
and current delivery resume state. No vendor source, retail asset, symlink,
recovery archive, or unrelated file was changed or removed. Ignored build/cache
content remains ignored and untouched.

## Attempted validation, not acceptance

The previous GCC coupled build terminated with a link failure in
`original_drawable_queries_fixture`: the new initial/animated particle callbacks
reference unresolved `Matrix3D::Get_Z_Rotation() const`. The declaration and
source definition exist in WWMath; deciding how the native game target should
provide that behavior remains implementation work. No library patch is authorized
or implied. Source reference: WWMath/matrix3d.cpp::Get_Z_Rotation uses
WWMath::Atan2(Row[1][0], Row[0][0]).

Log: `/tmp/n2-model-particle-coupled-build.log`. Because the build and CTest
commands were connected with `&&`, the focused CTest command did not execute;
`/tmp/n2-model-particle-coupled-focused.log` was not created. There are no live
owned build/test jobs at consolidation. Diff whitespace checks pass. No new
test pass, sanitizer pass, frozen matrix, or visual result is claimed here.
Earlier related144 and particle-growth results remain evidence exclusively for
their recorded committed sources, not for this checkpoint.

## Resume within the same slice

Review the native math linkage and the coupled initial/animated/visibility paths,
then complete actual state-particle semantic and allocation failure/retry tests.
The two newly registered semantic families have not run. Add suppressed-state,
idle restart, scaling, late locally caught mutation failure, pooled acquisition,
and repeated valid-parent cleanup coverage as required by the existing plan.
Run focused GCC and GCC/Clang sanitizer validation before the frozen related
matrix. Preserve every failure/retry pair; do not weaken coverage or increase
timeouts. Full concrete19 tick/lazy events/physics/AI/geometry, camera, startup and
loaded simulation remain pending. N2 and slice03 remain in progress.
