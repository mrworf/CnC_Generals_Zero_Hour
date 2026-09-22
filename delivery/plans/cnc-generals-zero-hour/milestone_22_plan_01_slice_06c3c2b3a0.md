# M22 slice 06C3C2B3A0: safe cross-DSO aligned allocation

## Outcome and boundary

Before B3A physical source-scene acceptance, preserve the original allocator for ordinary process allocations while allowing over-aligned C++ allocations from Vulkan/LLVM DSOs to pair with the C++ runtime's aligned delete. This is an independently testable process-allocator correction; it does not accept B3A, change W3D per-class pools, or alter original rendering behavior.

## Behavior and ownership

The process still routes ordinary `new`/`delete` through `GameMemory.cpp`. It no longer defines Linux process-wide aligned new/delete overloads with a private preceding header. The C++ runtime owns both sides of aligned allocation, including nothrow allocations from other DSOs. The caller must use a matching aligned delete; allocation failure follows the runtime's usual throwing/nothrow contract. No UI authorization, persistent data transition, or user input applies.

## Validation and commit boundary

Positive: cross-target 64-byte aligned allocation and release preserve alignment and original raw-allocation accounting. Negative boundary: a nothrow aligned allocation of the form used by third-party DSOs can be released without interpreting the pointer as an original private header or changing original raw accounting. Run GCC Debug and Clang ASan+UBSan allocator and identity tests, provider-removal, and inspect linked aligned symbols. Run a bounded Vulkan asset-only control with validation to verify DSO initialization, but do not infer source-scene acceptance from it. Preserve the existing B3A worktree and its separate failure evidence. Commit only `GameMemory.cpp` aligned-overload removal, allocator tests, this plan, and its evidence.

Acceptance: both toolchains pass the focused allocator tests; the process exports no project-owned aligned overload; asset-only Vulkan control reaches two device generations. B3A's repeated physical source scene remains an explicit later gate.

Commit: `delivery: M22 slice 06C3C2B3A0 use runtime aligned allocation`.
