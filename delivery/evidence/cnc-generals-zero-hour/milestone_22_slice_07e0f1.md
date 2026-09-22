# M22 slice 07E0F1 evidence: drained polygon render-task pool

## Ownership and lifecycle proof

- A GDB allocation/free watchpoint at the post-device residual boundary found the rigid-only raw block at 6,152 bytes: `ObjectPoolClass<PolyRenderTaskClass,256>::Allocate_Object_Memory` → `PolyRenderTaskClass::operator new` → `DX8TextureCategoryClass::Add_Render_Task` → `MeshClass::Render` → `RTS3DScene`.
- The other live raw block was the 16,384-byte process-static `meshgeometry.cpp::_PlaneEQArray`, present in both the empty and rigid controls.
- The source frame deletes each `PolyRenderTaskClass` after category rendering. `DX8MeshRendererClass::Shutdown` now performs retirement after `Invalidate(true)`, pending-delete drain and temporary-buffer clear, and asserts the guarded pool reports fully drained. The accepted generic pool negative leaves live nodes and all counts unchanged; empty retirement is idempotent and a second generation reallocates and retires normally.
- Before this slice, the rigid factory diagnostic ended at residual 27 versus empty/device residual 26. With only the shutdown retirement added, rigid ends at 26. No allocation baseline or assertion was weakened.

## Focused and physical validation

- `original_w3d_presentation`, `original_w3d_source_static_scene`, `original_w3d_source_mixed_scene`, `original_w3d_source_owned_pass_scene`, and `original_w3d_source_mixed_fault_matrix`: 5/5.
- Direct Khronos-validation source probes: static scene and injected mixed-fault/retry both pass.
- Repeated host Vulkan source-static scene, including the existing two extents, two color formats and source re-entry: GCC Debug 30/30; Clang Release 30/30.
- Paired factory diagnostic: empty changed pixels 0/residual 26; rigid changed pixels 74,850/residual 26.

## Broad gate

Freshly rebuilt exact-tree non-GPU suites all pass:

- Linux GCC Debug: 194/194.
- Linux GCC Release: 194/194.
- Linux Clang Release: 194/194.
- Linux GCC ASan/UBSan: 194/194.
- Linux Clang ASan/UBSan: 194/194.

The three dependency-ledger integrity tests pass after updating only the `dx8renderer.cpp` hash and shutdown ownership contract. Pending 07E0F files and the unrelated renderer diagnostic were isolated for the exact-tree gate. Retail symlink content was neither read nor modified.
