# M22 slice 07E0B1 evidence: disabled original shadow owner

Native `W3DTerrainVisual::init()` constructs `W3DShadowManager` unconditionally after terrain and track ownership. `GlobalData` defaults both `m_useShadowVolumes` and `m_useShadowDecals` to false. The canonical manager's Linux CPU branch now initializes exactly one disabled-shadow owner with an active original edge/display, supports empty reset/resource release/reacquire, and clears only its own global publication on destruction. Enabled volume/decal mode, duplicate owner, no edge, and torn-down display reject; original shadow add/render remains typed pending. Native Windows code and class layout are unchanged.

The original `RTS3DScene` Linux source guard formerly rejected any non-null shadow manager, even when both modes were disabled and no shadow was queued. It now allows that empty owner to coexist with the existing zero/one-rigid source scene, while rejecting an enabled mode or queued shadow. Recording tests cover manager re-entry, duplicate and enabled-mode negatives separately, queued-shadow draw abort/retry, reset and publication teardown. The manager remains live during source and physical W3DView frames; four target generations per process check two extents and BGRA8/RGBA8 with unchanged rigid/absence pixels and explicit Khronos diagnostic rejection.

Validation:

- Focused source/identity/ledger tests pass on GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan. The sandboxed sanitizer invocation hit LSan's documented ptrace environment limitation; rerunning both leak-capable controls on the host passed. That environment failure is not an ASan/UBSan source failure.
- GCC Debug and Clang Release `original_w3d_view_scene_bgfx` each passed 30/30 fresh-process Khronos Vulkan repetitions, four target generations per repetition.
- All five fully rebuilt non-GPU suites pass 194/194 each; provider-removal and dependency ledger pass; `git diff --check` is clean.

No retail symlink content was modified. Zero-track, no-water and no-smudge owner prerequisites, W3DTerrainVisual composition, original production factory, tactical update, display draw, and all enabled shadow rendering remain pending.
