# M22 06C3C2B3A historical physical-source blocker (resolved)

This is retained as diagnostic history. The allocator prerequisites `b6d503d` and `8937f87`, Buffer ownership prerequisite `b8e1f88`, source failed-task cleanup, leak-capable controls, and final repeated validation-enabled scene test resolved this gate. Current acceptance is recorded in [the B3A evidence](milestone_22_slice_06c3c2b3a.md); the observations below are not current blockers.

The first original `WW3D::Begin_Render` → `Render(SimpleSceneClass, CameraClass)` → `End_Render` rigid/static scene has produced expected bgfx Vulkan RGBA pixels on the host RTX driver with validation enabled. The Recording source-scene path and direct bgfx one-mip LOD0 positive/LOD1 negative also pass. This is **not** slice acceptance: repeated runs expose independent memory/teardown failures.

## Failure 1: original-source CPU memory corruption

The focused fixture loads only `TEST.TRIANGLE`, `TEST.ZERO01`, and `TEST.SUPPLY01` through the original W3D asset manager each device generation; the general source graph fixture retains its broad generated packet. Source meshes are removed and released before `WW3D::Shutdown`. A subsequent narrower scope also destructs stack `SimpleSceneClass`/`CameraClass` before Shutdown. Neither focus nor scope stabilized the test. With `ZH_M22_STATIC_CONTROL=no_readback`, `ZH_M22_STATIC_COLOR_FORMAT=bgra8`, `ASAN_OPTIONS=detect_leaks=0:abort_on_error=1`, and `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`, ASan has observed:

- An invalid virtual dispatch at `SceneClass::Render` line 238 on the second source frame, before readback.
- A crash clearing `DX8Wrapper::CpuState::texture_states` at `DX8Wrapper::Reset_Source_State` line 137, with an ASCII-pattern invalid map pointer, after the nested scene/camera scope correction.
- An invalid `MaterialInfoClass::VertexMaterials` pointer in `MaterialInfoClass::Free` → `MeshModelClass::Reset` → `WW3DAssetManager::Free_Assets` during source shutdown.

A separate GDB no-readback run crashed in the *first* source frame at `DX8Wrapper::Set_DX8_Texture_Stage_State` line 382 while source `ShaderClass::Apply` was called from original category `Flush`: stage-0 `std::map` root had invalid ASCII-pattern pointer `0x747865545f385844` (`DX8_Text...`). Another run observed corrupted buckets in `OriginalGpuEdge::vertices_`. These are multiple independent CPU structures, so a single MaterialInfo refcount hypothesis is insufficient. A hardware watchpoint on `SimpleSceneClass` vptr in a passing two-generation run saw only the legitimate destructor transition; it has not identified the first bad write. No-readback reproduction excludes diagnostic readback as a necessary cause. Pinned bgfx Vulkan `ReadbackVK::pitch` is exactly width × 4 for both RGBA8/BGRA8, and device submissions use `bgfx::copy`, not caller-lifetime `makeRef`.

The ASCII prefix resembles a source state marker, but the physical device `record_marker(std::string_view)` is a synchronous no-op; Recording converts the view to an owned string immediately. Marker-view retention is therefore not demonstrated. The first corrupting write still needs a stack/watchpoint before changing production state ownership.

## Failure 2: second-generation native shutdown

Source pixels, `wait_idle`, zero original VB/IB allocations, and exact public-handle retirement (two caller-owned color/depth targets after original edge/WW3D shutdown, zero after target destruction) have completed before an intermittent second-device shutdown stall or render-thread crash. One captured fault stack reaches bgfx Vulkan `CommandQueueVK::destroy<VkImage>` through `libVkLayer_khronos_validation.so` during `bgfx::shutdown`. A separate GDB repeat crashed on the second generation *during frame submission*: render thread `libVkLayer_khronos_validation.so` → bgfx Vulkan `CommandQueueVK::kick(false)` → `RendererContextVK::submit` → bgfx renderFrame, while the main thread was in original `ShaderClass::Apply` inserting a texture stage state. Public handle counts do not prove deferred Vulkan destroy safety. A device-only public bgfx two-generation RGBA8 control passed with validation, while source-scene runs remain intermittent. A render-thread backtrace at the *stall*, rather than the crashes, remains outstanding; do not attribute either fault to external Vulkan until source memory corruption is resolved or excluded.

No timing-based drain increase, validation bypass, source-provider substitution, or B3A acceptance is justified by these observations. The slice remains uncommitted and blocked pending the first corrupting write/lifetime root cause and repeatable source-scene stress under validation/ASan.

## Bounded re-entry isolation (Clang ASan+UBSan, Vulkan validation)

All cases use the same focused three original W3D meshes; `no_readback` omits diagnostic readback only. The added controls are diagnostic, not acceptance:

| Case | Repeats | Result |
| --- | ---: | --- |
| Full source scene via Recording, 50 generations × 10 fresh processes | 500 generations | pass |
| Original Begin→End only via Recording | 50 generations | pass |
| One bgfx device generation, full source scene, no readback | 8 fresh processes (GCC ASan: 10) | pass |
| Two bgfx generations, original assets/edge, no frame | 8 fresh processes | pass |
| Two bgfx generations, original assets/edge, edge begin/end only | 8 fresh processes with teardown trace | pass |
| Two bgfx generations, direct public outer clear without original runtime | 12 fresh processes | pass |
| Two bgfx generations, direct public outer + inset clear | 8 fresh processes | pass |
| Two bgfx generations, original Begin→End only, no scene or geometry | 2 pass, 1 bounded timeout with teardown trace | fail |

The bounded original Begin→End failure completed first bgfx destructor (`destroy resources` → `frame begin` → `frame done; shutdown begin` → `shutdown done`), then completed the second source `wait_idle` and printed `done`. The second bgfx destructor printed `destroy resources` and `frame begin`, but never returned from `bgfx::frame()` before `timeout 45s` exited 124. This is a concrete deferred-frame stall, not merely a slow shader compile. The first failing CPU write remains unknown; no per-draw buffer creation is required for this narrow failure. The source Begin→End path additionally calls TextureLoader update, dynamic VB/IB reset, source sorting Flush, statistics and source-state reset; each is a remaining candidate or an interaction with bgfx teardown. No source/provider behavior has been changed to mask the issue.
