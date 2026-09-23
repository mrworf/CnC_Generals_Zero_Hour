# M22 slice 08B3A2: source volume-geometry CPU closure

08B3A2 extracts only the CPU side-wall construction from the original
`W3DVolumetricShadow::constructVolumeVB` route. Generated directed silhouette
pairs retain the source connected-strip reordering, closed-strip side-wall
winding, and 16-bit/16,384-vertex limits while allocating and releasing the
accepted 08B3A1 source slots. The Recording probe verifies the exact
six-vertex/eighteen-index closed triangle outline, rejects missing/foreign
provider, malformed, degenerate, zero-extrusion, and overflow inputs, rolls
back create/upload failures before retry, recreates resources, removes the
provider, and ends two generations with zero resources.

No `W3DVolumetricShadow` owner, update/render task, shadow type admission,
stencil/composite state, raw Direct3D/private Vulkan API, retail reachability,
or pixel claim is present. Those remain explicitly owned by 08B3B and 08B3C.

Final-tree acceptance:

- Focused GCC Debug and Clang Debug geometry probes passed. Fresh
  non-GPU/non-LAN suites passed **208/208** in GCC Debug/Release, Clang
  Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan. Broad sanitizer CTest
  used `LSAN_OPTIONS=detect_leaks=0` only for the established ptrace limit.
- Host `ASAN_OPTIONS=detect_leaks=1` focused buffer/geometry, tracks, water,
  identity, provider-removal, and dependency-ledger checks passed **7/7** in
  both sanitizer configurations.
- Validation-layer physical factory bootstrap, rigid, and generated-map
  checks passed **3/3**; they do not claim a physical volume or pixel result.
  Serial LAN validation passed **4/4** in all six configurations.
- Final dependency-ledger, identity/provider, and whitespace checks passed.
  The pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic remains
  unstaged and outside this slice.
