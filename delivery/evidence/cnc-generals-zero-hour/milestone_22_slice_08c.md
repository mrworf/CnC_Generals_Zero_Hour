# M22 slice 08C: bounded selector-owned water scalar transaction

08C closes the exact next original `W3DTerrainVisual` water setup transaction
after the accepted 08B owner marker: height clamps, scalar transform, grid
resolution, and attenuation.  The four calls are admitted only when the
explicit host retail selector is set and the receiver is the exact published
original water owner.  The route does not infer an enabled water or cloud
configuration; default, foreign, non-finite, and invalid scalar inputs remain
typed failures.  Immediately after the complete transaction the redacted
route reaches the next pending boundary and rolls all owners back.  No retail
frame, pixels, raw graphics API, private path/name/hash, or retail byte is
recorded.

Focused proof:

- Generated Recording `original_w3d_terrain_water` passed in GCC and Clang
  Debug.  It proves two scalar-owner generations, exact source call order and
  resulting clamp/transform/resolution/attenuation state, selector absence,
  foreign owner and invalid scalar rejection, reset/load/update re-entry, and
  zero Recording resources.
- The read-only redacted campaign/skirmish Recording reachability gate passed
  1/1 across two modes and two generations.  It observes only aggregate
  markers for source-owner construction and scalar-transaction completion,
  followed by status-3 rollback with zero owners and resources.

Final-tree acceptance:

- Fresh non-GPU/non-LAN CTest suites passed **213/213** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan+UBSan, and Clang ASan+UBSan.
  Sanitizer broad suites used `ASAN_OPTIONS=detect_leaks=0` only for the
  sandbox ptrace limitation.
- Host `ASAN_OPTIONS=detect_leaks=1` focused scalar-owner, presentation
  identity, and dependency-ledger checks passed **3/3** in both sanitizer
  configurations.  The sandbox-only leak-enabled attempt aborts before test
  execution because LeakSanitizer cannot run under ptrace; the host result is
  the acceptance result.
- The existing paired public Vulkan stencil-edge validation-layer probe passed
  **1/1** with no test-reported validation diagnostics.  Serial loopback LAN
  validation passed **4/4** in all six configurations.
- Final dependency-ledger and whitespace-diff checks pass.  The unrelated
  `tests/renderer/test_bgfx_device.cpp` diagnostic remains unstaged.
