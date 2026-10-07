# N1 slice 01 — stock renderer semantics

Date: 2026-10-07. Result: **slice passed; N1 not yet accepted**.
Authority: `docs/zero-hour-linux-port-plan.md`, packet `linux-upstream-v2`.
No retail input, archived implementation, private SDK interface or vendor change.

## Configuration

- bgfx `cca91681c953d2de9531197b0f580c866ffaa775`
- bx `d86e4ea9d9da6e832a3ff41398587d82b772c69b`
- bimg `101b5b5fd4670f82cfdec8e98aa1ab9ee93bb2a1`
- Official origins, exact HEADs and clean tracked/untracked source status verified
  before/after stock runtime/shaderc build and after tests.
- Stock `gmake-linux-gcc`, `debug64`, `bgfx-shared-lib` and `shaderc` targets.
- Host RTX 4070, vendor `10de`, device `2786`, NVIDIA 615.71.09.
  Public capabilities report 4096 views and 65535 draws; these are capacities,
  **not** proof of a complete original scene fitting.
- GCC 16.2.1 / Clang 22.1.8 owner fixture, C++20; unmodified GCC-built shared
  dependency in all variants. Khronos validation reported in enabled instance
  layers; runner rejects validation errors and missing completion.
- Repository-owned test source SHA256:
  `027935a801beac78b2e40630fd4343b7fc6caecfbe410bc331124bc4b022a62f`.

## Requirement → source → executable case

All functions are in `tests/renderer/qualification.cpp`; CTest exposes one case
per family plus `renderer_all`. Source paths below are relative to
`GeneralsMD/Code/`.

| Requirement | Original source | Physical case and negative control |
| --- | --- | --- |
| Scoped color/depth/stencil clear | `Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp::Clear` | `viewport_and_depth`: exact inside/outside boundaries, separate depth/stencil resolve |
| Marker write mask with shadow count | `GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`, `Shadow/W3DVolumetricShadow.cpp` | `selective_stencil`: mask 0x80/0xff/0, preserved player bits, masked count increment and exact combined resolve |
| Authored mips and independent filters | `Libraries/Source/WWVegas/WW3D2/texturefilter.cpp`, `textureloader.cpp` | `mips`, `spatial_filters`: seven distinct authored levels, partial view, extreme minification, min≠mag and unclamped control |
| Scene capture/distortion order | `GameEngineDevice/Source/W3DDevice/GameClient/W3DSmudge.cpp` | `ordering_and_attributes`: draw/copy/later overwrite/shifted sample in ordered views; later scene remains green |
| Packed colors and transparency | `Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` packed layouts and material states | `ordering_and_attributes`: normalized Uint8 color and sequential alpha equation |
| Representative shader equations | `GameEngineDevice/Source/W3DDevice/GameClient/Shaders/terrain.nvp`, `fterrain.nvp` | `original_materials`: two independent nontrivial texture/diffuse equations |
| Projected shadow/render target | `Libraries/Source/WWVegas/WW3D2/matrixmapper.cpp`, `texproject.cpp` | `projected_shadows`: target capture, orthographic/varying-Q projection, multiplicative LEQUAL blend; outside-shadow and affine-substitution controls |
| Representative source formats | `Libraries/Source/WWVegas/WW3D2/textureloader.cpp` | `source_formats`: generated BC1 and BGRA8 with channel-sensitive pixels |
| Invalid fixture inputs | New fixture descriptor boundary | `admission`: empty/out-of-range/overflow/non-finite/invalid-handle rejection before submission, valid live candidate rendered afterwards |

Public stencil write-mask calling convention is established by upstream merged
[PR 3821](https://github.com/bkaradzic/bgfx/pull/3821). Texture view/LOD,
shader/uniform/program, render target, blit and readback interfaces all come from
the pinned public headers. No private include or copied backend helper is used.

## Executed matrix

Commands are reproduced in `docs/build.md`.

| Variant | Result | Elapsed |
| --- | --- | --- |
| GCC Debug, normal host | 12/12 CTest cases passed | 3.15 s |
| GCC ASan/UBSan, isolated bus | 12/12 passed, leak detection enabled | 17.87 s |
| Clang ASan/UBSan, isolated bus | 12/12 passed, leak detection enabled | 17.61 s |

Normal-host sanitizer **init-only and full** controls both reported 1,854 bytes
in three D-Bus allocations under both compilers. The empty control creates no
fixture shaders, geometry or textures. This is evidence of a host-stack leak,
not a blanket proof that the driver has no other issue. Per the user's earlier
approval, isolated child bus addresses remove this connection path; real RTX
Vulkan, enabled Khronos validation, ASan, UBSan and LeakSanitizer remain active.
No sanitizer suppressions, disabled leak detection or library patch was used.
Normal-host functional tests retain the ordinary bus environment. The normal-host
sanitizer controls remain recorded failures, not rewritten as passes.

Ignored local log paths and SHA256 identities:

| Log | SHA256 |
| --- | --- |
| `build/n1-gcc-stock-build.log` | `47e36730a253427a8b68d3f23a8b7f729c26ede495de318f901378eb4456b644` |
| `build/qualification-gcc/Testing/Temporary/LastTest.log` | `14aced8f80866c963a82755948e959c39f75a513d16bec849d7fc0e1f2d585b4` |
| `build/qualification-gcc-sanitize/Testing/Temporary/LastTest.log` | `25cda4f91c78295f48ef34fdb9cea466122668919dbb27b076ac7eb8cbc22021` |
| `build/qualification-clang-sanitize/Testing/Temporary/LastTest.log` | `3dcd7845a686ed60a5fbf1814826f9bccfe1c3c6c3f60f00341e4fed2c7fa370` |
| `build/qualification-gcc-sanitize/normal-host-init.log` | `b43edd6ebc125426f67df4500df808a80e8457ef2df1728d7f5058dcf976c480` |
| `build/qualification-clang-sanitize/normal-host-init.log` | `a168007852249b60fa9f1ad2bd2321e9ad911b9860f26aeb58507fc9be34eff4` |

## Not established by this slice

No original game startup, source allocator integration, full asset-format decoder,
complete material/visual parity, full-scene capacity, resource lifecycle plateau,
window resize/presentation or retail gameplay acceptance. Those remain N1 slice
02 and later milestones. A public API gap discovered there still fails N1.
