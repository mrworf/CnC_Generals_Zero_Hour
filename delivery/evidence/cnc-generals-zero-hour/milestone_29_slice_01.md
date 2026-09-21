# M29 slice 01 — ordered viewport-clear recorder evidence

The public renderer contract now exposes `ViewportClearDesc` and a fail-closed `GpuDevice::clear_viewport`. The recorder accepts a target-generation-matched active-pass clear, clips a partially overlapping rectangle, records independent color/depth/stencil flags and values, and preserves draw/clear/draw order. Invalid calls append errors only and no clear command. Full-target `begin_pass` clear/load and successful-present initialization behavior remain unchanged. SDL_GPU does not claim support for the new operation; M30 owns physical bgfx submission.

Validation on the asset-free GCC Debug preset:

```text
cmake --preset linux-gcc-debug                                               PASS
cmake --build build/linux-gcc-debug --target renderer_recording_device_tests renderer_contract_tests -j 4  PASS
ctest --test-dir build/linux-gcc-debug --output-on-failure -R '^(renderer_contract|renderer_recording_device|renderer_public_headers)$'  3/3 PASS
git diff --check                                                              PASS
```

Positive cases include selected C/D/S and color-only clear, clipped extent and ordered interleaved draws; negative cases include inactive pass, no flags, empty/disjoint rectangle, stale target generation/handle, nonfinite/out-of-range values, stencil on depth-only format and ordered-view exhaustion. This is CPU contract evidence, not hardware pixels or original retail-scene acceptance.
