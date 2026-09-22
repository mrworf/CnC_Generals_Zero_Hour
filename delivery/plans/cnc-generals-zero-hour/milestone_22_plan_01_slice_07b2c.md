# M22 plan 01 slice 07B2C: status-scene Vulkan validation output gate

## Outcome and boundary

Requires accepted 07B2. The status-scene Python runner captures the original child process output but currently checks only exit code and pixel marker on success. A non-fatal Khronos validation `Validation Error` or `VUID-` line could therefore be hidden. Reject those diagnostics from combined child stdout/stderr, preserving the existing physical pixel test and production code unchanged. This is a test-observability correction, not additional rendering support.

## Positive, negative and gate

Direct controls must accept ordinary successful output and reject each validation diagnostic token even with a successful process marker. The public-bgfx CTest still requires `VK_LAYER_KHRONOS_validation`; 30 fresh-process GCC and Clang runs must pass under the new output gate. Run four-preset source-scene/full asset-free suites, identity/ledger, and leak-capable source-only sanitizer controls. Commit plan, test utility/unit, CTest wiring and evidence independently. Do not promote full 07 or retail acceptance.
