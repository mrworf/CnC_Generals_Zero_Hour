# M28 original CPU presentation and media resource evidence

Date: 2026-09-20
Scope: M28 only; asset-free x86-64 Linux CPU presentation, UI resources, and audio definitions

## Result

M28 acceptance is satisfied at its independent-provider boundary. Three source-owned extraction units preserve named original W3D CPU/resource, Image/Font/WND, and GameAudio definition behavior and bind it to the accepted VFS, recording renderer, retained-font, UI recorder, and null-output audio interfaces. All focused consumers run without a window, physical GPU, Vulkan device, or audio endpoint. Owned synthetic fixtures suffice; no retail content, private path, retail hash, ARM64 target, browser/service integration, or complete GameClient initialization was used.

## Provider and behavior evidence

- `zh_original_resources` compiles `OriginalCpuPresentation.cpp`, `OriginalUiResources.cpp`, and `OriginalAudioDefinitions.cpp`. Compile commands, link maps, live symbols, runtime witnesses, checked extraction markers, and provider-removal linking bind the public API to these production sources.
- `CpuPresentation` registers model/texture/animation loaders, parses bounded owned resources, owns a source scene/light/assets, and emits an actual recorded draw. Its explicit capability surface supports CPU resources and recorded drawing while rejecting physical-device and web-browser operations.
- The W3D/DX8 seam distinguishes CPU initialization from device initialization/destruction. Six injected acquisition failures unwind in reverse order. Success, error, destructor, and repeated-shutdown paths restore both M28 ownership counts and `RecordingGpuDevice` resources to zero; device acquisitions remain zero.
- `UiResources` preserves `Image::parseImageCoords` normalization and rotated-width/height semantics, validates WND controls and every callback before publication, retains the selected VFS font through M8 `FontFace`, and records the layout through `UiRecorder`.
- The owned `Menus/BlankWindow.wnd` fixture has a root, button, and static-text child. Layout init/update/shutdown callbacks, window draw callbacks, the retained font source, rotated image dimensions, and child commands are observable. Callback replacement, missing callback/layout/image/font, unsupported control, malformed bounds, staged failure, and unknown invocation fail visibly.
- `AudioDefinitions` uses the source vocabulary for AudioSettings, MusicTrack, AudioEvent, and DialogEvent fields, including Filename, Volume, LoopCount, Priority, Control, and Ambient. Same-kind later layers replace earlier definitions; incompatible duplicates and malformed/range-invalid fields fail before registry publication.
- Valid owned music resolves through VFS and is decoded/scheduled by the M12 `AudioManager` in null-output mode. Unknown/non-music lookup is rejected. Missing music returns in under one second, preserves `Audio/not-present.wav` in the diagnostic, performs zero CD searches and zero modal waits, and never reports loaded state.
- Teardown stops/drains audio work before dropping definitions. Normal, repeated, and four injected-failure paths restore definition, voice, worker, and device counts to zero. No production fixture fallback or quitting-state mutation exists.
- The checked dependency ledger records source hashes and separates runtime-proven independent providers from inspected later consumers: full W3D/GameClient/window/audio registries and reset stay M20-owned; map-start `addProp`/`preloadAssets` stay M21-owned; DX8 physical initialization/drawing/destruction stays M22-owned; interactive music stays M23-owned; CRC mismatch WND stays M25-owned.

Runtime witnesses from `linux-gcc-debug`:

```text
original-resources CPU: ok providers=OriginalCpuPresentation.cpp loaders=3 assets=3 devices=0
original-resources UI: ok providers=OriginalUiResources.cpp layouts=1 windows=3 callbacks=7 devices=0
original-resources audio: ok providers=OriginalAudioDefinitions.cpp definitions=3 workers=0 devices=0 cd-search=0 modal=0
```

## Validation

Focused `original-resources` label after stabilization:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 10/10 passed |
| `linux-clang-debug` | 10/10 passed |
| `linux-gcc-release` | 10/10 passed |
| `linux-clang-release` | 10/10 passed |

Canonical full asset-free CTest, run after the final audio-table adjustment with loopback access required by the existing LAN tests:

| Preset | Result |
|---|---:|
| `linux-gcc-debug` | 92/92 passed |
| `linux-clang-debug` | 92/92 passed |
| `linux-gcc-release` | 92/92 passed |
| `linux-clang-release` | 92/92 passed |

Focused sanitizer validation used Clang Debug with `ZH_ENABLE_ASAN=ON`, `ZH_ENABLE_UBSAN=ON`, `ZH_ENABLE_GPU_TESTS=OFF`, and `ZH_ENABLE_RETAIL_TESTS=OFF`. Execution used:

```text
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
ctest --test-dir /tmp/zh-m28-sanitizers -L original-resources --output-on-failure
```

Result: 10/10 passed. Leak detection was disabled, so this is not a leak-detector claim. Explicit ownership probes instead establish zero live loaders/assets/scenes/lights, callbacks/layouts/windows/fonts, definitions/voices/workers, recording resources, and device acquisitions after normal, repeated, malformed/missing, and injected-failure teardown.

## Acceptance mapping

- Actual original CPU/UI/audio consumers in four presets: production extraction translation units, compile/link/live-symbol identity, named runtime witnesses, and 10/10 focused results in every preset.
- Source-owned resources/loaders/callbacks and visible failures: recorded loader/scene/image/layout markers, callback invocation, malformed/missing/unsupported tests, and provider-removal failure.
- Valid and missing music: owned PCM success through null output; prompt logical-path failure with zero CD/modal attempts and no fabricated success.
- Lifecycle correctness: six CPU, four UI, and four audio failure stages plus normal/repeated teardown restore exact counts.
- No hardware: capability interception, null output state, and zero device-acquisition count; focused tests were run with GPU and retail gates disabled.
- Ledger and later ownership: checked source hashes cover W3DDisplay, DX8Wrapper, image/font/WND, GameAudio tables, map-start terrain, reset BlankWindow, interactive music, and CRC mismatch layout owners.
- Regression safety: source classification remains exactly 3,293 paths, focused ASan/UBSan passes, and 92/92 canonical asset-free tests pass in every required preset.

## Deferred boundary

M20 retains complete GameClient/display/window/audio registry initialization and destruction. M21 retains map-start terrain props and full asset preloading. M22 retains physical Vulkan scene/device/pixel acceptance. M23 retains real menu controls, cinematics, and interactive media flow. M25 retains network CRC mismatch layout behavior. None of those later milestones retroactively supplies the independent CPU/resource behavior accepted here.
