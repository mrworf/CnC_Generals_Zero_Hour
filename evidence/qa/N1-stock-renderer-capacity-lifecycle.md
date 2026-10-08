# N1 stock-renderer capacity and lifecycle acceptance

Executed 2026-10-07, following semantics slice `22c25e7be6c7613a119d8a2becec6da71b1c25d4`.
This report belongs to the commit introducing it; no self-referential SHA is used.
Authority: `docs/zero-hour-linux-port-plan.md`, seven N1 suitability gates.

## Executed configuration

Pristine official bgfx `cca91681c953d2de9531197b0f580c866ffaa775`, bx
`d86e4ea9d9da6e832a3ff41398587d82b772c69b`, bimg
`101b5b5fd4670f82cfdec8e98aa1ab9ee93bb2a1`. Stock GCC Debug shared runtime
and shaderc, public headers/APIs only. `tools/upstream_bgfx.py verify` passed
after qualification. Original source remains the reset baseline; no retail data
was opened. SDL3 3.4.16 owns native Wayland windows.

RTX4070 (vendor10de/device2786), NVIDIA615.71.09, Vulkan, enabled Khronos
validation. Owner builds: GCC16.2.1 and Clang22.1.8 ASan/UBSan; these instrument
repository fixtures, not the stock dependency or original engine memory pools.
Sanitizer variants use the approved isolated-bus fixture, detect_leaks=1,
halt_on_error=1, no suppressions. Normal-host functionality does not isolate buses.

Commands are the complete configure/build/CTest matrix in `docs/build.md`.
Frozen final CTest results:

| Build directory | Result | Elapsed | LastTest.log SHA256 |
| --- | --- | --- | --- |
| build/qualification-gcc | 18/18 | 9.90s | bdd5ea786452082547ee93eb5dfa563c383e9a9fca599821124530ff1038e53e |
| build/qualification-gcc-sanitize | 18/18 | 51.27s | 6cf58148f7b1fb57cf36266ae73a13ce9919edd27bd73b6451c73d04ef9b86e8 |
| build/qualification-clang-sanitize | 18/18 | 48.27s | 237dcfd9428b9c38f8417fe60e1d00ad0d8e8814a57bccb2ded6e85b97a881ad |

Complete generated-input logs remain in each ignored build directory under
`Testing/Temporary/LastTest.log` and named family logs. All physical families
produced terminal completion and active validation evidence; no sanitizer,
Vulkan validation, assertion or missing-output diagnostics remain.

## Requirement → source → physical test

| Gate | Original source / durable source map | Current physical evidence |
| --- | --- | --- |
| 1 viewport clears | dx8wrapper viewport/clear consumers; original-engine-formats.md | renderer_clears; boundary final scoped clear preserves captured scene/tail |
| 2 selective stencil + shadows | W3DScene.cpp, W3DVolumetricShadow.cpp | renderer_stencil, combined same-process renderer_all |
| 3 authored mips/filtering | texturefilter.cpp, textureloader.cpp | renderer_mips, renderer_filters: authored ranges, base-only, min/mag, extreme minification |
| 4 draw/copy/distortion order | W3DSmudge.cpp, ordered scene-target contract | renderer_ordering, renderer_draw-boundary: draw→copy→clear, preserved distinct witnesses |
| 5 materials/attributes/projection | terrain.nvp, fterrain.nvp, matrixmapper.cpp, texproject.cpp | renderer_materials, renderer_projection, renderer_formats |
| 6 cardinality/resources/uploads | renderer-workload-census.md with native terrain/tree/bridge/shadow/particle source paths | renderer_capacity, renderer_draw-boundary, renderer_uploads |
| 7 resize/cancel/replace/shutdown | original source owner lifetime constraints; public SDL/bgfx services | renderer_lifetimes, renderer_presentation-init, renderer_presentation |

Gates 1–5 retain detailed source equations and negative controls in
`N1-stock-renderer-semantics.md`; the final full matrix reran those semantics
with the current common support and shader programs.

Capacity: three complete generations of 4096 terrain tiles, 128 grouped texture
arrays and vertex/index owners each. Each generation loads 318,898,176 exact
host payload bytes, then physically verifies 43,826 distinct mixed draw witnesses
across 13 consumer groups, including every terrain layer. All 13,056 upload
holders released; same-owner public resource counts return exactly to baseline.
Settled texture estimate is 49,152 bytes and GPU estimate 266,403,840 bytes for
all three generations: estimates are diagnostics, not private allocator proofs.
Normal generated load/render/readback/teardown took 2.556s for 956,694,528 bytes
total; this is not a pure transfer-bandwidth benchmark or a game loading promise.

Independent draw boundary: 65,534 visible draws plus one clear-only touch equals
the public 65,535 render-item limit. Every visible identity and untouched tail
is read back; the exact requested peak is corroborating, not sole evidence.
Compound count overflow, bound+1 blits and out-of-range views reject before
upstream calls. Upload family initializes 64 high-LOD textures and source-sized
geometry, 70,357,564 bytes / 72 holders, with tail selection pixels.

Lifecycle: six accepted scene generations, partial candidate faults at 0/1/31,
canceled preparation, zero/bound+1/UINT32_MAX rejection, stale/out-of-range
tokens, changed texture layers and dynamic vertex/index backing, replacement
and corrected retry. All 976 holders release and resource baselines recover.
Warm native dynamic backing is established before the repeated-owner baseline.
Three native contexts each resize through 160×120, 321×241, 192×128, 160×120;
swap-chain dimensions and offscreen pixels are checked after every resize.

## Coupled findings and limits

Direct per-tile handles exhausted shared stock tables. Game-owned grouped
buffers/texture arrays preserve each tile's data, not a modified upstream limit.
Mutable textures use null-memory creation followed by initialization/update;
memory-backed creation is immutable. Explicit BGRA8/D24S8 is required for the
new public SwapChain path. Touches consume render-item capacity.

An initial native-window fixture reported 224 leaked bytes under both sanitizers,
also reproduced in initialization-only contexts. One SDL video owner alone did
not fix it. Loader diagnostics placed return addresses in unloaded NVIDIA
mappings. Balanced public SDL_Vulkan_LoadLibrary/UnloadLibrary ownership spanning
all windows resolves both controls and full families, with final unload and
SDL_Quit, leak checking retained, no driver pin or dependency edits.

N1 accepts stock bgfx for the named qualification gates, not whole-game parity.
Source-configured model/particle/road counts are not universally bounded. N3
must measure integrated demand and requalify larger or different lowering.
Original process allocation, retail decode, intended effects/geometry, input,
media, gameplay and distribution portability remain N2–N7, not accepted here.

## Frozen source identities

- lifecycle.cpp: 4cb209df7729b212a8b37ee47033d718874e1cf11228863384b6e30d50e0fa54
- support.h: 23bc91b3391df864ea825d35fdeedc62d6f3058a86b7e68e0d43e3f7467d2757
- workload.h: 58e8a1eaea8a3c25952aeaee10ce45a09701487af84680506ac5e85ecd8ebd50
- qualification.cpp: 8449f4b89323a465154050513809eb046a774fa99a7a8669eb31aa21cd093669
- fs_array.sc: f8fc5feb5675639a6e53980e3fb2684d6fa3356a6fe68b05191e8bf40477b7cc
- CMakeLists.txt: 922f183c4631243beb7dfcf9df7d0307a22ef54c6de17a9a4f27bed5afd0b010
