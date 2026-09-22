# M22 slice 06C3C2A evidence — original scene traversal

## Result and boundary

The canonical `SceneClass::Render` now executes its original pre-processing, fog selection, default customized render and post-processing on Linux. `SimpleSceneClass::Customized_Render` executes the authored visibility check, update list, four-slot light reset, source light-environment selection and visible-object/render-hook loop. No adapter scene or replacement object is used. The Linux-only extra polygon passes reject before callbacks because their wireframe/ZBIAS-7 physical behavior is not accepted. This slice does **not** accept `WW3D::Render(scene)`, static-sort draining, completed frame pixels, or retail scenes; C2B and later slices own those.

The owned source fixture loads `TEST.TRIANGLE` through `WW3DAssetManager`, adds it and a far-cullable object to `SimpleSceneClass`, registers a frame-update object, adds an original directional `LightClass`, and invokes the protected original `SceneClass::Render` through a test-only access subclass in a Recording pass. It verifies visible/hidden selection, render hook pre/post counts, light-environment count, selected fog and first light-slot reset. It deliberately invokes `TheDX8MeshRenderer.Flush()` only to clean and inspect the queued source mesh work, not as a completed WW3D scene claim.

Negative controls prove a missing device edge and unsupported extra pass do not invoke the hook; invalid fog does not publish a partial selected state or trace; injected hook failure restores `RenderInfoClass::light_environment` and the same render info succeeds on retry. The test checks original light refs after scene removal. Explicit original renderer shutdown before asset teardown prevents a static object-pool destructor-order UAF found by the first sanitizer run. The accepted original light-device tests retain invalid physical light coverage.

## Validation

- GCC Debug original-rendering non-GPU: 40/40 pass.
- GCC and Clang Debug focused graph, scene, camera clear, ABI and provider removal: 5/5 each pass.
- GCC and Clang ASan/UBSan/LSan focused graph, scene and camera clear: 3/3 each pass outside the ptrace-restricted sandbox. The initial GCC sanitizer run found the fixture teardown UAF; the corrected rerun is clean.
- `python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv`: pass.
- `git diff --check`: pass.

## Residual

The source wrapper entry and static/sort flush remain gated until C2B. No retail content, private path, or physical bgfx frame is used as a substitute for this source traversal result.
