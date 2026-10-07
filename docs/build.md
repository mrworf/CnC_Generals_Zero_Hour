# Stock renderer qualification build

The game itself is not yet ported after the clean reset. These commands qualify
the renderer/toolchain only. Run from the repository root on x86-64 Linux.

Prerequisites: Git, Python3, Make, GCC/G++, Clang/Clang++, CMake, Ninja, pkg-config,
X11/OpenGL/Wayland and Vulkan development packages, a Vulkan driver and Khronos
validation layer. SDL3, FreeType, Fontconfig, zlib and FFmpeg development packages
are required for later original-process integration. No private asset is needed.

```sh
python3 tools/upstream_bgfx.py acquire
python3 tools/upstream_bgfx.py verify
python3 tools/upstream_bgfx.py build --compiler gcc --configuration debug64 --jobs 4
python3 tests/toolchain/test_upstream_bgfx.py
```

All dependency sources and outputs live in ignored `build/upstream/`. Acquisition
uses exact official commit IDs from `third_party/bgfx.lock.json`; it never patches
or resets an existing checkout. A wrong/dirty checkout must be investigated or
replaced by a fresh build root. Do not link anything from `.port-recovery/`.

The upstream runtime output is under
`build/upstream/bgfx/.build/linux64_gcc/bin/`; shaderc is built from the same source
revision. Only supported upstream make targets/configuration are used.
The bootstrap verifies all three source trees before and after building.

GPU/network checks may need the host execution context: this environment's
sandbox hides its NVIDIA ICD and DNS. `vulkaninfo --summary` on the host verifies
the real GPU/layer. Do not mistake a sandbox failure for a missing system driver.

```sh
cmake -S . -B build/qualification-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/qualification-gcc -j 4
ctest --test-dir build/qualification-gcc --output-on-failure

cmake -S . -B build/qualification-gcc-sanitize -G Ninja -DCMAKE_BUILD_TYPE=Debug -DZH_SANITIZE=ON -DZH_ISOLATED_BUS=ON
cmake --build build/qualification-gcc-sanitize -j 4
ctest --test-dir build/qualification-gcc-sanitize --output-on-failure

cmake -S . -B build/qualification-clang-sanitize -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Debug -DZH_SANITIZE=ON -DZH_ISOLATED_BUS=ON
cmake --build build/qualification-clang-sanitize -j 4
ctest --test-dir build/qualification-clang-sanitize --output-on-failure
```

These compiler variants instrument repository-owned test code and link the same
stock GCC Debug runtime; they do not claim instrumented vendor internals or the
original allocator. The explicit isolated-bus test option affects only child test
processes. Fresh normal-host init-only and full controls reproduced the same
1,854-byte D-Bus leak; isolated controls and families pass with all leak checks
enabled. There are no suppressions or library edits. The normal functional build
leaves the host bus environment unchanged. See the evidence report for limits.

CTest registers initialization, nine independent semantic families and a combined
same-process repeat. Per-family logs are in the build directory. Failures,
sanitizer diagnostics, absent enabled validation, or missing completion markers
are errors. All graphics cases are generated, offscreen and asset-free.
Neither these semantics checks nor successful bootstrap alone accepts N1 or the
game: source-derived capacity and scene lifecycle qualification is still required.
