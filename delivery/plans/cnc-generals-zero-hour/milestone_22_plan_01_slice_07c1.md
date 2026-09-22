# M22 plan 01 slice 07C1: original default empty 3D scene frame

## Outcome and boundary

Requires accepted 07B2C. Translate only the default, empty/no-terrain original `RTS3DScene::doRender` to its authored `WW3D::Render(this,m_camera)` path and the reached default fog/fixed-light/flush orchestration. An initialized source GameClient scenario supplies original globals. The frame owns public Recording/bgfx attachments and preserves the original 3D scene as authority. No object, terrain, shroud, shadow, occlusion, translucent, custom pass, particle or production-display acceptance is claimed; reached unsupported nonempty/mode routes must reject before successful frame.

## Positive, negative and gate

Prove source scene/camera traversal, balanced clear-only frame and zero resource/ref leaks across two device generations. Missing edge/camera, unsupported custom pass, populated scene and stale target must reject/abort with retry and no published production display slot. Injected upload/draw failures require a reached object path and are deferred to 07C2, not simulated in a no-draw frame. Preserve native branch and class layout. Run GCC/Clang source-only sanitized, source identity/ABI/provider, four full asset-free suites and explicit validation-output checked host Vulkan readback at two extents/formats. One independent plan/source/test/evidence commit. Later 07 children add original object/terrain/shadow/effect consumers and production factory.
