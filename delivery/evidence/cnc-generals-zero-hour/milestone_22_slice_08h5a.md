# M22 slice 08H5A: authored static terrain vertices

The CPU `HeightMapRenderObjClass` no longer rejects accepted nonzero authored
terrain classes or primary blend metadata. Initialization materializes the
map-owned base/alpha atlas pair before vertex construction. Every populated
cell stores base-atlas UVs in channel one, primary blend UVs in channel two,
and authored blend alpha in diffuse alpha while preserving white RGB and the
accepted fixed base topology.

Generated coverage uses the existing flat control plus a multi-class authored
map containing primary/extra blend and cliff records. It checks exact queried
coordinates and alpha in the CPU backup, both accepted base passes, injected
buffer failure and clean retry. The scene-attachment two-generation fixture
now correctly opens and releases its generated map and map-owned atlases
inside each device-edge generation. No retail input was used.

Acceptance on the final tree:

- Focused terrain/construction gates pass with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass 261/261;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes the focused test with GCC and Clang.
- Physical public Vulkan validation-layer controls pass 2/2, serial host LAN
  passes 4/4 in all six configurations, all three dependency ledgers pass,
  and `git diff --check` is clean.

Extra-blend inventory, material and dynamic submission remain the ordered
08H5B–08H5D dependencies.
