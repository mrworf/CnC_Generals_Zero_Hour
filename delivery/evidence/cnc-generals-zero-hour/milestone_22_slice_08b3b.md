# M22 slice 08B3B: public stencil volume/composite state contract

08B3B translates the original volumetric-shadow GPU edge into three public,
backend-agnostic `PipelineDesc` values only: clockwise color-disabled stencil
increment, counter-clockwise color-disabled stencil decrement, and a
`DESTCOLOR/ZERO` composite constrained by the complementary stencil mask. It
does not admit a `W3DVolumetricShadow` owner, source update/draw ordering,
retail input, resize/presentation path, raw Direct3D/private Vulkan API, or a
pixel claim; 08B3C owns the source-side route.

The generated Recording witness verifies the exact state fields and the
increment → decrement → composite command order. It rejects missing shader
handles and depth-only composite targets, rejects unsupported D24S8, creates
and uploads only after retry from injected failures, rejects an out-of-pass
draw and an injected draw failure, recreates for two generations, and releases
every handle. The public bgfx/Vulkan probe submits the same three descriptors
to a D24S8 off-screen pass in two generations under validation layers.

Final-tree acceptance:

- Fresh non-GPU/non-LAN suites passed **210/210** in GCC Debug/Release, Clang
  Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan. Broad sanitizer CTest
  used `LSAN_OPTIONS=detect_leaks=0` only for the established ptrace limit.
- Host `ASAN_OPTIONS=detect_leaks=1` focused slots/geometry/CPU/stencil,
  tracks/water, identity, provider-removal, and dependency-ledger checks
  passed **9/9** in both sanitizer configurations.
- The new off-screen public bgfx Vulkan validation-layer three-pass probe
  passed; it exercises the real D24S8 state edge but deliberately makes no
  pixel assertion. Serial LAN validation passed **4/4** in every configuration.
- Final dependency-ledger and whitespace checks passed. The pre-existing
  `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged and is
  outside this slice.
