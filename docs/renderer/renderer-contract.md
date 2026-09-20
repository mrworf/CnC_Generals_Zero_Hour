# Renderer closure contract

M2 closes the renderer at the source/API boundary without creating a window or graphics device. `docs/renderer/legacy-api-mapping.tsv` classifies every renderer identifier observed in the in-scope legacy C/C++ source. `tools/renderer_inventory.py --check` is fail-closed: an unclassified or multiply classified identifier breaks the `renderer-contract` test label.

Engine-facing code uses the opaque handles and descriptors in `include/zh/renderer`. The internal `GpuDevice` interface is the sole submission boundary shared by the later recording and real implementations. Public headers contain no platform graphics types. Pipeline identity includes shaders, vertex layout, primitive topology, blend/color mask, depth/stencil, raster state, target formats, point-size behavior, premultiplied-alpha behavior, and fog. Keys are immutable after construction and support bounded cache lookup.

The contract permits at most four uniform buffers per shader stage. Blocks are explicitly 16-byte aligned and separated into frame, material, object, and effect data. Texture, sampler, render-target, upload, point-list, and pass limits fail before backend submission. Two-dimensional, cube, and three-dimensional textures are explicit. BC1/BC2/BC3 use the backend format only after its public support query; otherwise the caller decodes to RGBA8 on the CPU. BC2/BC3 mappings preserve the premultiplied-alpha distinction inherited from DXT2/4.

Coordinates are left-handed, depth is 0 through 1, clockwise vertices are front-facing, texture origin is top-left, UI positions retain the negative half-pixel offset, and packed colors are ARGB8. Fog distance is view-space. Resize is an explicit `stable -> requested -> recreating -> stable` transition; zero extents suspend presentation, failed recreation remains pending, and successful recreation advances a generation.

The checked shader registry contains UI, terrain, water, points/particles, and WWShade families. Each has explicit vertex and fragment GLSL modules compiled offline to SPIR-V. The points family writes point size; water covers projected render-target sampling; WWShade demonstrates multipass texture/projected/fog state within the four-uniform limit.

SDL_GPU is the provisional implementation backend. M2 does not claim device support, synchronization correctness, pixels, or performance. M14 owns those claims on the local Vulkan-capable x86-64 system. A demonstrated abstraction gap follows the fallback decision in the authoritative port plan; no private Vulkan escape hatch is permitted.
