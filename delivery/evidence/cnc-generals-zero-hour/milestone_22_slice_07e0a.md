# M22 slice 07E0A evidence: original empty terrain render owner

The original `BaseHeightMapRenderObjClass` and `HeightMapRenderObjClass` are compiled from their canonical source files in the full W3D GameClient target. On the Linux CPU branch, the derived original class publishes `TheTerrainRenderObject` and `TheHeightMap` only after base construction and only with an active GPU edge plus initialized W3DDisplay scene/assets. The owner has no map or terrain buffers; empty `updateCenter`, resource release/reacquire, and reset remain safe. Map initialization, map redirection, height sampling, physical terrain Render, scene registration, LOD, and other map-dependent operations reject explicitly. Native Windows behavior is unchanged. The CPU header supplies the original `DX8_CleanupHook` interface so the original inherited class layout is not silently reduced.

This is a prerequisite, not a tactical or drawn-terrain slice. In original `W3DView::update`, the reached terrain dependency is the base pointer's `doesNeedFullUpdate()` and `updateCenter()` at source lines 1254–56; the only `TheTerrainVisual` call is behind `DO_SEISMIC_SIMULATIONS`. Further camera/GameClient/TerrainLogic and original W3DTerrainVisual/production factory work remains separate. 07E draw is pending because the initialized GameClient still publishes LinuxDisplay/LinuxView; nulling `TheTacticalView` in a local-display fixture would not validate production ownership.

The initialized GameClient `original_w3d_view_scene` exercises no-edge and torn-down-display rejection, one original no-map owner, duplicate rejection, class ID/global pointer identity, map-load and no-map sample rejection, empty update/reset, direct Render rejection with no frame begun, and release clearing both singleton pointers. The same source scenario then draws a rigid original W3D mesh through the accepted W3DView/RTS3DScene route, proving that the empty terrain owner does not alter existing absence/presence pixels or resource teardown. Four bgfx target generations per process cover two extents and BGRA8/RGBA8; the Python runner rejects `Validation Error` and `VUID-` from child output.

Validation:

- GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan full non-GPU suites: 194/194 each after complete rebuilds. Focused source-only scene passed in both leak-capable sanitizer presets.
- GCC Debug and Clang Release: 30/30 fresh-process `original_w3d_view_scene_bgfx` each with Khronos Vulkan validation, four target generations per process.
- Original GameClient presentation identity and provider-removal tests include both canonical terrain files; dependency ledger hashes were updated. `git diff --check` is clean.

No retail symlink content was modified. Map tiles/textures, terrain pixels, W3DTerrainVisual ownership, tactical update, production display/UI/view factory publication, W3DDisplay::draw, shroud, tracks, water and shadows remain pending.
