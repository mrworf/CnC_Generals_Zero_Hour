# M22 plan 01 slice 06C3C2B1: canonical WW3D scene wrapper

## Goal and boundary

Requires accepted C2A. Restore source `WW3D::Render(layer list/layer/scene/object)` in the Linux full configuration. A recording frame invokes original Begin, source camera update/apply, optional C1 clear, source solid fill/ambient choice, C2A scene traversal, original Flush, and End in order. This proves source wrapper reachability and decision order only; B2 owns static-sort exception safety and B3 owns physical pixels/aggregate acceptance.

## Invariants and failure

Preserve original methods and `TheDX8MeshRenderer.Flush`/`SHD_FLUSH`/static-sort/sorting order. CPU state supports original SOLID fill token only; point/line/extra pass reject before camera/scene publication. Require active original frame, non-null scene/camera/layer members and selected edge before side effects. On camera, clear, fog, hook or queue failure, abort the original source frame, reset selected state/queues where original cleanup surfaces permit, and prove a fresh frame retry. No synthetic scheduler or private content. Do not claim success from a manually flushed mesh.

Reached initialization dependency: native `DX8Wrapper::Do_Onetime_Device_Dependent_Inits` initializes `TheDX8MeshRenderer` category lists and native shutdown releases them, while the CPU `WW3D::Init` omits `DX8Wrapper::Init`. Restore that original lifecycle at the CPU source WW3D Init/Shutdown boundary (idempotent with earlier fixture-level Init and safe on partial initialization). Never allocate a list in Flush or only in a test. Verify init failure and retry before accepting this slice.

Reached layer-source dependency: `WW3D::Render(LayerClass/LayerListClass)` uses canonical `LayerClass` constructors/refcounted scene and camera/destructors from original `layer.cpp`, but the CPU `zh_w3d` source list omitted that file. Add only this original provider to the existing target and ledger; verify its link-map symbol and negative null-layer behavior. Do not construct a replacement layer type.

## Verification and commit

Owned W3D mesh/simple scene and original light, Recording device with two camera viewports; assert exact source marker order, source frame begin/end, source solid and ambient, source mesh draw, object overload and layer ordering. Negative controls for null/inactive, line/point, missing edge, injected scene hook, stale attachment and retry, plus original renderer category init failure/retry. GCC/Clang focused original-source/ABI/provider tests, original-rendering CPU set, ledger, sanitizer. Record `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2b1.md`. One commit: `delivery: M22 slice 06C3C2B1 restore WW3D scene wrapper`.
