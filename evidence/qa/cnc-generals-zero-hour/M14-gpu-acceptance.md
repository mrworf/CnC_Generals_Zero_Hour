# M14 SDL_GPU hardware acceptance evidence

## Disposition

**Blocked on external PRE-012 validation-layer installation.** The SDL_GPU Vulkan implementation and project-owned hardware scene matrix pass on the primary GPU. PRE-013 is not complete because `VK_LAYER_KHRONOS_validation` is not discoverable, so this record does not claim a validation-error-free run or final visual acceptance.

No public SDL_GPU capability gap was observed. The section 9 backend fallback is not triggered; SDL_GPU remains the selected provisional backend pending the validation-layer rerun.

## Hardware and runtime context

| Item | Observed value |
|---|---|
| GPU | NVIDIA GeForce RTX 4070 |
| Vulkan API | 1.4.341 device; 1.4.357 loader |
| Driver | NVIDIA 610.57.04 (`610.57.4.0` reported by SDL) |
| SDL | 3.4.14 |
| SDL_GPU backend | `vulkan` |
| Texture capabilities | BC1, BC2, and BC3 supported |
| Architecture | Linux x86-64 |
| Validation layer | `VK_LAYER_KHRONOS_validation` absent |

The host `vulkaninfo --summary` probe found the RTX and six NVIDIA/Steam layers, but not the Khronos validation layer. `pacman -Q vulkan-validation-layers` also reported the package absent. The repository gate fails closed with the installation command context rather than treating SDL debug mode alone as validation coverage.

## Exercised backend paths

- Public SDL_GPU resource creation/destruction for buffers, 2D/cube/render/depth textures, samplers, SPIR-V shaders, and cached graphics pipelines.
- Transfer-buffer uploads for dynamic vertex/index data and decoded RGBA movie textures; CPU shadows feed SDL_GPU uniform pushes.
- Render-pass clear/store, viewport, vertex/index/sampler/uniform binding, indexed and non-indexed draws, debug groups/labels, offscreen targets, swapchain acquisition, blit, submission, and presentation.
- Window claim/release, resize, focus hide/show, fullscreen enter/leave, idle wait, complete teardown, and a second device creation/presentation generation.
- Negative boundaries for null/duplicate window claim, stale presentation source, missing/nested pass, invalid texture upload, stale handles, unsupported triangle fan without CPU expansion, and backend/shader-root option rejection.

## Scene and semantic matrix

The hardware executable presents project-owned generated pixels only. It runs each row twice, once per device generation, and additionally drives the actual M8 UI recorder, M9 world recorder, M10 effects recorder, and M13 generated-movie player through `SdlGpuDevice`.

| Coverage | Result |
|---|---|
| UI, English font sample, cursor/layering | Pass |
| Terrain, mesh/animated mesh, roads/decals, trees, selection marker | Pass |
| USA, China, and GLA faction geometry | Pass |
| Water/reflection, particles, projected texture, bump environment | Pass |
| Shadows, stealth, post effect, WWShade multipass | Pass |
| Generated RGBA movie upload, aspect-fit draw, focus/EOS return | Pass |
| Resize, focus, fullscreen, teardown/recreation | Pass |
| Left-handed coordinates, 0..1 depth, clockwise front face | Command/pipeline mapping exercised |
| Half-pixel UI, ARGB8 color, premultiplied alpha | UI/effect paths exercised |
| View-space fog, explicit depth bias, top-left texture origin | World/effect paths exercised |
| Video frame/audio-clock ordering | Headless video suite plus GPU movie presentation pass |

The two complete 16-scene presentation loops took 696 ms and 681 ms in the recorded run, below the 10-second functional threshold. These timings are functional metrics, not a performance target.

## Retail and cross-build evidence

- The retail-enabled `ui|video` selection passed 7/7, including bounded decode of all project-manifest Bink variants through the read-only user-owned corpus.
- All four canonical GCC/Clang debug/release presets built, and their default `gpu|ui|video` selections passed. Default presets intentionally contain no hardware test because `ZH_ENABLE_GPU_TESTS` remains off.
- The explicit GPU build passed `renderer_gpu_acceptance`; the complete `gpu` selection then failed only `renderer_gpu_validation_layer` with the absent-layer diagnostic.
- No retail bytes, digests, physical filenames, or private absolute paths are present in this record or the project-owned output.

## Required rerun

Install Arch package `vulkan-validation-layers`, confirm `vulkaninfo --summary` lists `VK_LAYER_KHRONOS_validation`, then rerun the explicit GPU and retail+GPU commands in `docs/building-linux.md`. M14 may be accepted only if both GPU tests pass and the enabled Khronos validation stream reports zero errors for the complete scene/lifecycle matrix.
