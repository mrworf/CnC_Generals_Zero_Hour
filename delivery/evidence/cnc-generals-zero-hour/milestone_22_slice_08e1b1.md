# M22 08E1B1: generated mapped-image provider lifecycle

This slice binds the real original `INI::parseMappedImageDefinition` callback
to a project-owned generated descriptor and `ImageCollection`.  It does not
decode texture bytes, create GPU resources, load a layout, inspect retail
content, or make a pixel claim.

The dedicated probe runs two generations through the original INI callback
and proves named case-insensitive lookup, descriptor identity, parsed
dimensions, duplicate reparse with retained owner identity, missing input,
malformed-field failure with collection-scoped cleanup, descriptor-only
unknown-format/zero-extent state without raw payload, absent-provider
rejection, provider removal, and zero ownership.  The original absent global
returns before consuming a supplied block; the following fields therefore
reject, which is retained as the fail-closed boundary.

`INIMappedImage.cpp` is recorded as an M22 runtime ledger edge.  A live
provider-removal relink removes `INIMappedImage.cpp.o` and fails on
`INI::parseMappedImageDefinition(INI*)`.

Final acceptance:

- Fresh non-GPU/non-LAN broad suites passed **215/215** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  The two sandbox sanitizer broad suites used `ASAN_OPTIONS=detect_leaks=0`
  only because LeakSanitizer cannot start under sandbox ptrace restrictions.
- Host leak-enabled focused provider plus provider-removal checks passed
  **2/2** in both GCC and Clang sanitizer configurations.
- The proportional public bgfx Vulkan validation-layer stencil-edge probe
  passed **1/1** in GCC Release with no test-reported validation diagnostics.
- Host serial loopback LAN passed **4/4** in every one of the six
  configurations.  The sandbox-only UDP denial occurred before LAN test code
  and was not accepted as a product result.
- Dependency ledger validation and `git diff --check` pass.  The unrelated
  `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged.
