# M22 plan 01 slice 07E0C: original tactical view empty bootstrap

## Outcome and dependency

Requires accepted 07E0B. `InGameUI::init` creates and initializes the tactical view, attaches it to the published display, then calls `setWidth(display width)`, `setHeight(0.77 × display height)` and `setDefaultView(0, 0, 1)`. Preserve that original UI call order with bounded CPU implementations of W3DDisplay dimensions and W3DView dimensions/default view, then accept an empty no-map `updateView` only under the original display/view/terrain owner graph. Native Windows camera and display behavior remains unchanged. No normal-runtime factory switch or display draw in this slice.

## Entry, state and failure

The original display starts at the current Linux runtime's 800×600 bootstrap dimensions; its width/height setters update the base display state, rejecting zero, oversized and active-frame changes. Original view setters require initialized cameras, the published original display and valid positive viewport bounds; they update View dimensions, camera viewport/aspect and width FOV using the native source formulas. `setDefaultView(0,0,1)` applies the original default pitch/max-height rule. `updateView` accepts only the published original tactical view and no-map terrain owner with zero pending full update, no camera lock, movement, rotation, filter or unsupported effects; it leaves the empty camera/frame unchanged. All map-loaded, camera-motion and advanced modes stay typed pending. Failure must preserve prior dimensions/camera/owner pointers.

The initialized GameClient source probe temporarily publishes original display, visual and tactical view in factory order, restores the headless owners after teardown, and checks first-pass and repeated UI geometry, empty tactical update, invalid dimensions and unsupported-mode failures. The physical empty frame remains byte-identical to its earlier baseline across formats/extents/generations. The normal runtime factory remains unchanged.

## Acceptance and commit

Positive/negative source and physical controls, five rebuilt full non-GPU suites including leak-capable GCC/Clang ASan+UBSan, identity/provider/ledger checks, and repeated host Vulkan gate pass. One independent plan/source/tests/evidence commit. Local W3DDisplay draw and atomic production factory switch remain later 07 children.
