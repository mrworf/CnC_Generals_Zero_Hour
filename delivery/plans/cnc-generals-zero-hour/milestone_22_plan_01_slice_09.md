# M22 plan 01 slice 09: Vulkan visual and cumulative milestone acceptance

## Outcome and dependencies

Requires slice 08 and PRE-012/PRE-016. The same original retail campaign and
skirmish scenes render and present through SDL_GPU Vulkan on this RTX/Wayland
host with `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation` explicitly set.

## Acceptance

Scan combined output for zero `Validation Error` and `VUID-`, even on exit 0.
Capture/review permitted visual evidence for each family; reject placeholders,
missing required assets, wrong state or incomplete frames. Exercise both
dimensions of resize/device recreation, idle/teardown, injected failure and
zero owned resources after reset. Run four canonical presets and complete
asset-free suites, focused GCC/Clang ASan+UBSan, installed/arbitrary-CWD
fixtures with isolated XDG, source/provider-removal and fresh ledger checks.
Prove retail corpus metadata unchanged and no Direct3D/private Vulkan calls.

## Commit boundary

One independently validated commit: `delivery: M22 slice 09 validate Vulkan retail presentation`.
