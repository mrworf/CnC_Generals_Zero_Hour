# M22 06C3C2B3A0 allocator evidence

Clang ASan+UBSan first reproduced an aligned-delete heap-buffer-overflow during Vulkan's libLLVM initialization, before any original asset load: `GameMemory.cpp`'s process-wide aligned delete read its private header 16 bytes before a pointer allocated by libLLVM's aligned nothrow new. The same source file still owns ordinary original global new/delete; only the Linux aligned overloads were removed so libstdc++ owns matching aligned pairs.

Validation after the correction:

- GCC Debug: `original_process_allocator`, `original_process_identity_allocator`, and `original_process_provider_removal` pass.
- Clang ASan+UBSan: `original_process_allocator` and `original_process_identity_allocator` pass with `ASAN_OPTIONS=detect_leaks=0`; default LSan cannot run under this environment's ptrace confinement.
- Linked allocator binary lists runtime weak aligned new/delete symbols, not project strong aligned overloads.
- Host-GPU, Khronos-validation, Clang ASan+UBSan `asset_only` control passes two BGRA8 device generations with the opt-in W3D pool-size check enabled. The sandbox-only Vulkan-init failure was caused by hidden GPU device nodes.
- The prepared diagnostic build that bypasses W3D per-class pool overrides also passes the same two-generation `asset_only` host-GPU control. This comparison does not implicate or clear the W3D pools for full source rendering.

This does **not** accept B3A. In repeated host-GPU Clang ASan+UBSan two-generation full source-scene no-readback stress, runs 1 and 2 passed, but run 3 failed in second-generation `WW3D::Shutdown`: `MaterialInfoClass::Free` read a misaligned/corrupt `TextureClass` pointer (`0x80803702ff803702`). The remaining source-scene/asset ownership failure is tracked by B3A's blocker evidence and must be resolved before physical acceptance.
