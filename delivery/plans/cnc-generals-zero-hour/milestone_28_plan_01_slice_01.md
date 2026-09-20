# M28 plan 01 slice 01: CPU scene, loaders, and device lifecycle

## Goal and observable outcome

An owned scene fixture registers original-named resource loaders, creates source-owned scene/light/assets, and emits deterministic commands through `RecordingGpuDevice` without acquiring a physical device or window. Initialization, unsupported capability, partial failure, and repeated teardown paths expose exact ownership counts.

## Scope

- Add the M28 production-facing provider library and interface.
- Extract the independently executable CPU ownership/loader behavior named by `W3DDisplay` and `W3DAssetManager` sources.
- Adapt resource creation and draw recording to the accepted renderer contract.
- Model the DX8Wrapper CPU/device boundary explicitly: CPU initialization is valid without a device; device-only initialization/drawing fails visibly; shutdown is idempotent.
- Remove browser/service operations from this provider surface.

## Non-scope

Complete W3DDisplay/GameClient construction, shader/device selection, pixel output, physical GPU/window acquisition, terrain/map-start producers, and the excluded DX8 web browser.

## Dependencies and ordering

Requires accepted M27 logical data and M7 recording renderer. It establishes ownership and lifecycle primitives used by slices 02-03.

## Entry point and end-to-end behavior

The focused executable mounts owned fixture data, creates a recording adapter, initializes CPU presentation, registers bounded model/texture/animation loaders, loads fixture resources, creates a scene and light, records one pass, and tears down. A recorder snapshot and provider identities make the source producer observable.

## Data and state transitions

`uninitialized -> CPU ready -> loaders registered -> resources/scene live -> recorded -> resources released -> loaders cleared -> stopped`. Failure injection at every acquisition stage unwinds in reverse order. Duplicate names and over-capacity input fail before state mutation.

## Authorization and permissions

No authorization surface. The consumer reads only caller-owned fixture roots and performs no persistent writes.

## Validation and error handling

Positive cases cover all loader kinds, scene/light/resource ownership, deterministic command order, repeated sessions, and idempotent shutdown. Negative cases cover missing/malformed/oversized/duplicate resources, unsupported loader/capability, device-only operation while headless, and every partial-initialization stage.

## Expected implementation surfaces

`CMakeLists.txt`; `include/zh/original_resources.h`; source-owned W3D extraction unit; focused CPU tests; dependency-ledger/provenance controls introduced as needed.

## Required commands

- Configure/build the focused target in all four presets.
- Run the focused M28 CPU tests in all four presets.
- Run checked ledger/provenance/source-classification controls touched by the slice.

## Acceptance criteria

- Runtime witness names the source-owned provider and records real loader/scene work.
- No physical device or window is acquired; device-only capability fails explicitly.
- Resource/loader/callback counts return to their baseline on success, repeated shutdown, and every injected failure.
- Missing required provider or provenance marker fails the relevant identity gate.

## Commit boundary

One commit containing this plan, interface/provider, renderer/VFS adaptation, focused tests, and corresponding checked ledger/provenance changes. UI/audio behavior is excluded.

## Result

Complete. `CpuPresentation` loads original-named model, texture, and animation resources through the accepted logical VFS, owns the registered loaders/scene/light/assets, and emits an observable source marker plus a real draw through `RecordingGpuDevice`. The explicit CPU/device capability boundary rejects physical-device and browser work without weakening recorded drawing.

The focused runtime, live-symbol identity, extraction-provenance, and dependency-ledger tests pass 4/4 in GCC/Clang Debug/Release. Owned fixtures cover valid work, duplicate/missing/malformed resources, record-before-init, six acquisition failure stages, and repeated teardown. Recording resource and M28 ownership counts return exactly to zero in every path; device acquisitions remain zero. The resulting commit is recorded in the governing plan after creation.
