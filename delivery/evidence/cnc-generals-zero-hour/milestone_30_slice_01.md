# M30 slice 01 — pinned runtime/resource evidence

Status: focused slice evidence only; not M30 renderer acceptance.

- Transaction start: `491830444463a61ba824e7194675ba7d838a4b43`, clean worktree.
- Public source pins: bgfx `81d81fba72c42d348c589514c774bbfe01e110fa`, bx `25315498841259323e18f549d1ad9d9aba6632cc`, bimg `87aaad3ac882e741889fdd4263224e5d12c26f99`; reviewed common BSD-2-Clause license digest in `third_party/bgfx_shaderc.lock`.
- `bash tools/renderer/bootstrap_bgfx_runtime.sh`: passed offline against the pinned local cache, produced `libbgfx-shared-libRelease.so`; no repository vendor copy or network fetch.
- `cmake --preset linux-gcc-debug` and `cmake --build --preset linux-gcc-debug --target renderer_bgfx_device_tests`: passed.
- `ctest --test-dir build/linux-gcc-debug -R renderer_bgfx_device_contract --output-on-failure`: 1/1 passed (non-Vulkan and empty shader-root rejects).
- Related GCC Debug `renderer_contract`, `renderer_recording_device`, `renderer_texture_loader`, `renderer_bgfx_device_contract`: 4/4 passed. Clang Debug configure, focused build and contract test: passed.
- `build/linux-gcc-debug/renderer_bgfx_device_tests --gpu`: sandbox result was public bgfx Vulkan initialization failure because the NVIDIA ICD was unavailable there; identical test with host GPU access passed, including two init/shutdown cycles, physical RGBA8 texture allocate/upload/destroy/recreate, and positive/negative buffer and stale-handle checks. This does not exercise rendering or the Vulkan validation layer.
- Missing/wrong-cache negative configure: `cmake -S . -B /tmp/zh-m30-missing-bgfx -DZH_BGFX_SOURCE_ROOT=/tmp/zh-m30-no-such-source -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++` failed at the intended pinned-source check without touching supplied sources.

Buffer and sampler handles deliberately remain typed CPU descriptors until slice 03's draw/bind boundary supplies missing layout/index width. Ordered clears, shader programs, presentation and full asset-free/GPU acceptance remain pending. The original retail symlink was neither read nor modified in this slice.
