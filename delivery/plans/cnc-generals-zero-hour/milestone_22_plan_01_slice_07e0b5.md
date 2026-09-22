# M22 plan 01 slice 07E0B5: disabled-water original scene membership

## Outcome and dependency

Requires accepted 07E0B3 and 07E0B4. Native `W3DTerrainVisual::init` adds its original `WaterRenderObjClass` to `W3DDisplay::m_3DScene` even when the water plane is disabled. The accepted no-water owner currently remains unattached; close that source-semantic gap before visual composition. The original scene must own a ref and recognize precisely its published, zero-extent, disabled water object as non-rendering while preserving one rigid object's physical pixels and existing unsupported-object rejection. Enabled water, cloud/grid, foreign objects and water rendering stay typed unavailable.

## Entry, state, failure

In the initialized GameClient source probe, add the original no-water object to the original 3D scene through `Add_Render_Object`, verify parent/refcount and stable empty/rigid/detach pixels, then remove it before owner release and display teardown. The CPU scene traversal, visibility, update and flush skip only this disabled original water identity; switching its mode to enabled must reject the frame and recover without draw/pass leakage. An unsupported second object remains rejected. Repeat over fresh physical device generations. Preserve native Windows scene behavior and original class layouts.

Surfaces: canonical W3DScene CPU branch, original view-scene source/physical fixture, identity/provider check if required, ledger, evidence. No terrain-visual implementation or factory switch in this slice.

## Acceptance and commit

Positive/negative scene-ref and pixel controls, five rebuilt non-GPU suites including GCC/Clang leak-capable ASan+UBSan, 30-run GCC/Clang host Vulkan source-scene gates, provider/ledger checks and clean diff pass. One independent plan/source/tests/evidence commit. W3DTerrainVisual composition remains pending.
