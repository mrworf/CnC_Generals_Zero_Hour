# M22 slice 08B3 — aggregate volumetric-shadow closure

This aggregate composes 08B3A's original source slots and CPU side-wall
geometry, 08B3B's public three-pass stencil contract, 08B3C0's XYZ/16 bridge,
and 08B3C's bounded original `SHADOW_VOLUME` owner in one generated,
two-generation Recording frame.  The new aggregate gate requires the complete
terrain/tracks/volume/water order, rollback then retry, default typed rejection,
provider removal, and zero ownership.  It adds no retail route, pixel claim,
raw Direct3D, or private Vulkan API.

Acceptance recorded on the final aggregate tree:

- `original_w3d_volumetric_shadow_aggregate`: generated two-generation
  Recording execution, including the prerequisite slots, geometry, public
  three-pass edge and bounded owner witnesses, plus the existing missing,
  typed, rollback/retry, removal and teardown controls.
- Fresh non-GPU/non-LAN CTest suites: 213/213 in GCC Debug, GCC Release,
  Clang Debug, Clang Release, GCC Sanitized and Clang Sanitized.  Sanitizer
  broad suites use `detect_leaks=0` only for the host ptrace limitation.
- Leak-enabled focused Clang sanitizer route: 3/3; physical public Vulkan
  stencil-edge validation: 1/1; serial LAN boundary suites: 4/4 in each of
  the six configurations.
- `tools/check_original_dependency_ledger.py` and `git diff --check` pass.

The physical test validates the already-accepted public edge used by this
composition.  The aggregate itself remains a Recording/order-and-ownership
claim, not a retail or pixel-output claim.
