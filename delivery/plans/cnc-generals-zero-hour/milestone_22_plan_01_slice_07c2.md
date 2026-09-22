# M22 plan 01 slice 07C2: one original rigid object through RTS3DScene

## Outcome and boundary

Requires accepted 07C1. Extend its default/no-terrain source frame to one owned original rigid `MeshClass` with no `DrawableInfo`, so the canonical `RTS3DScene::Visibility_Check`, update list, `renderOneObject`, material/light environment, original DX8 category and flush paths determine the draw. Source mesh bytes and ownership come from a bounded generated W3D chunk fixture; do not add renderer-authored geometry. Other GameClient categories, terrain, shroud, shadow, particle, occlusion, custom passes and production display remain guarded.

## Positive, negative and gate

The source-identified Recording frame must show one visible original rigid draw with balanced source refs and no stale geometry after detach. A camera-hidden object, missing asset/object, second object or unsupported category must be absent or reject before successful frame as appropriate. Injected buffer upload and draw failures must abort/retry with no active pass or leaked source/device resources. Validate public bgfx Vulkan object pixels against clear-only absence at two extents/formats and two fresh generations under explicit Khronos layer and child output checking. Run GCC/Clang leak-capable focused source tests, four full asset-free suites, source identity/ABI/provider and ledger freshness. Commit plan/code/tests/evidence independently; no full 07 or retail acceptance.
