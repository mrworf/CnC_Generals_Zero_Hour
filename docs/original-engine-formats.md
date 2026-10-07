# Original engine findings for the clean restart

This is a source reference, not a claim that the new implementation passes tests.
The baseline is `0a05454d8574207440a5fb15241b98ad0b435590`. Prior implementation
and evidence are archived; no old milestone acceptance transfers automatically.

## Rendering requirements verified during restart assessment

- `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h` declares viewport
  clear, transforms, packed layouts, texture stages, copy rectangles, 2D/cube/3D
  textures and render targets. Translate actual call-site semantics; this is not
  a mandate to emulate every D3D8 API.
- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`
  writes only stencil bit `0x80` for potential occluders while preserving player
  color bits. Stencil read masks and write masks are different requirements.
- `.../GameClient/Shadow/W3DVolumetricShadow.cpp` shares stencil meaning with
  marker/occlusion rendering. Test their combination, not independent shadows.
- `.../GameClient/W3DSmudge.cpp` acquires/copies background scene content and
  resets WORLD/VIEW transforms for its screen-space effect. The retired port's
  simplified atlas/black-fan fixture is not a visual parity reference.
- `.../GameClient/Water/W3DWater.cpp` selects multiple textures, texture
  transforms, shader variants and reflection behavior. Preserve the selected
  source path rather than substituting one generic transparent surface.
- `.../WW3D2/texturefilter.cpp` owns DEFAULT/BEST filter resolution. Initialize
  actual profile tables in fixtures; zeroed tables can invent no-mip behavior.

## Reusable ownership cautions

Original global allocation is replaced by memory-pool services. Pair ownership
at allocation and deletion, including library temporary/nothrow paths. A failed
constructor does not invoke its destructor; guard acquired references before
later throws. Native resource handles may deduplicate while acquiring additional
reference units. None of these facts require a private backend inventory API.

Descriptor mip count is logical metadata, not proof of initialized GPU backing.
Packed source vertex slots and source-selected coordinate transforms are shader
protocols; transport names are not semantic authority. New stock shader layouts
may differ when explicit translation preserves those semantics.

## Pending evidence

N1 must verify all renderer claims on a pristine stock dependency build. N2 must
reinspect native persistence layouts and original allocator service boundaries
before implementing them. Do not copy historical ABI assumptions into file
formats; derive fixed-width encodings from the original reader/writer pair.

## Fresh stock-bgfx source findings (N1 semantics; whole milestone pending)

Official bgfx `cca91681c953d2de9531197b0f580c866ffaa775` was acquired directly
from upstream into a fresh build tree. Public `setTexture` supports a mip/layer
view and quarter-mip `lodMin/lodMax` limits. This replaces the retired custom
base-level flag mechanism. `tests/renderer/qualification.cpp::mips` and
`spatial_filters` prove authored ranges and distinct min/mag spatial selection
on the RTX Vulkan device. This is not yet full original filter-table integration.

Upstream merged stencil write masks in PR3821 (merge
`7b3834644276012ab6643c45965cdfcf9b6ee457`, API148). The second public
`setStencil` argument carries the write mask in its RMASK field; the first
argument carries the compare mask. The public signature's prose is terse;
the official merged change establishes the intended calling convention:
https://github.com/bkaradzic/bgfx/pull/3821
The generated test uses public headers/macros only and tests 0x80/0xff/0x00.
No copied private helper or dependency modification is used.

The new upstream revision uses `TextureRegion` for blit/read and no longer
exports the old texture-blit/readback capability bits. Validate the required
formats/usage with public `isTextureValid` and actual readback tests. This is an
ordinary game-side API adaptation, not a reason to modify dependency sources.

## Projector and material representation

`GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/matrixmapper.cpp`,
`Compute_Texture_Coordinate` computes S/T from ViewToPixel rows 0/1, Q from
row 3. `Apply` installs rows 0/1/3 with PROJECTED|COUNT3 for perspective;
orthographic mapping uses COUNT2 without division. `texproject.cpp`,
`Pre_Render_Update` composes projector projection, inverse projector transform
and camera transform. `Init_Multiplicative` uses ZERO/SRC_COLOR blending,
LEQUAL and disabled depth writes. `Configure_Camera` insets the capture viewport
by one texel. These are game shader/state semantics, not a required raw D3D ABI.

The generated `projected_shadows` case uses explicit stock Mat4/STQ transport,
fragment division, a generated target and multiplicative blend. Its varying-Q
control distinguishes division from an affine substitute. Full original camera,
depth-gradient and intensity integration remains N3 work. Commented ZBIAS lines
in `W3DVolumetricShadow.cpp` are not an active depth-bias requirement; do not
promote them into a mandatory public API gap without a reachable caller.

`GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Shaders/terrain.nvp`
mixes texture1/texture0 with diffuse alpha then multiplies by diffuse.
`fterrain.nvp` multiplies both textures and diffuse. Owned `fs_terrain.sc` and
`fs_flat.sc` implement those equations with explicit stock inputs; generated
pixels cover nontrivial color/alpha, without copying a prior shader ABI.

`WW3D2/textureloader.cpp` has explicit DXT1/3/5 and A8R8G8B8 paths. N1's
`source_formats` covers generated BC1 and BGRA8 uploads, not a complete format
decoder or all asset formats. Engine decoding and format dispatch remain N2/N3.

See `evidence/qa/N1-stock-renderer-semantics.md` for current configuration and
acceptance boundaries. None of these tests load proprietary assets.
