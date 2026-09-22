# M22 plan 01 slice 06C3C2B3C: physical fault matrix and C3 aggregate

## Outcome and boundary

Requires B3B. Complete source scene/static-sort aggregation by proving original clear, category, static, sorting and stale-target failures abort without partial success or dangling queue/refcount/device resources. Fresh frame and device-generation retry must produce accepted source pixels. Then run the full M22-relevant suite. Only this slice accepts B3/B/C2/C3C/C3; C4 WWShade retail audit and M22 later slices remain pending.

## Verification and commit

Run positive source scene/static/category recording and public-bgfx pixels at two extents/generations, negative fault injection with exact failure sites and bounded refs, explicit Khronos validation-output scan, GCC/Clang Debug focused and full original-rendering/renderer/ABI/provider tests, both ASan/UBSan/LSan configurations, dependency ledger and full CTest matrix. Distinguish test-environment Vulkan ICD/ptrace failures from product failures with a minimal repro. Evidence `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b3c.md`. One commit: `delivery: M22 slice 06C3C2B3C accept original scene aggregate`.
