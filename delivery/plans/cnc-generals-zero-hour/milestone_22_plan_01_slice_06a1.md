# M22 plan 01 slice 06A1: original DX8Wrapper physical draw methods

## Dependency and outcome

Requires accepted 05B2B2B3B2B. Restore reached original `DX8Wrapper::Set_Vertex_Buffer`, `Set_Index_Buffer`, `Set_Index_Buffer_Index_Offset`, `Draw_Triangles`, `Draw_Strip`, and actual source `DX8PolygonRendererClass::Render` call contract at their existing device-edge sites in mutually exclusive CPU/device configurations. Preserve original reference/engine-reference lifetimes, index start, base vertex, vertex range and 16-bit element width; derive exact FVF from original bound vertex buffer, never duplicate geometry or material decisions. `OriginalGpuEdge` may submit only a source-bound descriptor in a caller-owned pass. Direct source method tests may issue explicit original ShaderClass/TextureClass/Material/transform state but cannot claim a category or WW3D frame. Unsupported primitive/FVF, out-of-range index/base, missing pass, stale device owner, induced bind/upload/draw failures, teardown/reset/retry must reject without partial success. Preserve prior typed category/skin/sorting edge until later slices.

## Validation and commit

Original wrapper methods and original polygon renderer, not a test adapter draw, positively issue bounded Recording indexed commands against original allocated/uploaded buffers. Source command order, exact bytes/offsets, no dummy pass or generated pixels, error rollback and owner lifetimes are witnessed. Existing B3B2B shader GPU and full GCC/Clang suites, provider-removal, ABI/link map, focused sanitizers and ledger pass before one coherent commit. Original rigid/category interleaving remains 06A2; original category lighting and additional exact FVF layouts remain 06A3.
