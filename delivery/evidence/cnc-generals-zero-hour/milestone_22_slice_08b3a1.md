# M22 slice 08B3A1: source buffer-slot CPU closure

08B3A1 admits only the portable, source-compatible `W3DBufferManager` slot
allocator needed by the later volume-geometry slice.  The generated Recording
probe publishes the provider for two isolated generations, allocates supported
position/index slots, writes a bounded triangle, and binds it through the
public renderer edge.  It proves create and upload failure rollback followed
by retry, rejects invalid/oversize formats and sizes plus foreign and duplicate
releases, releases/reacquires resources, removes the provider, and ends both
generations with zero owned edge resources.

This slice deliberately does not extract volume geometry, admit a volumetric,
projected, or decal owner, invoke raw Direct3D or private Vulkan, make a pixel
claim, or inspect retail content.  Those boundaries remain owned by 08B3A2,
08B3B, and 08B3C.

Final-tree acceptance:

- Focused `original_w3d_volumetric_buffer` passed in GCC Debug and Clang
  Debug.  Fresh non-GPU/non-LAN suites passed **207/207** in GCC
  Debug/Release, Clang Debug/Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  Broad sanitizer CTest used `LSAN_OPTIONS=detect_leaks=0` only for the
  established ptrace limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` focused buffer, tracks, water, identity,
  provider-removal, and dependency-ledger checks passed **6/6** in both
  sanitizer configurations.
- Validation-layer physical factory bootstrap, rigid, and generated-map
  checks passed **3/3**.  They validate the existing factory route only; no
  physical volume or pixel result is claimed.  Serial LAN validation passed
  **4/4** in every one of the six configurations.
- The final original dependency-ledger validator and whitespace diff check
  passed.  The pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic
  remains unstaged and outside this slice.
