# M22 plan 01 slice 07FG0: public bgfx multi-mip sampled texture lifecycle

## Goal and outcome

Make the public Vulkan bgfx device faithfully own a bounded sampled 2D source
texture with its declared mip chain.  A caller can allocate a supported
RGBA8/BGRA8/A1R5G5B5 sampled 2D texture, upload every declared mip at its exact
extent, destroy it, and recreate it without retaining a live native resource.
This is the physical device prerequisite for the generated terrain atlas used
by 07FG; it does not publish that map route.

## Scope and non-scope

Included: public `BgfxGpuDevice` allocation, level-aware upload validation,
native bgfx mip creation, stale-handle rejection, allocation rollback, and a
direct physical Vulkan contract test across two ownership generations.

Excluded: production factory/map loading, Recording substitution, retail
content, resizing, terrain/material draw behavior, pixels or visual claims,
compressed/depth upload expansion, and arbitrary partial mip regions.

## Dependencies and boundary

Requires completed 07FF.  07FG depends on this slice.  The slice stays below
the original atlas consumer: its test exercises `BgfxGpuDevice` directly and
does not create a production map or change GameClient factory behavior.

## State, validation, and recovery

`TextureDesc.mip_levels` is accepted only for a supported sampled 2D color
format with an extent-valid complete-prefix mip count.  Render targets remain
single-level.  Each upload must target a live texture, a declared level, and
the exact shifted extent with a format-valid pitch and byte count.  Invalid
format, extent, level, pitch, byte count, stale handle, and oversize mip count
fail closed without allocating or mutating ownership.  If native allocation or
slot insertion fails, the temporary bgfx handle is destroyed.  Destruction
retires only the owned texture; recreation gets a new opaque generation and
two complete device generations end with zero live resources.

No authorization boundary applies: this is in-process, asset-free public
renderer transport.

## Implementation surfaces

- `src/renderer/bgfx_device.cpp`: bounded mip-count and per-level upload
  support.
- `tests/renderer/test_bgfx_mip_texture.cpp`: direct physical Vulkan positive,
  negative, rollback/recreation, and lifecycle contract.
- `CMakeLists.txt`: independent non-GPU contract and explicit physical Vulkan
  test registration.
- The existing original-runtime dependency ledger is validation-only for this
  adapter slice: no original source identity changes, so no ledger row is
  invented or rewritten.
- This plan, its governing-plan index, and slice evidence.

## Validation and acceptance

1. Focused GCC Debug contract proves complete four-level upload, malformed
   extent/level/pitch/byte-count and invalid descriptor rejection, stale
   rejection, recreation, and two-generation zero ownership.
2. The same direct binary under validation-enabled Vulkan is clean: no
   `Validation Error` or `VUID-` output.
3. Build and run the proportional non-GPU/non-LAN CTest suite in all six
   configured GCC/Clang Debug/Release/ASan+UBSan builds; sanitizer broad CTest
   may use `LSAN_OPTIONS=detect_leaks=0` only for ptrace, followed by focused
   host `detect_leaks=1` proof.
4. Run the repository serial LAN exception in all six builds, then final
   identity/provider/ledger and `git diff --check` checks.

## Commit boundary

Commit exactly the 07FG0 plan/index, implementation, independent test/CMake
registration, ledger update, and evidence in one reviewable slice commit.
Leave the pre-existing `tests/renderer/test_bgfx_device.cpp` diagnostic
unstaged and unchanged.
