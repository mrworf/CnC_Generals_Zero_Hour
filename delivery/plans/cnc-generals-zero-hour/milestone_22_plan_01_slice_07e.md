# M22 plan 01 slice 07E: original W3DDisplay bounded view frame

## Outcome, dependency and non-scope

Requires accepted 07D and separately delivered original terrain ownership and production-compatible display/UI/view publication children. Investigation of the initialized GameClient found `TheTacticalView` is a published `LinuxView`, not a local W3DView: `InGameUI::init()` creates it through the current Linux factory and attaches it to LinuxDisplay. Native `W3DView::update()` unconditionally uses `TheTerrainRenderObject`; a fixture that nulls the tactical singleton to draw a local W3DDisplay would not exercise production ownership. Therefore the former no-terrain local-display frame proposal is rejected. After the dependencies are accepted, the canonical original `W3DDisplay::draw()` Linux CPU branch can own a bounded default black-clear `WW3D::Begin_Render` → original `Display::drawViews` → `WW3D::End_Render` transaction for a production-published original W3DView and an empty terrain baseline. All reached shroud, tracks, water, particles, shadows, UI, movie, dynamic LOD, view filters and other tactical modes remain guarded.

## Entry, state, errors and surfaces

An initialized GameClient source scenario obtains the published original display and original tactical view from the production-compatible owner path, binds Recording/bgfx targets, and calls `W3DDisplay::draw()` directly. Zero-object output is black; attaching one generated original rigid W3D object changes physical pixels. The display rejects missing edge/owners/view/camera/target/terrain, multiple or non-original views, an already active WW3D frame, and unsupported global modes before beginning a successful frame. A failed original scene draw must leave WW3D and the public pass inactive so the same display/view can retry. No new authorization or user data mutation applies. Surfaces: `W3DDisplay.cpp` CPU draw branch, scenario probe/test, ledger, plan/evidence. Investigate exact original global predicates and abort ownership; do not add an artificial success path for unsupported operations.

## Acceptance and commit

Positive and negative Recording tests cover empty/rigid/detached source frames, view ownership, no-edge/no-view/stale-target/extra-view rejection and injected draw abort/retry. Public bgfx Vulkan compares absent/visible/detached pixels across BGRA8/RGBA8, two extents and fresh device generations with explicit Khronos diagnostic rejection. Verify GCC/Clang leak-capable ASan+UBSan source controls, provider/ABI and ledger identity, fully rebuilt mandated asset-free suites and repeated physical runs. One plan/source/tests/evidence commit. Production factory, tactical frame updates and advanced scene families remain later 07 children.

## Result

Accepted children 07E0D through 07E0F close this bounded no-map frame. The
canonical display/view/scene path has Recording failure/retry coverage and a
BGRA8/RGBA8, two-extent physical matrix; the production-order opt-in factory
publishes the same original owners and proves empty and generated-rigid pixels
with clean teardown. This aggregate does not promote the opt-in factory to the
default runtime and does not accept map terrain or enabled shroud, tracks,
water, shadows, particles, or other effects. Full slice 07 and retail slices
08–09 remain pending. Evidence: [07E](../../evidence/cnc-generals-zero-hour/milestone_22_slice_07e.md).
