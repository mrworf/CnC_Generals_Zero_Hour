# Renderer qualification workload census

Authority is the original source at baseline `0a05454d8574207440a5fb15241b98ad0b435590`.
This generated workload establishes renderer headroom for named demands. It does
not define a maximum accepted map, asset, object count or a gameplay limit.
All paths below are relative to `GeneralsMD/Code/`.

## Verified formulas and owners

| Owner | Source contract | Qualification demand |
| --- | --- | --- |
| Flat terrain | `GameEngineDevice/Source/W3DDevice/GameClient/FlatHeightMap.cpp`: CELLS_PER_TILE=16, width=(extent+14)/16 | extent 1025 → 64×64=4096 tiles |
| Terrain geometry | `.../W3DTerrainBackground.cpp::doTesselatedUpdate`: (width+1)² candidate vertices; recursive cell triangles | Fully refined grid: 289 vertices, 1536 Uint16 indices per tile |
| Terrain base texture | `.../W3DTerrainBackground.cpp`: PIXELS_PER_GRID=8; `WorldHeightMap.cpp::getFlatTexture` rounds width×pixels to power of two | 128×128 RGBA8 per tile → 256 MiB base textures |
| Terrain LOD | `.../W3DTerrainBackground.cpp::updateTexture`: 2×/4× pixels per grid, chosen by LOD | 64 additional 512×512 textures → 64 MiB; others retain base |
| Terrain passes | `.../W3DShaderManager.cpp::Init`: flat terrain with cloud/lightmap has two passes in retained two-stage path; pixel path uses one | Two full passes → 8192 draws |
| Model hierarchy and volumes | `GameEngineDevice/Include/W3DDevice/GameClient/W3DVolumetricShadow.h`: MAX_SHADOW_CASTER_MESHES=160; `GameEngine/Include/GameClient/Shadow.h`: MAX_SHADOW_LIGHTS=1 | Stress parameter 64 casters → 10240 mesh draws plus 20480 front/back volume draws |
| Volume geometry | `.../Shadow/W3DVolumetricShadow.cpp`: MAX_SHADOW_VOLUME_VERTS=16384 | One full-size generated geometry backing |
| Trees | `GameEngineDevice/Include/W3DDevice/GameClient/W3DTreeBuffer.h`: 30000 vertices, 60000 indices, 64 types, 4000 tree slots | Full-size backing; 64 type groups with two passes |
| Bridges | `.../W3DBridgeBuffer.h`: 12000 vertices, 24000 indices, 200 bridges; cpp drawBridges loops visible bridges then shroud | Full backing; 200×2 draws |
| Shorelines | `.../FlatHeightMap.cpp`: initial map array 4096 tiles, batches 512 | Eight batch draws; 4096 is an initial allocation, not a hard map limit |
| Particles | `.../W3DParticleSys.h`: 512 points/group; cpp flushes groups | Stress parameter 16 groups: 32768 quad vertices, 49152 indices, 16 draws |

The following are explicit mixed-frame stress parameters rather than source
maxima: 4096 shroud tile draws, 64 marker draws, eight water draws, two scene-copy
consumers, 128 UI draws and 64 high-LOD texture witnesses. They exercise retained
consumer categories sharing views, commands, samplers and resources with terrain.
Road counts and total particles are configured from GlobalData INI; defaults
alone do not bound them. Source-visible model/material multiplicity is likewise
not globally bounded by the terrain tile count. N3 must measure actual integrated
scene demand and rerun qualification for any larger or materially different
lowering. Never claim every possible user map fits from this generated census.

## Shared frame budgets

The mixed workload has 43826 diagnostic draw identities: 8192 terrain, 10240
models, 20480 volumes, 128 tree groups, 400 bridges, eight shoreline batches,
16 particles, 4096 shroud, 64 markers, eight water, two distortion, 128 UI and
64 LOD witnesses. All identities get distinct physical pixel witnesses. A
separate capacity-boundary case exercises the public draw limit minus one,
including ordered clear/pass/copy consumers and no overflowing submission.
Its explicit clear-only `touch` is a dummy submit consuming the final render
item. Frame admission counts draws plus touches, blits and highest view separately;
bound+1 and overflowing compound demand reject before any upstream call.

Grid layout is 32 bytes (XYZ, packed color, two UV pairs), so terrain geometry
is 4096×(289×32+1536×2)=50462720 bytes. Texture/geometry loading is chunked into
32-tile steps. Source holders live through release callbacks; advancing renderer
loading frames is independent of simulation. Persistent resources serve gameplay
frames; this is not a per-frame hundreds-of-MiB transient upload requirement.

The physical stock handle limits are 4096 textures, vertex buffers and index
buffers, shared with the renderer and other scene consumers. A direct one-handle
per terrain tile lowering cannot represent the 4096-tile example with headroom.
The fixture groups 32 tiles per vertex/index buffer and texture array: 128 handles
of each kind, with exact per-tile index ranges and layer-selected authored data.
This is a supported game-side organization using stock texture-array shaders,
not a changed framework limit. N3 can reuse this representation or another
physically validated batching scheme appropriate to real materials.

Stock memory-backed texture creation is immutable. Updated providers allocate
with null initial memory, initialize all declared layers, then update through
the public API. Release callbacks retire host source holders; physical readback
proves sampled data after update and is independent of that retirement event.

Capacity, high-resolution upload and lifecycle contexts are independent, since
the backend may retain prior frame high-water allocations. Measure settled
resource counts on the same context with unchanged preparing owners. Public
memory estimates are reported separately from exact test-owned payload bytes.
No private allocator introspection or forced shrink is permitted.

## Validation status

Source formulas and the named generated workloads passed normal-host Vulkan
plus both owner sanitizer variants. See
`evidence/qa/N1-stock-renderer-capacity-lifecycle.md` for the frozen acceptance
matrix, exact payloads, physical witnesses and limits. This accepts N1's bounded
qualification; actual original-world demands and visuals remain N3 work.
