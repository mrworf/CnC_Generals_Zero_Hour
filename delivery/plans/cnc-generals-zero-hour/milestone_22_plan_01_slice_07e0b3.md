# M22 plan 01 slice 07E0B3: original no-water object ownership

## Outcome and dependency

Requires accepted 07E0B2. The canonical original `WaterRenderObjClass` provides the empty owner that W3DTerrainVisual::init constructs even when `GlobalData::m_useWaterPlane` is false, water type is translucent zero and both water extents are zero. It accepts those default no-water parameters with its existing parent RTS3DScene, owns no D3D/GPU textures, grid, water-track system or render pass, supports reset/update/resource release/reacquire and teardown, and leaves physical source-scene pixels unchanged. Enabled water plane, water mesh/grid, sky/cloud/reflection, wakes, water pixels, and map-loaded behavior remain typed pending.

## Entry, state and failure

Under active original edge/display owners, create one original water object and initialize with `m_useWaterPlane == false`, zero extents and `WATER_TYPE_0_TRANSLUCENT` against the original 3D scene. Re-entry with identical no-water settings is stable; reset/update/load and resource release/reacquire on that empty owner do not draw or retain a device resource. Reject no edge, missing/wrong/stale parent, duplicate water publication, positive extents, enabled water mode, unsupported type/grid/render attempts, leaving globals and scene references recoverable. The owner is not attached to a source scene until that scene's disabled-water membership semantics are separately validated. Preserve native Windows branch and class layout.

Surfaces: canonical `Water/W3DWater.cpp` plus any minimal original header/compile dependency, full-draw target, initialized source and physical fixture, identity/provider removal, ledger and evidence. No retail symlink edit, production factory switch, or terrain visual composition.

## Acceptance and commit

Recording positive/negative owner and no-draw controls, source/physical empty/rigid scene pixel consistency over BGRA8/RGBA8, two extents and repeated Khronos generations, GCC/Clang leak-capable ASan+UBSan, provider-removal/ledger and five rebuilt full non-GPU suites pass. One independent plan/source/tests/evidence commit. 07E0B4, visual composition, factory and 07E draw stay pending.
