# M22 slice 07FD0: original decal-shadow request owner

The CPU-only original path now has an owned generated source witness for the
future shadow route. A read-only `LogicFixture` creates an actual
`W3DModelDraw` with `SHADOW_DECAL`, texture `M22SourceShadow`, size `16x8`,
and offset `1,-2`. Its real `W3DModelDraw::allocateShadows` call reaches the
original `W3DShadowManager::addShadow(m_renderObject, &shadowInfo)` path and
receives the existing typed pending-decal error before a shadow scene, queue,
or Recording resource can be published.

The dedicated probe runs this source owner through two manager generations and
then calls `GameClient::destroyDrawable`; it verifies removal from both the
object and primary scene, and zero retained Recording resources. Its marker is
`owner=1 decal-request=2 typed-negatives=3 removal=1 generations=2 resources=0`.

`SHADOW_VOLUME`, `SHADOW_PROJECTION`, and `SHADOW_NONE` are direct-manager
negative controls only. They demonstrate the default/unsupported type
boundaries but are not treated as an admission witness. No active manager
pass, shadow buffers, queue, scene ordering, pixels, retail input, water or
particle behavior is implemented by this slice; those remain 07FD and later.

Acceptance passed:

- Focused GCC Debug coupled terrain/source suite: 6/6; focused Clang Debug
  source-owner test: pass; GCC Debug identity/provider/ledger suite: 8/8.
- GCC Debug, GCC Release, Clang Debug and Clang Release exact
  non-GPU/non-LAN suites: 204/204 each.
- GCC ASan+UBSan and Clang ASan+UBSan exact non-GPU/non-LAN suites: 204/204
  each under CTest's `detect_leaks=0` ptrace limitation. The decisive host GCC
  `detect_leaks=1` source-owner/coupled terrain suite passed 5/5.
- Host Vulkan validation `original_w3d_view_scene_bgfx`: pass.
- Host serial LAN: 4/4 passed in every one of the six configured builds.
- The original dependency ledger validator and `git diff --check`: pass.

The sandbox CTest wrapper cannot collect LeakSanitizer diagnostics under
ptrace. The host focused leak-detection result above is the leak acceptance
evidence; Vulkan and UDP are likewise separately host-run rather than counted
as sandbox broad-suite coverage.
