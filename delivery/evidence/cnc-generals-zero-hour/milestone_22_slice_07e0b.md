# M22 slice 07E0B evidence: original empty terrain visual composition

The canonical `W3DTerrainVisual` Linux branch now composes the native empty-startup owner sequence: `HeightMapRenderObjClass`, zero-track system, disabled `W3DShadowManager`, no-water `WaterRenderObjClass`, and empty `W3DSmudgeManager`; it then attaches the disabled water object to the original 3D scene and applies only accepted zero-grid defaults. It preserves the original class layout and native Windows source. Map load, terrain tiles, enabled effects, tactical update, factory publication and display draw are typed pending. On destruction, it detaches water and releases only its owned children before the display shuts down WW3D.

The initialized GameClient probe temporarily publishes this original class in the production factory-before-init order, then restores its pre-existing headless visual after original-owner teardown; it does not switch the production factory. Controls cover no-edge and enabled-water preinit rejection without partial publication, duplicate visual non-displacement, idempotent init, reset/update, map-load rejection, exact child globals and water scene refcount, and teardown before display shutdown. A second original display/view lifetime within each device generation draws an empty frame identical byte-for-byte to the accepted first empty baseline. The earlier one-rigid source/physical frame oracle remains in the same probe.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host, with leak-capable sanitized source controls.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions. Each child covers BGRA8/RGBA8, two extents, four physical device generations and two original display lifetimes per generation; child output rejects `Validation Error` and `VUID-`.
- Focused source, identity and provider-removal controls pass; original dependency ledger and `git diff --check` pass.

No retail symlink content was modified. Production factory, tactical update and display draw, plus map-loaded terrain and enabled effects, remain pending 07E children.
