# M7 slice 01: recording device and resource contracts

## Goal and observable outcome

Callers can create, use, snapshot, and destroy opaque renderer resources through `RecordingGpuDevice`. Valid streams produce stable normalized snapshots; invalid lifetimes, descriptors, bindings, upload ranges, pass nesting, and pipeline-cache growth fail with operation and label provenance.

## Scope

Implement generation-checked buffers, textures/render targets, samplers, shaders, and pipelines; CPU-backed dynamic buffer uploads; pass/draw ordering; immutable bounded pipeline reuse; normalized snapshots; and exhaustive positive/negative tests. Keep the existing descriptor validation and `GpuDevice` entry point compatible.

## Non-scope

Texture-file parsing, compressed texture decoding, DX8 state translation, real graphics submission, synchronization, and pixels.

## Dependencies and ordering

Requires the M2 renderer descriptors and interface. It is the first M7 slice and provides the recorder used by slices 02 and 03.

## Entry point and behavior

Tests construct `RecordingGpuDevice`, call the normal `GpuDevice` API, inspect `last_error()` after rejected create/destroy/use operations, and compare `snapshot()` text. Successful create allocates a typed opaque handle; destroy invalidates its generation; resource references are checked before commands are appended. Equal pipeline keys reuse a cached pipeline and preserve its original immutable descriptor. Cache capacity is fixed at construction and overflow is rejected.

## Data/state transitions

Resource slots transition `unused -> alive -> destroyed`, with generation increments on reuse. Pass state transitions `idle -> active -> idle`. Commands append only after complete validation. CPU buffer bytes change only after a valid upload. Pipeline cache grows only for unique validated keys and never beyond its configured bound.

Authorization is not applicable because this is an in-process renderer API with no privileged operation.

## Validation and recovery

Rejected operations leave command/resource state unchanged and set an actionable error including operation and relevant label. Tests cover valid ordering plus unsupported descriptors, stale/wrong/destroyed handles, duplicate destroy, null upload bytes, bad ranges, missing draw bindings, use after destroy, nested/missing passes, attachment mismatch, and cache explosion. Recovery is demonstrated by a valid operation after a rejected one.

## Expected surfaces

`include/zh/renderer/recording_device.h`, `src/renderer/recording_device.cpp`, `tests/renderer/test_recording_device.cpp`, and `CMakeLists.txt`.

## Validation commands

Build and run the focused `renderer_recording_device` test in one debug preset, then run the `renderer-contract` label. Milestone-wide four-preset validation follows slice 03.

## Acceptance and commit boundary

The recorder behavior and all listed negative cases pass without display/GPU/retail/network access. Commit the plan, implementation, tests, and build registration as `delivery: M7 slice 01 add recording GPU device`.
