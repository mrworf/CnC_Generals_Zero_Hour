# M22 slice 07E0B3 evidence: original no-water owner

Native `W3DTerrainVisual::init` constructs `WaterRenderObjClass` even for the default no-water configuration. The canonical Linux source now initializes that original object with zero water extents and translucent-zero type only under an active GPU edge and the matching original 3D scene. It publishes one owned global, supports idempotent empty reset/update/load and resource release/reacquire, and releases only its own publication on teardown. It neither acquires water textures/grid buffers nor joins a render scene. The original header's two redundant in-class qualifications were removed for Clang parsing without changing class layout or native behavior.

The source probe checks no-edge, wrong/stale parent, enabled-water, duplicate owner, nonzero extent, unsupported water type, grid and direct-render rejection, plus re-entry and teardown. These failures leave the accepted owner and source scene intact. Its physical view-scene control retains the established empty-versus-rigid pixel oracle in BGRA8/RGBA8, two extents, and four device generations per child.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host, with leak-capable sanitized source controls.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions, four target generations each; child output rejects `Validation Error` and `VUID-`.
- Focused source, identity and provider-removal checks pass; original dependency ledger and `git diff --check` pass.

No retail symlink content was modified. Enabled water, map-loaded terrain, smudge owner, terrain visual composition, production factory, tactical update and display draw remain pending.
