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
