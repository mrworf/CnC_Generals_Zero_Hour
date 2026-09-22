# M22 slice 07F6A evidence: packed terrain texture transport

## Contract and source result

- `TextureFormat::bgr5a1` is appended to the public renderer enum, preserving
  every established value. `OriginalGpuEdge` maps source
  `WW3D_FORMAT_A1R5G5B5` to that exact sampled 2D format.
- Recording preserves the four authored 16-bit patterns byte-for-byte. SDL
  maps them to `SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM`; bgfx maps them to
  `bgfx::TextureFormat::BGR5A1`.
- A physical 2x2 nearest-sampled texture produces opaque red, green and blue
  quadrants plus an alpha-clear quadrant on both public Vulkan backends. Each
  test process performs two complete device generations and retires all owned
  resources.

## Negative and dependency controls

- Both devices report and enforce sampled-only support: packed render-target
  capability and creation fail. Short rows, unsupported original formats,
  injected creation failure and injected upload failure fail before acceptance;
  retry succeeds and teardown returns Recording resource counts to zero.
- The first physical negative exposed SDL creation accepting a usage its
  capability query rejected. `create_texture` now enforces the same public
  capability result before native allocation. A first enum placement also
  shifted existing values; appending the new value restored ABI stability, and
  rebuilt presentation identity/provider-removal controls pass.
- Terrain atlas packing, `TextureClass` atlas ownership, materials, terrain
  submission, `W3DTerrainVisual::load`, active effects and terrain pixels remain
  pending. Generated shader/test bytes are workspace-owned; retail inputs and
  retail symlink content remain untouched.

## Acceptance gates

- Final Clang ASan/UBSan/LSan focused Recording and original-edge controls:
  2/2. Leak detection is enabled for these CPU/source tests.
- Final exact-tree non-LAN: GCC Debug 196/196, GCC Release 196/196, Clang
  Release 196/196, GCC ASan/UBSan/LSan 196/196 and Clang ASan/UBSan/LSan
  196/196. Full builds pass in all five presets.
- LAN tests pass 4/4 serially in all five presets under established local-socket
  permission. A parallel fixed-port collision was invocation-only diagnostic
  and is not acceptance evidence.
- Khronos-validation physical shader sampling passes SDL 10/10 and bgfx 10/10;
  a final rebuilt-binary run passes 2/2. `detect_leaks=0` is scoped only to
  these GPU processes because SDL/DBus retains process-global allocations;
  validation-output scanning remains active. CPU/source leak-capable gates stay
  enabled.
- The dependency ledger checker and all ledger tests pass; `git diff --check`
  passes on the slice tree.
