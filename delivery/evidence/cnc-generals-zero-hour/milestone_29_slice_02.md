# M29 slice 02 — source-facing clear and inventory evidence

`OriginalGpuEdge` now retains the last successfully translated source camera viewport only within the active frame, attaches the current frame-target generation and translates selected clear flags/values into the ordered device command. The owned CPU fixture records a full-target frame clear, draw, camera-scoped C/D/S clear, draw, and successful end in exact order; inactive/end/no-camera and invalid stencil input fail before a clear. This does **not** connect full original `WW3D::Render` or accept a retail scene; those remain M22 after M30.

The renderer mapping's target/lifecycle categories now identify `ViewportClearDesc` and the selected bgfx device edge. The [migration ledger](../../../docs/renderer/bgfx-migration-ledger.md) records all 13 categories, preserved CPU evidence and M30 physical obligations. The source dependency ledger was refreshed for the changed adapter source without changing retail data.

Validation on asset-free GCC Debug:

```text
cmake --build build/linux-gcc-debug --target original_w3d_gpu_edge_tests original_w3d_cpu_graph_tests ui_renderer_tests world_recorder_tests effects_recorder_tests -j 4  PASS
ctest --test-dir build/linux-gcc-debug --output-on-failure -R '^(original_w3d_gpu_edge_failure|original_w3d_source_frame|original_w3d_camera_apply|renderer_inventory_negative|renderer_public_headers|ui_renderer|world_recorder|effects_recorder)$'  8/8 PASS
python3 tools/renderer_inventory.py --check                       504 IDs / 13 rules PASS
python3 tools/check_original_dependency_ledger.py --root . --ledger docs/original-runtime-dependency-ledger.tsv  PASS
python3 -m unittest tests.m2.test_renderer_inventory tests.m2.test_renderer_public_headers  4/4 PASS
git diff --check                                                    PASS
```

Evidence grade: source-facing adapter and affected UI/world/effects commands are CPU runtime; bgfx physical mapping is source-inspected only. Prior SDL_GPU tests are historical preservation evidence, not accepted bgfx hardware proof.
