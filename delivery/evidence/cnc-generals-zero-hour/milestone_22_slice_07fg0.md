# M22 slice 07FG0: public bgfx multi-mip texture prerequisite

`BgfxGpuDevice` now accepts bounded, sampled 2D color textures with a declared
mip prefix no longer than the source extent permits.  It creates the native
bgfx mip chain, admits only exact per-level extent/pitch/payload uploads, and
retains the existing single-level render-target boundary.  This is direct
public-bgfx transport: no production GameClient factory, map profile,
Recording-backed shortcut, retail asset, resize, draw, pixel, or visual claim
is included.

The independent physical contract covers RGBA8 and source A1R5G5B5 in two
device generations.  Each creates an 8x4 four-level sampled texture, uploads
8x4/4x2/2x1/1x1, rejects an out-of-range level, wrong extent, short pitch,
oversized payload, over-mipped descriptor and mipped render target, then
retires/recreates the opaque handle and verifies zero public resources after
each generation.

Final-tree acceptance:

- All six rebuilt non-GPU/non-LAN suites passed fresh **206/206**: GCC
  Debug/Release, Clang Debug/Release, GCC ASan+UBSan and Clang ASan+UBSan.
  The two broad sanitizer CTest runs used `LSAN_OPTIONS=detect_leaks=0` only
  for the established ptrace limitation.
- Host leak-enabled (`ASAN_OPTIONS=detect_leaks=1`) source ownership,
  terrain-atlas, particle/provider, shadow, presentation-identity and ledger
  focus passed **6/6** in both GCC and Clang sanitizer configurations.
  A stricter exploratory GCC rerun with `UBSAN_OPTIONS=halt_on_error=1`
  separately reproduces the existing `GlobalData.cpp:1028` null `FileSystem`
  member-call diagnostic in `original_w3d_presentation_identity`; it is outside
  this adapter-only slice and did not occur in either fresh broad suite.
- Direct host Vulkan validation passed
  `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation python3
  tools/run_validation_clean.py build/linux-gcc-debug/renderer_bgfx_mip_texture_tests`
  with no `Validation Error` or `VUID-` output.  GCC and Clang sanitizer
  physical runs pass with address/undefined checks and `detect_leaks=0`.
  A `detect_leaks=1` physical GPU exploration reports 3,636 bytes in six
  allocations wholly in host `libdbus-1` during Vulkan initialization, matching
  the repository's documented external DBus/Wayland caveat; it is neither
  suppressed nor claimed as slice-owned leak-free evidence.  The test's two
  zero-public-resource lifecycle assertions remain mandatory and pass.
- The serial local UDP/LAN exception passed **4/4** in every one of the six
  configurations.  Final original dependency-ledger, identity/provider focus
  and whitespace-diff checks passed.

The pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
unstaged and was not changed by this slice.
