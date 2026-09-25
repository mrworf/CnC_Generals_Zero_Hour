# M22 slice 08H5B: extra-blend tile inventory

The CPU `HeightMapRenderObjClass` now scans accepted authored terrain metadata
through `getExtraAlphaUVData` in source row-major order. It retains an
exact-capacity packed-coordinate inventory only after the full height-map
geometry transaction succeeds. Explicit free, destruction, failed later
buffer creation and two-generation re-entry all restore empty inventory
ownership.

Generated coverage uses a flat zero-entry control, a one-entry authored map
and an eight-entry map with a cliff-tagged final cell. It checks exact order
and capacity, later-stage failure without partial publication, clean retry and
teardown. No retail input was used.

Acceptance on the final tree:

- Focused terrain gates pass with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes the focused test with GCC and Clang.
- Physical public Vulkan controls, serial host LAN in all six configurations,
  all three dependency ledgers and `git diff --check` pass.

Road-base alpha material ownership and dynamic extra-blend submission remain
the ordered 08H5C–08H5D dependencies.
