# M22 slice 08B3C0: original XYZ volume public-edge dispatch

08B3C0 closes the execution seam between the accepted source CPU slot/geometry
prerequisites and the accepted public stencil protocol. `OriginalGpuEdge`
accepts only published original `XYZ` vertex and 16-bit index ranges inside an
active caller-owned D24S8 source frame, then submits the existing public
increment, decrement, and composite descriptors in that order. The bridge owns
only its public shaders, pipelines, and transient source-buffer bindings.

It rejects inactive frames, absent/foreign providers, unsupported layouts,
out-of-range indices, unsupported D24S8/color targets, and injected
shader/pipeline/buffer-create/upload/draw failures. Generated Recording
evidence proves retry, two-generation recreation, provider removal, exact
three-pass order, and zero resources. A paired host Vulkan test submits the
same bridge under `VK_LAYER_KHRONOS_validation`; its log has no `Validation
Error` or `VUID-` output. This is an execution/state witness only: it admits no
`W3DVolumetricShadow` owner, source scheduling, retail route, resize or
presentation behavior, raw Direct3D/private Vulkan API, or pixel claim.

Final-tree acceptance:

- Full builds completed in GCC/Clang Debug, Release, and ASan+UBSan presets.
  Fresh non-GPU/non-LAN CTest suites passed **211/211** in each of the six
  configurations. Broad sanitizer suites used `ASAN_OPTIONS=detect_leaks=0`
  only because the sandbox ptrace layer prevents LeakSanitizer from running.
- Host `ASAN_OPTIONS=detect_leaks=1` focused volume slots, geometry, CPU
  closure, stencil contract, and C0 bridge tests passed **5/5** under both GCC
  and Clang sanitizers. The initial sandbox leak-enabled run failed solely with
  the documented LeakSanitizer-under-ptrace fatal error; the host reruns are
  the acceptance result.
- The paired `original_w3d_volumetric_stencil_edge_bgfx` host Vulkan test
  passed with validation layers enabled and a clean explicit output scan.
  Loopback LAN tests passed **4/4** in every one of the six configurations.
- Final source identity/provider and dependency-ledger tests passed. The
  unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged.
