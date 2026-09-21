# M22 plan 01 slice 05: Vulkan presentation and milestone acceptance

## Goal and observable outcome

The identical slice 04 original retail scenes render and present through public SDL_GPU Vulkan on the verified RTX, with explicit Khronos validation, reviewed visual evidence, resize/resource recreation, and bounded teardown.

## Scope

- Bind the slice 03 translator to `SdlGpuDevice`; do not add a hardware-only producer path.
- Present both scene families, resize in both dimensions, recreate device resources, wait idle, destroy, and create a second device generation.
- Capture only permitted project-owned/derived visual evidence and record private-safe review results.
- Close four-preset, full-suite, sanitizer, identity, provider-removal, source-classification, ledger, installation/arbitrary-CWD, and diff checks.

## Validation and error handling

- Set `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`; combine stdout/stderr and fail on `Validation Error` or `VUID-` even at exit zero.
- Reject unavailable layer/device/session, missing shader/resource, stale resize generation, presentation failure, timeout, placeholder output, visual family omission, and nonzero teardown ownership.
- Compare retail metadata before/after every hardware run and isolate XDG output.

## Acceptance criteria

- Both real scene families visibly render through the same original producer chain on SDL_GPU Vulkan with zero validation errors/VUIDs.
- Resize/recreation and two-generation teardown complete within bounds with zero resources.
- Four canonical preset builds/full asset-free suites and focused GCC/Clang sanitizers pass; canonical schema, identity, provider-removal, and ledger gates pass.

## Commit boundary

Commit Vulkan binding/evidence, cumulative fixes, slice status, and milestone acceptance as `delivery: M22 slice 05 render original retail scenes on Vulkan`.
