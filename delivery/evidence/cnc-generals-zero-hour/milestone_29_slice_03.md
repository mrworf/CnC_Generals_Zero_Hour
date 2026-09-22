# M29 slice 03: offline bgfx shader closure

Scope: repository-owned shader families only. The original-content symlink was
not read, copied, hashed or modified. No new physical bgfx pixels are claimed.

## Implementation

- Pinned public bgfx/bx/bimg revisions, reviewed BSD-2-Clause license SHA-256,
  and exact upstream shaderc patch are recorded in `third_party/`.
- `bootstrap_bgfx_shaderc.sh --offline-sources` verified three supplied public
  checkouts read-only, staged committed revisions to an ignored owned cache,
  applied exactly the reviewed patch there, and built shaderc 1.19.161. A
  second run completed idempotently. `--acquire` is the explicit one-time
  network route; configure, build and test have no network operation.
- Fail-closed GLSL lowering maps source UBO/texture declarations to bgfx's
  Vulkan set-0, one stage-UBO, separate image/sampler binding model. The
  generated JSON manifest retains source block and texture-stage identity.
  The reviewed patch additionally preserves source vertex attribute locations
  and 16-byte integer vector reflection.
- All 39 renderer/effects and original-FVF variants are built and installed
  alongside 39 manifests. Historical SDL_GPU SPIR-V stays unchanged.

## Positive validation

- `cmake --preset` and full `cmake --build --preset` passed for Linux GCC and
  Clang, Debug and Release. Each preset's `zh_bgfx_shaders` target and two
  focused CTests passed. The four independently built 39-binary inventories
  had the same aggregate SHA-256:
  `94ab350bd2a405eddb45deb63392864ef244d245d85b8a59f5c0c3bcb17a15b0`.
- `verify_bgfx_shaders.py` passed the exact family set, bgfx stage/container
  version, SPIR-V validity, set/binding map, texture reflection, original-FVF
  integer uniforms and vertex attributes in all four presets. WWShade's four
  source UBOs are retained as named members of one bgfx stage UBO. Only the
  three `world.frag` textures multiplied by literal zero are accepted as dead;
  the verifier requires their exact source usage shape.
- `cmake --install` into a temporary prefix produced 39 `.bin` files and 39
  `.json` manifests. `renderer_inventory.py --check` reported 504 identifiers
  / 13 rules. Focused public-header, inventory, recorder and world-corpus tests
  passed. The full asset-free GCC Debug suite (`ctest --preset linux-gcc-debug
  -LE 'gpu|retail' -j 4`) passed 172/172, with local UDP test permission.

## Negative validation and preserved boundary

- The bridge unit test rejects unsupported descriptor sets/bindings, duplicate
  texture bindings, unhandled descriptors, malformed bgfx envelopes, missing
  families, wrong descriptor set and missing reflected textures.
- Configure with a missing pinned shaderc failed. An offline-source checkout
  at the wrong Git revision failed before any staging/patching. The staged
  patch's reverse-application and exact Git diff matched the committed patch;
  altered/unreviewed staged changes fail its bootstrap check.
- The full suite initially exposed `world_corpus` accumulating ordered views
  over 64 logical frames with no presentation. Its isolated test consumer now
  presents the completed world color target after each frame; recorder's
  within-frame exhaustion and reset-on-present negative/positive tests remain.
  Unchanged LAN tests initially failed only because sandbox UDP sockets were
  denied; rerunning them with local socket permission passed 2/2.

M30 owns bgfx physical device, program creation, binding/pixel/format and
presentation acceptance. M22 owns the complete original WW3D scene route and
retail evidence. SDL3 platform/input and the existing CPU baseline were
preserved.
