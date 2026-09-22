# M22 slice 07F6C evidence: flat terrain base material selection

## Contract and source result

- Linux `TerrainTextureClass::Apply` now executes the native method's exact
  live behavior: delegate to `TextureClass::Apply(stage)`. The remaining
  terrain-specific body is source-disabled under `#if 0` and was not revived.
- Original `DX8Wrapper` delayed-state dispatch reaches the virtual terrain
  override, selects the exact F6B base-atlas handle, and creates the expected
  bilinear min/mag, nearest mip, repeating-address sampler. A repeated state
  apply is idempotent.
- Disabling WW3D texturing selects a null texture while preserving the source
  filter sequence. Explicit delayed-state retirement removes the sampler before
  map teardown. Two complete atlas/device generations return device resources
  and source allocations to their established baselines.

## Negative and dependency controls

- Injected sampler creation failure is reported before any draw, retains no
  sampler, and succeeds on retry. Existing edge controls retain invalid-stage,
  unpublished owner and prior-generation rejection.
- Recording contains no pass or draw. Active `AlphaTerrainTextureClass::Apply`
  blend state, alpha-edge material use, `W3DShaderManager::ST_TERRAIN_BASE`
  selection, `HeightMapRenderObjClass::Render`, `W3DTerrainVisual::load`, active
  effects and terrain pixels remain pending.
- Generated map/config/image inputs are workspace-owned and read-only. Retail
  inputs and retail symlink content were not touched.

## Acceptance gates

- GCC focused source test passes 1/1. Final Clang ASan/UBSan/LSan focus passes
  1/1 on the host with leak detection and halt-on-error enabled.
- Full final-tree builds pass in all five presets. Exact non-LAN/non-GPU suites
  pass 197/197 in GCC Debug, GCC Release, Clang Release, GCC ASan/UBSan/LSan
  and Clang ASan/UBSan/LSan; sanitizer suites use the canonical recoverable
  UBSan full-suite mode with LeakSanitizer enabled.
- LAN tests pass 4/4 serially in all five presets under established
  local-socket permission.
- Dependency-ledger, presentation-identity and provider-removal controls pass
  3/3. `git diff --check` passes on the slice tree.
