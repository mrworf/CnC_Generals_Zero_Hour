# M22 plan 01 slice 06C3C2A: original scene fog, light and object traversal

## Goal and outcome

Requires accepted C1. A canonical original `SceneClass::Render` call invokes source pre-processing, fog selection, `Customized_Render`, and post-processing; a `SimpleSceneClass` executes authored visibility, frame-update, first-four-light environment, render-hook and visible-object traversal in original order. A recording witness reaches real source mesh objects and records/rejects source state without a completed WW3D frame claim. C2B owns the wrapper/Flush and pixels.

## Scope, state and boundaries

Remove only the two Linux typed guards reached on the default scene path. Preserve the original source methods as single native/CPU bodies; use narrow CPU implementations only for already accepted `DX8Wrapper::Set_Fog` and `Set_Light`/`Set_Light_Environment` device edges. Preserve original source visibility lists/refcounts, `On_Frame_Update`, light ordering and `rinfo.light_environment` ownership. Do not invent an adapter scene, object, light, material or scheduler. Extra-pass line/clear-line and wireframe7 are not accepted by this slice unless the accepted physical state profile proves them; reject them explicitly before source publication and record retail reachability for C4/08. No file/network authorization; owned fixtures only and retail roots untouched.

## Failure and recovery

Unsupported extra-pass state, invalid fog, missing GPU edge, and injected hook failure must reject without silently skipping required objects or leaving modified source selected state, hook publication or frame ownership. Restore source light-environment selection as required for retry; no successful frame marker is emitted here. Invalid physical light selection remains covered by the accepted original light-device tests, while object validity and frame rollback are owned by C2B's caller boundary. Investigate exact refcount and post-processing order before changing source bodies.

## Surfaces, tests and commit boundary

Expected surfaces: canonical `scene.cpp` and narrowly reached original fog source, owned `tests/original_rendering/test_w3d_cpu_graph.cpp`, CMake test registration, dependency ledger and evidence. Positive: original asset-manager mesh and source light in `SimpleSceneClass`, visible versus hidden, source frame-update registration and light environment, original render-hook order and a direct source `SceneClass::Render` recording trace. Negative: unsupported extra pass, missing edge, invalid fog, injected hook error and same-render-info retry with bounded refs. Run GCC/Clang focused original-rendering and renderer tests, ABI/provider removal, dependency ledger, relevant sanitizers. C2B later supplies complete Recording/bgfx pixels and full suites. Evidence: `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_06c3c2a.md`. One commit: `delivery: M22 slice 06C3C2A restore original scene traversal`.
