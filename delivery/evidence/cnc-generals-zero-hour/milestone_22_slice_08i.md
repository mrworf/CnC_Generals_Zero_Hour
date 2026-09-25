# M22 slice 08I: general multi-tile terrain aggregate

Accepted slices 08I1 and 08I2 compose through the canonical CPU
`HeightMapRenderObjClass`. Generated X-only, Y-only, exact two-axis and partial
two-axis maps own checked row-major buffer grids, preserve global authored
payload at tile seams, update intersected tile-local storage, and submit every
valid cell in pass-major/tile-row-major order without fixed-capacity padding.

The aggregate covers partial construction/upload rollback, incomplete owners,
invalid edge metadata, physical bind failure, first/last draw failure in both
base passes, deterministic retry, base-before-extra-before-tracks ordering,
two process generations and zero retained resources. No retail input or
selector was used.

Acceptance on the composed final tree:

- Focused and related authored-terrain gates pass with GCC and Clang.
- Focused ASan+UBSan passes with `detect_leaks=0`; strict host LeakSanitizer
  passes with GCC and Clang.
- All six canonical configurations build and their non-GPU/non-LAN/non-retail
  suites pass 261/261; sanitizer suites use `detect_leaks=0` in the sandbox.
- Physical public Vulkan controls pass 2/2; serial host LAN passes 4/4 in all
  six configurations.
- All three dependency-ledger checks and `git diff --check` pass.
