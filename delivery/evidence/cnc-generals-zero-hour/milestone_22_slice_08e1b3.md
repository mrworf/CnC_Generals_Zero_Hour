# M22 08E1B3: generated video lifecycle aggregate evidence

The aggregate leaves the accepted source boundaries intact.  Its coupled gate
runs B3A's original `VideoPlayer`/`VideoStream` registry witness, B3B's
original-base `VideoBuffer` free witness, and B3C's live generated
stream-to-buffer transaction in dependency order.  The B3C transaction
exercises `open → ready/decompress → render → next → close`, rejects its
invalid/failure transitions, then resets and retries.  Every witness requires
two generations and a zero-ownership terminal marker.  No new production
owner, retail input, Bink decoder, W3D texture upload, raw Direct3D, or pixel
claim is admitted.

- Focused coupled source-identity/removal checks: **7/7** in GCC Debug and
  **7/7** in Clang Debug.
- Fresh non-GPU/non-LAN final-tree suites: **224/224** in GCC Debug, GCC
  Release, Clang Debug, Clang Release, GCC ASan, and Clang ASan.  Sandbox ASan
  broad runs use `detect_leaks=0` only because the host leak gate is separate.
- Host leak focus with `detect_leaks=1`: **7/7** in GCC ASan and **7/7** in
  Clang ASan.  Physical Vulkan validation: **1/1**.  Serial LAN: **4/4** in
  each of the six configurations.
- Direct and registered dependency-ledger checks and `git diff --check` pass.
  The unrelated `tests/renderer/test_bgfx_device.cpp` diagnostic remains
  unstaged.
