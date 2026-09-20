# M14 SDL_GPU hardware acceptance evidence

## Disposition

**Accepted on the primary x86-64 Vulkan GPU.** The SDL_GPU Vulkan implementation and project-owned hardware scene matrix pass with `VK_LAYER_KHRONOS_validation` enabled. Both the generated GPU suite and the retail+GPU suite completed with zero `Validation Error` and zero VUID output, so PRE-013 is complete for the required single-GPU gate.

No public SDL_GPU capability gap was observed. The section 9 backend fallback is not triggered; SDL_GPU remains the selected backend.

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
| Validation layer | `VK_LAYER_KHRONOS_validation`, Arch package `vulkan-validation-layers` 1.4.357.0-1 |

The host `vulkaninfo --summary` probe found the RTX and Khronos validation layer, and `pacman -Q vulkan-validation-layers` reported version 1.4.357.0-1. Hardware commands explicitly set `VK_INSTANCE_LAYERS=VK_LAYER_KHRONOS_validation`. The GPU executable runs through a repository wrapper that combines stdout/stderr and fails even when the executable exits zero if it observes `Validation Error` or `VUID-`.

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

The two complete validation-enabled 16-scene presentation loops each took 97 ms in the recorded run, below the 10-second functional threshold. These timings are functional metrics, not a performance target.

## Validation findings and disposition

The first enabled run exposed two locally recoverable errors and was not accepted:

- `VUID-vkCmdDraw-viewType-07752`: the shared effects texture slot placed a cube view in WWShade's 2D `projected_texture` binding. Effects now retain ordinary 2D slots and bind a dedicated cube only to the bump-environment cube sampler.
- `VUID-vkCmdDrawIndexed-imageLayout-00344`: the shadow caster sampled `world shadow map` while that same texture was its active color attachment. The shadow dependency pass now binds a neutral 2D fallback; main-world draws continue to sample the completed shadow map.

Focused command-stream tests pin both corrected bindings. The fail-closed output wrapper also has positive and negative unit coverage. The complete generated and retail-backed validation-enabled reruns emitted neither finding nor any other validation error.

## Retail and cross-build evidence

- The retail-enabled `ui|video` selection passed 7/7, including bounded decode of all project-manifest Bink variants through the read-only user-owned corpus.
- All four canonical GCC/Clang debug/release presets built, and their default `gpu|ui|video` selections passed. Default presets intentionally contain no hardware test because `ZH_ENABLE_GPU_TESTS` remains off.
- The explicit validation-enabled GPU build passed 2/2 GPU tests. The validation-enabled retail+GPU `gpu|ui|video` selection passed 9/9, including bounded decode of the read-only retail movie corpus.
- No retail bytes, digests, physical filenames, or private absolute paths are present in this record or the project-owned output.

## Acceptance result

The generated GPU suite passed 2/2 and the retail+GPU suite passed 9/9 with the Khronos layer explicitly enabled. Their combined output contained zero `Validation Error` and zero `VUID-`. Together with the scene, lifecycle, semantic, retail, and functional timing evidence above, this satisfies the M14 PRE-013 single-GPU acceptance gate.
