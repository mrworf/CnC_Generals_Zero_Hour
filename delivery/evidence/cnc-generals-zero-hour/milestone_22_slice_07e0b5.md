# M22 slice 07E0B5 evidence: disabled-water original scene membership

Native `W3DTerrainVisual::init` attaches its water object to the original 3D scene even in the default disabled-water configuration. The source probe now makes that same `Add_Render_Object` call, verifies the scene owns a second ref and the water object's parent is the original 3D scene, then detaches it before owner destruction and display shutdown. The canonical `RTS3DScene` Linux path recognizes only the published original water identity under disabled water/cloud flags and skips it in visibility, update, object and flush traversals. It does not relax the one-rigid-object limit or accept enabled water.

Recording controls turn on the water plane while it is attached and require a failed, recoverable frame with intact owner refs and no active pass. They also attach a second rigid object and require the existing unsupported-object failure. Empty, one-rigid and detached frames retain the established source and physical pixel oracle while water remains scene-owned. Its removal restores the owner's one ref and clears its scene pointer before teardown.

Validation:

- Five complete rebuilt non-GPU suites: GCC Debug, GCC Release, Clang Release, GCC ASan+UBSan and Clang ASan+UBSan each pass 194/194 on the host, with leak-capable sanitized source controls. An initial ledger-only failure was resolved by updating the changed original scene source hash; all five suites then passed.
- GCC Debug and Clang Release original view-scene bgfx each pass 30/30 fresh-process Khronos Vulkan repetitions, four target generations each; child output rejects `Validation Error` and `VUID-`.
- Focused source, identity and provider-removal controls, original dependency ledger and `git diff --check` pass.

No retail symlink content was modified. Active water pixels, W3DTerrainVisual composition, production factory, tactical update and display draw remain pending.
