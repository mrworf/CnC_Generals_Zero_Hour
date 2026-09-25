# M22 slice 08H5D: extra-blend terrain submission

The CPU `HeightMapRenderObjClass` now consumes its accepted row-major
extra-blend inventory after the two base passes. Visible cells produce exact
transient XYZNDUV2 vertices and 16-bit source topology from authored extra
UV/alpha and height data, including the source cliff-height diagonal override.
The alpha atlas is submitted once through the accepted road-base material.

Generated coverage proves zero, one and eight-entry routes; both ordinary
diagonals; a cliff-authored false-to-true override; exact index ranges;
base-pass then extra-pass order; hidden/disabled/unsupported handling; injected
extra draw failure without visible publication; retry, teardown and two
generations. Shader and dynamic wrapper bindings unwind on every exit. No
retail input was used.

Acceptance on the final tree:

- Focused terrain/material gates pass with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes the focused test with GCC and Clang.
- Physical public Vulkan controls, serial host LAN in all six configurations,
  all three dependency ledgers and `git diff --check` pass.

This completes the implementation children required for aggregate 08H5.
