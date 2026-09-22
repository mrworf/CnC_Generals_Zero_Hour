# M22 plan 01 slice 06C3C2B3C: physical fault matrix and C3 aggregate

## Outcome and boundary

Requires B3B. Complete source scene/static-sort aggregation by proving original clear, category, static, sorting and stale-target failures abort without partial success or dangling queue/refcount/device resources. Fresh frame and device-generation retry must produce accepted source pixels. Then run the full M22-relevant suite. Only this slice accepts B3/B/C2/C3C/C3; C4 WWShade retail audit and M22 later slices remain pending.

No retail files, GameClient integration, authored WWShade change, renderer abstraction change, or runtime default is in scope. Reuse B3B's owned W3D packet, `WW3DAssetManager` objects, `SimpleSceneClass`, original category/static/sort queues, camera and source frame entry. The existing `RecordingGpuDevice` controls are the bounded negative dependency; do not add test-only behavior to product source merely to trigger a failure. The attached public bgfx targets remain caller-owned.

## Fault/retry transaction

Add a separate focused source-scene fault-matrix mode. Start with a known-good original scene frame, then inject one failure per fresh source frame: malformed original clear input before a pass, physical draw rejection after the rigid draw when an original HLOD skin category is attached, rejection during static-list drain with both levels attached, and rejection during translucent sorted submission. Each failed call must throw at the expected source/physical site, leave no active source/public pass or success completion, release queued refs and selected DX8 buffers, and allow a fresh frame on the same objects/device. Destroy a bound depth target to reject a stale source Begin, then recreate/rebind and prove an accepted retry. A later fresh public-bgfx generation must still yield B3B's exact per-region pixel outcome. If the existing fault API cannot isolate a stage, measure the successful Recording draw order first and inject at that bounded count; do not weaken the category assertion.

Use existing B3A failed-draw task-retirement and source-frame clear/stale controls as independent controls, but the new focused mode must exercise all failure classes on the expanded scene. Keep source refs at their owning baseline and public resources at two caller attachments during WW3D teardown, then zero. No persistent state changes; after each negative, reset only the injected one-shot fault, original frame state and caller targets explicitly.

## Verification and commit

Run positive source scene/static/category recording and public-bgfx pixels at two extents/generations, negative fault injection with exact failure sites and bounded refs, explicit Khronos validation-output scan, GCC/Clang Debug focused and full original-rendering/renderer/ABI/provider tests, both ASan/UBSan/LSan configurations, dependency ledger and full CTest matrix. Distinguish test-environment Vulkan ICD/ptrace failures from product failures with a minimal repro. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3c.md`. One commit: `delivery: M22 slice 06C3C2B3C accept original scene aggregate`.
