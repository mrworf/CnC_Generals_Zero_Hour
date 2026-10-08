# Linux foundation and stock renderer builds

The complete game is not yet ported after the clean reset. The renderer and
original common-service fixtures are independently testable foundations, not
GameLogic or whole-game acceptance. Run from the repository root on x86-64 Linux.

Prerequisites: Git, Python3, Make, GCC/G++, Clang/Clang++, CMake, Ninja, pkg-config,
X11/OpenGL/Wayland and Vulkan development packages, a Vulkan driver and Khronos
validation layer. SDL3 development files are required for the native-window
qualification; this host uses SDL3 3.4.16. FreeType, Fontconfig, zlib and FFmpeg
development packages are required for later original-process integration.
Native runtime cache tests additionally require stock OpenSSL3 Crypto development
headers/library (Arch `openssl`, Ubuntu `libssl-dev`, Fedora `openssl-devel`).
Only public EVP digest APIs are used; this host supplies OpenSSL3.6.4 unchanged.
No private asset is needed.

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

CTest registers 36 cases: eighteen original-core/data cases plus provenance,
initialization, nine semantic families,
a combined same-process repeat, and six capacity/lifecycle families. The latter
are `capacity`, `draw-boundary`, `uploads`, `lifetimes`, `presentation-init`, and
`presentation`. Per-family logs are in the build directory. Failures,
sanitizer diagnostics, absent enabled validation, or missing completion markers
are errors. All graphics cases are generated and asset-free. Most are offscreen;
the two presentation families open native Vulkan windows, with the full case
rendering and resizing three successive windows. A visible brief test window is
expected. Run these cases from a live supported X11/Wayland session.

The SDL video and Vulkan-loader services outlive all windows/devices and are
explicitly released at the end. Use balanced public
`SDL_Vulkan_LoadLibrary`/`SDL_Vulkan_UnloadLibrary` ownership, rather than implicit
load/unload per window. On this NVIDIA host the implicit loader lifetime caused
a 224-byte sanitizer residual even in the initialization-only control; balanced
service ownership passes both compiler sanitizer controls without suppressions.
No driver library is pinned, and no dependency source is modified.

See `docs/renderer-workload-census.md` for formulas and bounded workload claims.
Renderer qualification does not prove original-game visual parity or whole-map
capacity; integrated scene demand and original ownership remain later gates.

## Headless original core

No private assets, SDL or bgfx SDK is needed for the core-only build:

```sh
cmake -S . -B build/original-core-gcc -G Ninja -DCMAKE_BUILD_TYPE=Debug -DZH_BUILD_RENDERER=OFF
cmake --build build/original-core-gcc -j 4
ctest --test-dir build/original-core-gcc --output-on-failure
```

Use `-DZH_SANITIZE=ON` in a separate build directory, and
`-DCMAKE_CXX_COMPILER=clang++` for the second required sanitizer compiler.
The five families exercise original values/CRC, three RNG streams, strings,
explicit pools and repeated original memory-manager ownership. They do not
implement replacement gameplay. General C++ allocations use the standard
owner; only explicit class/DMA allocations use game pools. ASan, UBSan and leak
checking remain enabled. A ptrace-based sandbox prevents LeakSanitizer teardown;
run sanitizer CTest on the host, rather than disabling leak detection. These
CPU-only families require no isolated-bus fixture or graphics session.

The same headless configuration also builds rooted original data consumers and
thirteen data cases: six functional families (roots, archives, RAM, catalogs,
text, INI), six independent allocation-failure batches and the generated audit
wrapper. Each fault batch exhausts its contiguous ordinal range plus terminal
success, with three same-process repeats. These are actual core/data owners,
not complete original GameLogic initialization or simulation acceptance.

`original_data_audit` is an optional read-only supplied-data probe, not a CTest
retail dependency. Run `tools/run_data_audit.py --binary <audit-executable>` with
repeated `--data-root <supplied-root>` options in Zero Hour/Generals order. Keep
actual roots out of committed commands/logs. The wrapper retains only bounded
public stage/mask/status and integrity outcomes, hashes regular input bytes
before/after, rejects unexpected output, and never writes into supplied roots.
Its successful mask proves indexing/basic-family presence and text initialization,
not full scenario completeness. Use `evidence/qa/N2-original-data.md` for limits.

The same native configuration now also requires system zlib and OpenSSL3 development headers
and builds original namespace/FP/Dict, cached map backing and TOC/chunk readers.
`original_runtime_*`, `original_map_*` and `original_chunk_*` are generated
CPU-only supporting fixtures. All38 original-owner cases pass normal/GCC/Clang
checks at the recorded checkpoint in `evidence/qa/N2-runtime-support.md`; they
do not yet execute original GameLogic. Explicit chunk output is covered by four
additional families; EAB/EAH, rooted profiles, timestamps and UTF-16-width
filtering bring the matrix to50. XDG atomic output and content/version-keyed
conversion storage bring the cohort to60. Header/ID graph contracts bring the
cohort to62. Five native container families bring the frozen cohort to67/67
normal/GCC/Clang passes, with complete original DSP-root include casing census.
Two source-locked enum configurations bring the supporting cohort to69/69.
They verify all49 selected enum representations/declarations, not whole owners.
NOX qualification and full startup, including binding these storage owners into
GameEngine/GlobalData and whole MapCache, remain pending; supporting fixtures
do not substitute for actual original startup or simulation.

Latest committed-progress checkpoint: the generated original-runtime suite has
178 tests, with normal GCC, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan results
recorded in `evidence/qa/N2-runtime-support.md`. The earlier counts above describe
historical supporting cohorts. Native source providers, protected user storage,
save/CRC transports and replay command/session ownership are supporting evidence;
the full native entry and deterministic scenario are still N2 completion gates.
Use the existing original-core build directories and normal `cmake --build` /
`ctest --output-on-failure` workflow; no proprietary assets are embedded in tests.
