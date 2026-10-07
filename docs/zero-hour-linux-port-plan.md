# Zero Hour Linux port — upstream-only restart

Status: user-approved implementation plan. Product: `cnc-generals-zero-hour`.
Packet: `linux-upstream-v2`. Baseline:
`0a05454d8574207440a5fb15241b98ad0b435590`.

This document supersedes all pre-reset plans, milestones and renderer contracts.
The user's explicit approval authorizes resetting the implementation, replacing
milestones, changing Zero Hour source, and using runtime conversion/local caches.
It does not authorize dependency forks or changing intended gameplay/visuals.

## Requirements

| ID | Required outcome |
| --- | --- |
| R01 | Verified recovery of prior history and dirty work, exact baseline reset, one active implementation. |
| R02 | Every adopted dependency/tool is an unmodified upstream version; public APIs and documented configuration only. |
| R03 | Native x86-64 Linux Zero Hour; Arch Linux is primary, GCC and Clang supported. No ARM64 or legacy MSVC obligation. |
| R04 | Users provide original Generals and Zero Hour assets. Never modify or redistribute them; decode in memory and optionally cache locally. |
| R05 | Preserve intended gameplay, visuals, audio and externally meaningful formats. Original bug fixes need a reproducer and regression test. |
| R06 | Prove stock bgfx suitability on Vulkan hardware before broad renderer integration. |
| R07 | Actual original-engine Linux startup, allocator/platform support, VFS and headless simulation; a replacement toy runtime is insufficient. |
| R08 | Complete source-required world rendering: terrain, models/animation, camera/LOD, shadows, water, particles, shroud, distortion and scene transitions. |
| R09 | Original menus/UI, fonts and supplied locale, keyboard/mouse/text input, sound/music/speech and movies; preserve loading/error flows. |
| R10 | Playable campaigns/skirmish/user maps, Linux save/load and replay, repeatable simulation across supported compilers/builds. |
| R11 | Linux-to-Linux IPv4 LAN discovery, join, map transfer, synchronized play and match termination. |
| R12 | Clean reproducible builds, asset-free packaging, installation documentation and a verified dependency-upgrade procedure. |

## Architecture

SDL3 owns platform/windows/input. Stock bgfx's Vulkan backend is the candidate
renderer, using matched unmodified bgfx/bx/bimg and stock shaderc. Author game
shaders in the compiler's supported language, with standard attributes/varyings
and explicit uniform transport. Rewrite game-side interfaces where useful; do
not preserve the previous custom GLSL ABI or build a complete D3D8 emulator.

Build a focused game renderer. Use ordered views/passes, preserving transparency,
clears and source material semantics. In bgfx, copies precede draws within a view:
draw→copy→draw requires distinct ordered views. Use game-owned scene render
targets for effects sampling the prior scene, then present the completed image.
Exact rasterization/coordinate conventions, packed attributes, stencil bit masks,
authored mip ranges and independent min/mag filtering need physical tests.

Prepare large scenes incrementally while loading without advancing simulation.
Retain persistent resources and update changed ranges during gameplay. Own and
measure game resources; use supported dependency diagnostics for backend memory.
Validate inputs and cancel game-owned candidates before submission. Do not demand
reversible GPU submission, private allocator inventories, or custom reservations.
Fatal backend failure is renderer failure, not a successful or partially dropped
frame. Threaded upload-release callbacks must not call simulation or free through
the wrong allocator. Never infer GPU completion from a source-release callback.

Retain appropriate stock SDL3, miniaudio, FFmpeg, FreeType/Fontconfig and zlib
integrations, freshly implemented as needed. Standard C++17 and CMake/Ninja are
the starting toolchain; an upstream-required newer compiler standard is allowed.
The original source and repository-owned shaders are editable; supplied audio,
textures, models and other proprietary content are not.

Explicit user data roots work independently of CWD. Writes go to XDG user paths.
Cache entries are keyed by source content plus converter version, disposable and
excluded from packages. Missing/corrupt caches rebuild; unwritable cache storage
falls back to memory conversion. Do not retain private selectors or raw asset
output in tracked diagnostics. Validate file lengths and traversal boundaries.

## Delivery order

| ID | Observable outcome | Direct dependencies |
| --- | --- | --- |
| N0 | Recovery archive, clean source baseline, settled specification and replacement packet. | None |
| N1 | Pristine toolchain and stock renderer hardware suitability gate. | N0 |
| N2 | Original-engine Linux foundation and headless simulation. | N0 |
| N3 | Original world rendering with loading and sustained scene lifetimes. | N1, N2 |
| N4 | Original interactive platform, UI and media. | N1, N2 |
| N5 | Playable single-player, persistence and determinism. | N3, N4 |
| N6 | Linux LAN matches. | N5 |
| N7 | Release and dependency-upgrade maintenance. | N6 |

Default execution order is N0→N1→N2→N3→N4→N5→N6→N7. Independent graph branches
do not imply permission to skip milestones. No agents. No old packet may resume.

## N1 suitability gate

Build pristine upstream sources in a new build directory, with recorded revisions
and public interfaces. Generated fixtures must prove all of the following:

1. Ordered viewport color/depth/stencil clears preserving outside pixels.
2. Selective stencil writes for occlusion markers together with shadow rendering.
3. Authored mip ranges, base-only sampling, unequal min/mag filters, and extreme
   minification; never sample uninitialized allocation tails.
4. Scene draw/copy/distortion ordering and sampleable intermediate targets.
5. Representative material/terrain shaders, packed attributes, alpha/blending,
   projected shadows and render-to-texture.
6. Source-derived draw, resource and upload workloads without dropped draws or
   continually growing resources; include loading throughput and repeated scenes.
7. Resize, replacement, loading cancellation and clean shutdown.

Record a requirement→source→test map. Upstream demos and historical tests are
research, not acceptance. Every selected interface must be public; an internal
implementation trick is not an accepted substitute for a supported contract.
If a required behavior needs a dependency patch, private API or visual compromise,
fail N1 and stop broad integration. Reassess SDL_GPU using the named reproducer;
do not implement another bgfx fork or silently select direct Vulkan.

## Validation and completion

Use generated assets for automated tests and the actual original source for
integration. Verify positive, boundary, malformed-input and recovery behavior.
Run focused tests during implementation, GCC/Clang ASan/UBSan for changed owner
boundaries, and Vulkan validation on frozen coherent changes. Split long sweeps
without losing coverage. Track source identity for claims about original runtime.

Retail acceptance is read-only with asset-integrity checks and redacted output.
Representative campaigns/skirmishes must reach match completion, save/load and
replay successfully; two local Linux processes must complete synchronized LAN.
Clean-build environments additionally cover Ubuntu/Fedora dependency portability;
do not require private Windows fixtures for release acceptance.

Upgrades change upstream pins, rebuild dependencies/shaders, and rerun N1 plus
original-game smoke coverage. Integration adaptations belong in this repository.
No proprietary data/derived cache is packaged. Missing packages, assets or GPU
capabilities must produce actionable diagnostics rather than apparent success.

## Exclusions and delegated decisions

No original Generals executable, WorldBuilder/content tools, DRM/CD checks,
GameSpy/Internet service replacement, remaster, ARM64, or preservation of VC6
projects/internal ABI. Windows save/replay import and mixed Windows/Linux LAN
remain optional future work. Debug-only unexercised source features must be
classified from source, not silently promoted into first-release scope.

Engine-internal refactoring, resource organization and equivalent rendering
techniques are delegated implementation choices. They cannot change gameplay,
source-required visible effects, external encodings, or the upstream-only policy.
