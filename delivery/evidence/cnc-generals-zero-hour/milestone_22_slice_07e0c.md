# M22 slice 07E0C evidence: original tactical view empty bootstrap

`InGameUI::init` creates, initializes and attaches the tactical view before calling width, 0.77-height and default-view setters. The canonical original Linux `W3DDisplay` now starts at the current runtime's 800×600 dimensions and accepts bounded published width/height changes outside an active WW3D frame. The canonical original `W3DView` follows the UI setter order, updating its inherited dimensions, camera viewport, aspect ratio and width field of view using the native formulas. The default `(0,0,1)` view applies the native default pitch and maximum-height rule. A published original tactical view with the original no-map terrain owner accepts a no-op empty update; camera lock, map/full-update, unsupported movement/filter and advanced view modes remain typed pending. Native Windows paths and class layouts are unchanged.

The initialized GameClient probe temporarily publishes original display, visual and tactical view in factory/UI order, then restores the pre-existing headless owners after original teardown. Direct source controls verify bootstrap and viewport/aspect, repeat the empty update, reject zero/oversized display dimensions, zero/out-of-bounds view dimensions, an unpublished view, advanced default, camera-locked update and active-frame display resize without changing accepted dimensions or owner pointers. The composed empty physical frame remains byte-identical to the earlier empty baseline; the pre-existing one-rigid oracle stays intact.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host, with leak-capable sanitized source controls.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions. Each child covers BGRA8/RGBA8, two extents, four device generations and two original display lifetimes per generation; child output rejects `Validation Error` and `VUID-`.
- Focused source, identity and provider-removal checks pass; original dependency ledger and `git diff --check` pass.

No retail symlink content or normal runtime factory was modified. Local original display draw and atomic production factory publication remain later 07 children; terrain-loaded tactical motion remains pending.
