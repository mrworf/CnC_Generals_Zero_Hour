# M22 slice 07E0B4 evidence: original empty smudge owner

Native `W3DTerrainVisual::init` unconditionally creates `W3DSmudgeManager` after its water object, including an empty world. The canonical Linux derived source now initializes one owner under the active original display/device edge, with the original `SmudgeManager` base linked for empty-set ownership. It sets hardware support to unavailable and owns no D3D index buffer, texture or draw. Empty re-entry/reset and resource release/reacquire are stable, while active sets cannot silently reset or render. The original base header's include case/order was corrected for Linux; class layout and native Windows source behavior remain unchanged.

The initialized GameClient source probe checks no-edge and torn-down display rejection, duplicate owner non-displacement, zero frame count, unavailable effect support, active-set reset/render rejection, and singleton teardown. The owner remains live alongside the accepted original view, scenes, empty HeightMap, zero-track, disabled-shadow and no-water owners through empty and rigid source/physical frames. Physical controls retain BGRA8/RGBA8, two extents, four independent device generations per child and an empty-versus-rigid pixel oracle.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host, with leak-capable sanitized source controls.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions, four target generations each; child output rejects `Validation Error` and `VUID-`.
- Focused source, identity and provider-removal checks pass; original dependency ledger and `git diff --check` pass.

No retail symlink content was modified. Smudge effects/pixels, W3DTerrainVisual composition, production factory, tactical update and display draw remain pending.
