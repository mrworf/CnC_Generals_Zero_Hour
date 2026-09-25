# M22 slice 08I1: general multi-tile terrain geometry ownership

The CPU `HeightMapRenderObjClass` now derives a checked row-major tile grid
from general visual-map dimensions, owns one fixed-capacity vertex buffer and
zero-initialized backup per tile, and records exact final-column/final-row cell
counts. `updateBlock` intersects global source regions with each tile and
writes tile-local storage while preserving authored global position, atlas UV
and alpha values. A failed allocation or upload unwinds every constructed
resource and leaves the owner reusable.

Generated coverage crosses X alone, Y alone and both axes, including exact and
partial boundaries. It proves tile order, seam and edge payloads,
deterministic unused capacity, bounded edge updates, a third-upload failure,
exact rollback, retry, teardown and two device generations. The accepted
single-tile render route remains unchanged; general multi-tile submission is
still owned by slice 08I2. No retail input was used.

Acceptance on the final slice tree:

- Focused geometry and related terrain tests pass with GCC and Clang.
- Focused ASan+UBSan passes with `detect_leaks=0`; strict host LeakSanitizer
  passes with GCC and Clang.
- All six canonical configurations build and their non-GPU/non-LAN/non-retail
  suites pass 261/261; sanitizer suites use `detect_leaks=0` in the sandbox.
- Physical public Vulkan display/factory controls pass 2/2. Serial host LAN
  passes 4/4 in all six configurations.
- All three dependency-ledger checks and `git diff --check` pass.
