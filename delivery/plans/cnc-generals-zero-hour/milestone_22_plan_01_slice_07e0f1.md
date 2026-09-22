# M22 plan 01 slice 07E0F1: retire drained polygon render-task pool

## Outcome and dependency

Requires accepted 07E0D3 guarded AutoPool retirement and accepted original rigid source drawing. The first production-factory rigid frame creates a 6,152-byte `PolyRenderTaskClass` slab in `DX8TextureCategoryClass::Add_Render_Task`; source rendering deletes every task, but CPU `DX8MeshRendererClass::Shutdown` retains the empty process-static slab. This prerequisite retires only that drained source cache so the factory rigid slice can meet the same post-device residual as its empty control.

## Scope and lifecycle

At CPU WW3D shutdown, after `Invalidate(true)` and pending-delete retirement have returned every render task, invoke the existing guarded `AutoPoolClass::Release_Empty_Blocks` for `PolyRenderTaskClass`. The guard preserves an active pool unchanged, so failed/incomplete drawing cannot free live tasks. Native Windows behavior, render ordering, mesh ownership, device/driver allocations, global allocation policy, terrain/effects, and retail content are out of scope. Authorization is not applicable to this internal teardown lifecycle.

## Entry point, state, and failure

Entry is the canonical `WW3D::Shutdown` call to `DX8MeshRendererClass::Shutdown`. A completed source draw transitions the task pool from an allocated/drained cache to zero blocks; a second generation reallocates and drains normally. If any task remains active, retirement returns false without changing the pool. Shutdown remains non-throwing and does not mask an active-task defect. The exact owner is established by an allocation watchpoint at the post-device boundary: the only rigid-only raw block is allocated by `PolyRenderTaskClass::operator new` from `DX8TextureCategoryClass::Add_Render_Task`.

## Validation and commit boundary

The existing direct 07E0D3 pool test supplies positive empty retirement, active-node refusal, idempotence and second-generation coverage for the unchanged guard. Add focused source lifecycle coverage that a real rigid render drains and retires this specific pool, while an empty frame creates no task slab. Run the original presentation/source tests, host Vulkan rigid versus empty allocation control, five non-GPU suites including GCC/Clang sanitizers, repeated GCC Debug and Clang Release physical generations, and ledger checks. Commit the shutdown call, plan, focused test/evidence and ledger independently before resuming 07E0F; do not weaken residual equality.

## Result

An allocation watchpoint at the measured post-device boundary identified the rigid-only 6,152-byte raw block as `PolyRenderTaskClass::operator new` called by `DX8TextureCategoryClass::Add_Render_Task`; the 16,384-byte `_PlaneEQArray` block was common to both controls. CPU mesh-renderer shutdown now asserts every task has returned before guarded slab retirement. The direct pool active-node/refusal/idempotence/re-entry test and all focused source/fault tests pass. The paired pending-factory diagnostic moved from residual 27 to 26, exactly matching the empty/device-only control, without changing the assertion. Five exact-tree non-GPU suites pass 194/194, and GCC Debug plus Clang Release source Vulkan repetitions pass 30/30 each. Evidence: [07E0F1](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07e0f1.md).
