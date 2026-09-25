# M22 slice 08H5C: extra-blend material edge

The CPU `W3DShaderManager` now exposes the source `ST_ROAD_BASE` minimum
profile as one alpha-atlas pass. Texture and vertex diffuse channels modulate
color and alpha, blending uses source alpha and inverse source alpha, depth
testing is less-equal and depth writes are disabled. The accepted two-pass
terrain-base route is unchanged; all noise, cloud/light-map and debug variants
remain unavailable.

Generated direct Recording coverage validates missing and unpublished texture
owners before state changes, invalid kind/pass/reset ordering, injected sampler
failure and retry, exact public combiner/blend/depth state, idempotent lifecycle
and reset/shutdown cleanup over two generations. The material route schedules
no draw, and no retail input was used.

Acceptance on the final tree:

- Focused shader/terrain gates pass with GCC and Clang.
- All six builds and canonical non-GPU/non-LAN/non-retail suites pass;
  sanitizer suites use `detect_leaks=0` in the sandbox.
- Strict host LeakSanitizer passes the focused test with GCC and Clang.
- Physical public Vulkan controls, serial host LAN in all six configurations,
  all three dependency ledgers and `git diff --check` pass.

Dynamic extra-blend geometry and ordered submission remain 08H5D.
