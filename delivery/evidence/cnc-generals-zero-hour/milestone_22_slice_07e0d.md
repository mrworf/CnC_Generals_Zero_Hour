# M22 slice 07E0D evidence: local original display frame

The canonical Linux `W3DDisplay::draw` now completes one local no-map frame through the published original tactical `W3DView`, source WW3D begin/end and an identity world transform. It requires the accepted original scene/asset, terrain/effect-owner and camera graph, matching bound color/depth extent, and default display/effect modes. It rejects an active frame before starting and retires a failed draw without retaining a pass. Native Windows code and class layouts are unchanged. Normal runtime display/view factories remain headless until a separately accepted atomic publication slice.

The initialized GameClient probe temporarily publishes the original display, visual and tactical view in UI order. Direct `W3DDisplay::draw` controls show no empty geometry, changed physical pixels with one generated rigid MeshClass, and byte-identical empty pixels after detach. Recording controls reject mismatched extent, stale depth, unsupported water mode, an already active WW3D frame and missing terrain owner; injected draw failure leaves source frame/pass inactive and retry succeeds with refs intact. The host Vulkan oracle runs BGRA8/RGBA8 at two extents across four fresh device generations and two display lifetimes per generation.

Validation:

- Five fully rebuilt non-GPU suites pass 194/194 each: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan. Sanitized suites include leak-capable source controls.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions. The test rejects `Validation Error` and `VUID-` output.
- Source identity/dependency ledger checks pass with the `W3DDisplay.cpp` hash updated; `git diff --check` passes.

No retail symlink content or unrelated renderer diagnostic was changed. Map-loaded terrain, shroud, tracks, shadows, UI overlays, movies and the production factory switch remain pending.
