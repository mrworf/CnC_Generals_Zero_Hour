# M22 08E1B2: generated Mouse provider lifecycle evidence

This slice closes the bounded original `Mouse` singleton edge used by
`SinglePlayerLoadScreen::update`: clearing an empty cursor tooltip and
restoring the source default delay. It deliberately does not present a
nonempty visual tooltip or assert full SinglePlayer layout ordering: both
need the later B4 DisplayString/layout owner closure.

The generated lifecycle probe exercises two generations of a project-owned
headless `Mouse`, confirms empty-tooltip/default-delay state, published
`TheMouse` identity, cursor capture/release, reset/re-entry, provider removal,
and zero remaining owner publication. A relinked removal check proves
`Mouse.cpp.o` supplies `Mouse::setCursorTooltip`.

Final acceptance:

- Fresh non-GPU/non-LAN broad suites passed **217/217** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  The sandbox sanitizer broad suites used `ASAN_OPTIONS=detect_leaks=0` only
  because LeakSanitizer cannot initialize under the sandbox ptrace policy.
- Host leak-enabled focused provider plus provider-removal checks passed
  **2/2** in both GCC and Clang sanitizer configurations.
- The proportional public bgfx Vulkan validation-layer stencil-edge probe
  passed **1/1** in GCC Release with no test-reported validation diagnostics.
- Host serial loopback LAN passed **4/4** in every one of the six
  configurations. The sandbox-only UDP denial occurs before LAN test code and
  is not accepted as a product result.
- Direct and registered dependency-ledger validation, plus `git diff --check`,
  pass. The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic remains
  unstaged.
