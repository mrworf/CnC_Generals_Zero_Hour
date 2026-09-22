# M22 plan 01 slice 07E0B1: original disabled-shadow owner lifecycle

## Outcome and dependency

Requires 07E0A. The canonical original `W3DShadowManager` supports the default disabled-shadow owner lifecycle needed by W3DTerrainVisual::init/reset/teardown: when `GlobalData::m_useShadowVolumes` and `m_useShadowDecals` are both false, initialize, empty reset, empty resource release/reacquire and destruction are stable with no derived shadow managers and no draw. Enabled shadow modes and shadow creation/render remain typed pending. This is not shadow visual acceptance or factory activation.

## Entry, state and failure

Under an active original edge/display, construct a manager, initialize, reset, release/reacquire and destroy, including a second generation. An enabled volume or decal mode must reject initialization without publishing a manager or altering shadow/render state; source shadow add/render paths continue rejecting. A duplicate global manager must not displace a live owner. Preserve native Windows behavior and original class layout. Avoid an unconditional no-op when a shadow feature is enabled. The default `GlobalData` constructor sets both flags false; a scenario can enable either flag as a negative control and restore it immediately.

Surfaces: canonical `Shadow/W3DShadow.cpp`, existing header only if needed, source/physical owner fixture, provider identity/ledger, plan/evidence. Do not touch retail symlink content or unrelated renderer diagnostics.

## Acceptance and commit

Recording tests prove disabled positive lifecycle and enabled/duplicate/no-edge negative controls, no shadow queue or draw after reset, and no owner/ref leak. Existing original view/scene pixels remain unchanged across bgfx formats, extents and repeated device generations with Khronos diagnostics rejected. GCC/Clang sanitized source, provider-removal/ledger, and five fully rebuilt non-GPU suites pass. One independent plan/source/tests/evidence commit; 07E0B and 07E remain pending.
