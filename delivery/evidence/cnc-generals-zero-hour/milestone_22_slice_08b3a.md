# M22 slice 08B3A: aggregate CPU source prerequisite

08B3A composes accepted A1 slots and A2 side-wall geometry in one generated,
two-generation Recording transaction. It allocates a bounded source triangle
alongside the exact source topology volume, proves shared-provider create and
upload rollback/retry, reverse slot/geometry teardown, resource recreation,
provider removal, and zero resources. No shadow owner, render task,
stencil/composite state, raw Direct3D/private Vulkan API, retail input, or
pixel claim is admitted.

Final-tree acceptance:

- Focused GCC and Clang probes passed. Fresh non-GPU/non-LAN suites passed
  **209/209** in GCC Debug/Release, Clang Debug/Release, GCC ASan+UBSan, and
  Clang ASan+UBSan. Broad sanitizers used `LSAN_OPTIONS=detect_leaks=0` only
  for the established ptrace limitation.
- Leak-enabled focused slots/geometry/aggregate, tracks, water, identity,
  provider-removal, and ledger checks passed **8/8** in both sanitizer builds.
- Physical Vulkan factory bootstrap/rigid/generated-map checks passed **3/3**;
  they make no volume or pixel claim. Serial LAN passed **4/4** in all six
  configurations.
- Final ledger, identity/provider, and whitespace checks passed. The existing
  `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged.
