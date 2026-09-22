# M22 slice 07E0B2 evidence: original zero-track owner

Native W3DTerrainVisual::init constructs `TerrainTracksRenderObjClassSystem` unconditionally; `GlobalData` defaults `m_maxTerrainTracks` to zero. Its canonical Linux source now publishes one zero-module owner only under the active original edge and matching W3DDisplay scene. Empty re-init/reset/update/flush and resource release/reacquire retain no modules, scene-owned tracks, or pending GPU buffers. Destructor clears only its own singleton. Missing edge, wrong or torn-down display scene, and duplicate owner reject without displacement. The previously accepted nonzero source pool/bind/cap/unbind route and active-flush typed GPU rejection remain unchanged.

An initialized GameClient source probe keeps the zero-track owner alive alongside the accepted disabled-shadow, empty HeightMap, W3DView and RTS3DScene owners. Positive and negative controls check publication, duplicate/no-edge/wrong/stale scene, null `bindTrack`, no pending buffers, reset/update/flush and teardown. The same source/physical scene retains no-object versus generated rigid pixels; bgfx covers BGRA8/RGBA8, two extents and four independent device generations per process, rejecting `Validation Error` and `VUID-` child output.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host. Existing active-track source regression remains included.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions, four target generations each.
- Original provider-removal and dependency-ledger checks pass; `git diff --check` is clean. Host sanitizer runs retain leak detection; a sandbox-only LSan ptrace limitation is not counted as a product failure.

No retail symlink content was modified. Active track pixels, no-water and no-smudge owner prerequisites, W3DTerrainVisual composition, production factory, tactical update and display draw remain pending.
