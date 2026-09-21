# Command & Conquer: Generals – Zero Hour native Linux port plan

Status: implementation-ready architecture and delivery plan. The current source tree does not build on Linux.

Target: the Zero Hour game in `GeneralsMD/`. The original `Generals/` executable, content-authoring tools, and media redistribution are not part of this port.

## 1. Scope and completion criteria

The release objective is a native x86-64 Linux build of Zero Hour that uses retail Zero Hour and base Generals data supplied by the user. Arch Linux x86-64 is the primary development and playable acceptance environment. Ubuntu 26.04 LTS and Fedora 44 remain clean-build portability environments. Other architectures are not supported. The repository must not download, modify, convert, or redistribute retail data.

A workable first release must:

- start from any working directory and use explicit Zero Hour and Generals data roots;
- validate the supplied data and report missing or unsupported content precisely;
- reach the main menu and support campaigns, skirmish, user maps, saves, replays, loading screens, movies, music, speech, sound effects, keyboard, mouse, and text entry;
- render the effects exercised by the supplied retail corpus, not only a menu or simple test scene;
- fully support and test the retail locale supplied for acceptance;
- preserve retail asset and CSF encodings and source-established save/replay fields through explicit fixed-width codecs;
- preserve deterministic simulation closely enough for repeatable replay CRCs across supported Linux builds;
- support Linux-to-Linux IPv4 LAN discovery, joining, map transfer, synchronization, and match completion; and
- build with both GCC and Clang, with port tests passing under AddressSanitizer and UndefinedBehaviorSanitizer.

Mixed Windows/Linux LAN is a later compatibility target, not a first-release criterion. The port should nevertheless preserve the established packet encoding wherever it can be proved so that it does not create an unnecessary protocol fork.

Windows 1.04 save and replay import is also a later compatibility target. Source analysis must guide portable field widths from the start, but release milestones cannot depend on private Windows fixtures that may never be available. Linux save/load, Linux replay round trips, and Linux determinism are mandatory. Actual Windows fixture validation is isolated in optional milestone M18 and cannot block M0-M17.

The following are explicitly out of scope:

- `Generals/`, WorldBuilder, exporters, 3DS Max plug-ins, GUIEdit, ParticleEditor, launchers, patchers, benchmark utilities, match bots, asset packers, and other tools;
- SafeDisc, CD checks, launcher handshakes, serial-number storage, or other DRM behavior;
- GameSpy accounts, Internet lobbies, matchmaking, statistics, patching, or a replacement online service;
- an embedded browser, Windows Media playback, frame capture, or video recording;
- controllers and controller-specific UI, IPv6, a mod manager, asset conversion, remastered graphics, or unrelated engine refactoring;
- non-x86-64 targets, including ARM64; and
- packaging formats such as Flatpak or AppImage until the native build and runtime dependency contract are stable.

The first successful compile or link is not acceptance. It is an intermediate milestone.

## 2. Architecture assessment

### What is good and should be preserved

- The game already separates much of its platform implementation behind factories and interfaces. `CreateGameEngine()` constructs the Win32 implementation (`GeneralsMD/Code/Main/WinMain.cpp:1104-1116`), while device-specific implementations live below `GameEngineDevice`. These are useful replacement seams.
- BIG parsing, INI and CSF loading, W3D loading, save/load, RefPack, simulation, terrain, and most UI behavior are engine-owned. They need data-model and safety fixes, not wholesale replacement.
- `DX8Wrapper` was intended to centralize Direct3D access (`GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.h:169-213`). Direct device access has leaked past it, but it remains the correct front-end boundary for a new renderer.
- The UDP wrapper already contains partial POSIX conditional code (`GeneralsMD/Code/GameEngine/Include/GameNetwork/udp.h:31-52`). The LAN protocol can be retained rather than replaced with a networking framework.
- The embedded browser is already omitted during normal engine initialization (`GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp:523-538`). Removing its dead UI paths is safer than inventing a replacement.
- Granny is conditional on `INCLUDE_GRANNY_IN_BUILD` (`GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DGranny.cpp:44-57`), and the normal Zero Hour project does not enable it. It should stay disabled unless retail data proves otherwise.

### What is risky or blocking

- The release project links D3D8/D3DX8, DirectInput, DirectSound, Miles, Bink, Winsock, GDI, Win32 UI, WinINet, and internal libraries (`GeneralsMD/Code/RTS.dsp:35-60`). This is a platform and renderer port, not a compiler-option exercise.
- Direct3D usage is broad. The source contains hundreds of direct device references and dozens of render states, texture-stage states, FVF declarations, and formats. Some calls bypass `DX8Wrapper`. A thin draw-call shim would compile but would not render the game correctly.
- Terrain, water, trees, bump effects, and other paths include legacy shader assembly (`.vsh`, `.psh`, `.nvp`, and `.nvv`). D3DX/NVASM runtime assembly has no portable modern equivalent; these shaders must be translated by behavior.
- The legacy solution includes a separate `wwshade` project (`GeneralsMD/Code/RTS.dsw:135`), and the source contains runtime shader-management code. Omitting it from the Linux source closure would create late link or rendering gaps.
- The Win32 entry point changes the working directory, parses mutable command-line storage, creates windows and splash UI, installs SEH handlers, uses a named mutex, and performs launcher/copy-protection work before `GameMain` (`GeneralsMD/Code/Main/WinMain.cpp:871-1072`). These concerns must be separated rather than transliterated.
- The source assumes the Win32 data model. `wchar_t` and `long` are 16 and 32 bits respectively on the original platform but normally 32 and 64 bits on x86-64 Linux. Save and packet code contains `sizeof(WideChar)` and raw-memory serialization (`XferSave.cpp:317-336`, `NetPacket.cpp:325-345`).
- Determinism depends on more than disabling fast math. `setFPMode()` resets x87 state, selects round-to-nearest, and selects 24-bit precision (`GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp:198-218`); it is called during INI processing and simulation. `BaseType.h` contains x87 and bit-level float-to-integer helpers (`GeneralsMD/Code/Libraries/Include/Lib/BaseType.h:177-248`). Native x86-64 code normally uses SSE, so the old behavior cannot be recreated by a single compiler flag.
- The packet layer performs raw `memcpy` using host sizes for booleans, enums, structures, coordinates, and wide characters. Even Linux-only LAN requires a fixed-width codec; otherwise compiler or build-option changes can alter the wire format.
- Miles is not a thin sound-device wrapper. The implementation uses callbacks, archive reads, 2D and 3D samples, streams, groups, loops, panning, pitch, listener state, distance, delay, occlusion, and ADPCM (`MilesAudioManager.cpp:41-101`, `:1289-1333`, and `:3179-3193`).
- Movies are Bink files and the player calls the Bink SDK directly (`BinkVideoPlayer.cpp:65-67` and `:233-399`). SDL does not provide a video decoder.
- GDI selects fonts and rasterizes glyphs into an engine atlas (`render2dsentence.cpp:1343-1545`). Language code may load a font stored in the game data (`GlobalLanguage.cpp:124-170`), so system font lookup alone is insufficient.
- GameSpy source references remain, but the corresponding library source is absent. Build exclusion and UI degradation are required; include-path changes cannot restore it.

### What should change

Introduce narrow Linux implementations at the existing platform seams, establish fixed data representations before porting subsystems, and translate the renderer behind the W3D/DX8 wrapper boundary. Treat renderer capability, deterministic simulation, serialization, and asset precedence as explicit deliverables with fixtures.

### What should not be refactored yet

- Do not replace the simulation, entity/component conventions, INI language, W3D model layer, UI model, save-game architecture, or custom allocators merely because they are old.
- Do not replace BIG with PhysicsFS or another package format. The existing format reader is required for retail compatibility.
- Do not modernize all ownership to smart pointers before the program runs. Fix ownership only where new asynchronous audio/video/render work or sanitizer evidence requires it.
- Do not multithread the simulation or introduce an ECS. Either would multiply determinism and regression risk without helping the Linux boundary.
- Do not retain a D3D-shaped public backend indefinitely, but do use it as a temporary migration facade so gameplay and asset code are not rewritten at the same time as the renderer.

## 3. Selected replacement stack

| Existing dependency | Replacement | Why it is required and why it was chosen |
|---|---|---|
| Visual C++ 6 projects and STLport | CMake 3.25+, C++17, system libstdc++/libc++; Ninja recommended | CMake can reproduce the existing target boundaries and dependency graph without keeping IDE metadata authoritative. C++17 supplies filesystem, threading, atomics, and containers without coupling the port to a larger language migration. |
| `WinMain`, User32 messages, DirectInput, IMM, cursor/clipboard/dialog helpers | SDL3 3.2+ | SDL covers windows, X11/Wayland selection, keyboard scancodes, UTF-8 text input/IME, mouse, fullscreen, displays, cursor, clipboard, and small OS services through one maintained layer ([events](https://wiki.libsdl.org/SDL3/SDL_Event), [keyboard guidance](https://wiki.libsdl.org/SDL3/BestKeyboardPractices)). |
| D3D8, D3DX8, `dxguid` | SDL_GPU with its Vulkan driver, provisionally selected by the GPU-independent API closure in M2 and confirmed on hardware in M14 | SDL_GPU supplies explicit pipelines, command buffers, buffers, textures, samplers, render passes, and presentation while SDL owns platform integration ([GPU API](https://wiki.libsdl.org/SDL3/CategoryGPU)). It is materially smaller than owning Vulkan directly and fits the existing state-cached renderer better than a scene framework. The delayed hardware gate deliberately accepts bounded rework risk so GPU-less development hosts remain productive. |
| D3DX/NVASM runtime shader assembly | Repository-owned GLSL compiled offline to SPIR-V with `glslc` | Legacy assembly must be translated semantically. Offline compilation makes shader failures build errors and avoids a runtime compiler dependency ([shaderc](https://github.com/google/shaderc)). |
| Miles, DirectSound, WinMM, Asimp3 | A reviewed and hash-pinned miniaudio release compiled into the game | miniaudio supplies output, mixing, streaming, resource management, WAV/MP3 and ADPCM decoding, groups, filters, and 3D spatialization in one small dependency, with custom I/O suitable for BIG data ([manual](https://miniaud.io/docs/manual/)). SDL audio alone would require the project to recreate these facilities. |
| Bink SDK | FFmpeg shared libraries: avformat, avcodec, avutil, swscale, swresample | FFmpeg contains maintained Bink demux, video, and audio decoders. It also supplies pixel conversion and resampling ([demuxer](https://www.ffmpeg.org/doxygen/trunk/libavformat_2bink_8c_source.html), [video](https://www.ffmpeg.org/doxygen/trunk/libavcodec_2bink_8c_source.html), [audio](https://www.ffmpeg.org/doxygen/trunk/binkaudio_8c_source.html)). This dependency is justified narrowly by required retail media. |
| GDI font APIs | FreeType 2 and Fontconfig; HarfBuzz/FriBidi only when required by the supplied locale | FreeType matches the existing glyph-atlas design. Fontconfig resolves installed fallback families. Fonts inside BIG files must be loaded directly from memory with FreeType, not registered as system files. HarfBuzz and FriBidi are conditional text-layout dependencies, not assumed baseline features. |
| Registry and executable-relative writable files | Existing INI parser plus XDG Base Directory paths | The engine already has a configuration parser. XDG provides predictable config, data, state, and cache locations without adding another library ([XDG specification](https://specifications.freedesktop.org/basedir/latest/)). |
| Winsock | POSIX sockets, `getaddrinfo`, and `poll` | The retained LAN protocol is IPv4/UDP. A networking framework would not remove the need to preserve packet layout, reliability, ordering, timeouts, or simulation synchronization. |
| Win32 threads, critical sections, interlocked operations, timers, and sleep | Standard C++ threads, mutexes, condition variables, atomics, and `steady_clock` | These APIs are sufficient and maintained. A dedicated concurrency or timing framework adds no value. |
| Bundled/missing zlib | System zlib | It preserves the existing ZL-tagged data while receiving distribution security fixes. Use a narrow adapter rather than editing upstream headers into the tree. |
| LZH-Light | No baseline decoder; recognize and reject `NOX` data explicitly | Substituting a different compressor would corrupt compatibility. Repository-wide runtime searches do not show a demonstrated main-game requirement. The supplied data validator is the final gate. |
| SafeDisc, GameSpy, EABrowser, WWDownload, Windows Media, Granny | No replacement | These are absent, obsolete, disabled, or outside the workable main-game boundary. Recreating them would add products and services rather than port the game. |

Heavy and security-sensitive libraries should normally come from the distribution. miniaudio is the exception: compile one reviewed version into the program and record its upstream URL, commit or release, hash, license, and options. Ordinary builds must not use CMake `FetchContent` or access the network.

Do not add SDL_image, stb_image, Boost, Qt, OpenAL, a general game engine, a logging framework, or a serialization framework unless concrete source or corpus evidence shows a gap that the selected stack cannot address.

## 4. Target architecture and build graph

Keep engine-facing interfaces and replace implementations below them:

```text
zh_main
  -> zh_game_engine       simulation, UI, INI, save/replay, LAN behavior
  -> zh_game_device       SDL platform factories and subsystem assembly
  -> zh_w3d               W3D assets and renderer front end
  -> zh_wwshade           runtime shader/effect management
  -> zh_wwsupport         WWMath, WWLib, WWDebug, WWUtil, WWSaveLoad
  -> zh_compression       RefPack, zlib adapter, existing codecs
  -> zh_platform_sdl      window, events, input, dialogs, clipboard
  -> zh_renderer_sdl_gpu  SDL_GPU implementation and pipeline cache
  -> zh_audio_miniaudio   Miles-compatible engine adapter
  -> zh_video_ffmpeg      Bink playback through FFmpeg and miniaudio
  -> zh_os_posix          files, XDG paths, sockets, timing, threading
```

The exact static-library split may be reduced when two targets have circular legacy dependencies, but source ownership must remain explicit. `wwshade` must appear either as `zh_wwshade` or as a documented part of `zh_w3d`; it must not disappear from the source closure.

CMake requirements:

1. Translate `.dsw`/`.dsp` manifests into explicit source lists. Do not glob production sources. Audit the resulting closure against actual includes and unresolved symbols because the legacy solution also names browser, download, benchmark, and missing third-party projects.
2. Generate stable build-version headers with `configure_file`; do not build the Win32 version helper tools.
3. Exclude tools, DRM, browser, GameSpy, Windows Media, CD-audio, Granny, and unused benchmark targets at the source-list level.
4. Add options only for real supported configurations: `ZH_BUILD_TESTS`, `ZH_ENABLE_ASAN`, `ZH_ENABLE_UBSAN`, `ZH_ENABLE_LAN`, `ZH_ENABLE_GPU_TESTS`, and `ZH_ENABLE_RETAIL_TESTS`. The last two default to `OFF`; configuring or testing on a GPU-less machine must not open a display or device. Do not add switches for dead service paths.
5. Use imported targets such as `SDL3::SDL3`, `Freetype::Freetype`, `Fontconfig::Fontconfig`, and `ZLIB::ZLIB`. Provide small imported FFmpeg targets if the installed packages do not export them. Avoid global include/link directories.
6. Compile GLSL to SPIR-V with declared custom commands and dependencies. Missing `glslc` or shader failure must stop the build.
7. Produce `compile_commands.json` and these stable x86-64 presets: `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release`. Presets print the host architecture during configuration and fail clearly on unsupported architectures.
8. Use `-fno-fast-math` and `-ffp-contract=off` for deterministic code. These are necessary safeguards, not a substitute for the numeric work in section 5.
9. Install only the executable, project-owned shaders/defaults, and required license notices. Never install or embed retail media.

The documented interface is `cmake --preset <name>`, `cmake --build --preset <name>`, and `ctest --preset <name>`. CTest labels are part of the milestone contract: `foundation`, `headless`, `data`, `determinism`, `platform`, `renderer-contract`, `ui`, `audio`, `video`, `lan`, `gpu`, and `compatibility`. Milestone checks select labels rather than depending on an undocumented test invocation.

Arch Linux x86-64 is the primary source-build and playable baseline. Record a tested package snapshot because Arch is rolling release. Ubuntu 26.04 LTS and Fedora 44 are clean-build portability baselines; both provide SDL3 and `glslc` packages ([Ubuntu SDL3](https://packages.ubuntu.com/resolute/libsdl3-dev), [Ubuntu glslc](https://packages.ubuntu.com/en/resolute/glslc), [Fedora SDL3](https://packages.fedoraproject.org/pkgs/SDL3/SDL3/index.html), [Fedora glslc](https://packages.fedoraproject.org/pkgs/shaderc/glslc/)). Exact package names for every dependency, including Arch's `vulkan-validation-layers`, and tested versions must be recorded in the M0 build guide rather than inferred by contributors.

## 5. Portable data model, serialization, and determinism

These changes precede renderer or gameplay claims because they affect every persistent and networked boundary.

### Fixed-width types

- Define engine integers with `<cstdint>`: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, and `uint64_t`.
- Preserve `Bool` only as an internal compatibility name. Never use its host representation in a file, CRC stream, or packet.
- Replace pointer-to-`Int` casts with `intptr_t`/`uintptr_t` only where an integer representation is genuinely required.
- Replace assumptions that `long` is 32 bits. Add static assertions for every engine alias and externally visible layout.
- Remove MSVC inline assembly, `__int64`, SEH, pragmas that affect binary layout implicitly, and unaligned typed pointer casts.

### Unicode

Use `char16_t` as `WideChar` and retain `UnicodeString` as a sequence of UTF-16 code units. This is deliberate source churn: it preserves the Win32 representation used by CSF data, saves, replays, packets, string lengths, hashing, and code-unit indexing.

Required work:

- convert engine-owned `L"..."` literals to `u"..."`;
- replace `wcs*`, wide `printf`, and wide `scanf` calls with bounded UTF-16 helpers whose format and failure behavior are tested;
- define UTF-16LE read/write functions independent of host endianness and alignment;
- validate surrogate pairs when converting to or from UTF-8, while allowing existing code-unit operations where retail formats require them;
- expose UTF-8 only at SDL, POSIX path, logging, Fontconfig, and FFmpeg boundaries; and
- keep lossy ASCII conversions explicit and restricted to identifiers that the format defines as ASCII.

Do not use `-fshort-wchar`. It changes the ABI expected by the C/C++ runtime and third-party libraries without making wide CRT calls safe.

### Serialization and packets

Create shared little-/big-endian primitives for fixed-width integers, IEEE-754 32-bit floats, UTF-16LE strings, byte arrays, and bounded length-prefixed collections. Use them for save/replay/CRC/network paths instead of raw host-object copies.

The packet audit must replace every `sizeof(enum)`, `sizeof(Bool)`, `sizeof(WideChar)`, and `memcpy` of structs such as coordinates with named fields. Decoders must validate packet type, exact or bounded length, element count, string terminator/length, and protocol version before allocation or state mutation. Unknown versions fail cleanly.

Preserve Windows 1.04 byte layouts where they are established by source. Save and replay readers must be versioned so an optional later Windows-compatibility path can be added without changing the Linux format. The first release promises Linux-to-Linux persistence and LAN and must not use host-sized fields merely because Windows fixtures are absent. Mixed-OS LAN and Windows save/replay import remain separate acceptance efforts because they require reference clients or files that are not prerequisites for this port.

### Floating-point behavior

The original x87 24-bit precision mode cannot be reproduced globally and reliably in normal x86-64 GCC/Clang code. Use this migration strategy:

1. Inventory calls to `setFPMode`, the `REAL_TO_INT_*` macros, inline x87 helpers, float bit manipulation, and floating-point values included in CRC/packet/save data.
2. Write characterization tests for truncation, floor, ceiling, round-to-nearest, negative halves, values around integer boundaries, large finite values, signed zero, infinities, and NaNs where callers can supply them.
3. Replace assembly with explicit helpers based on `std::trunc`, `std::floor`, `std::ceil`, or `std::nearbyint`, plus checked narrowing and separately specified exceptional behavior. Do not rely on C++ casts where the old macro had different semantics.
4. Establish `FE_TONEAREST` through `<cfenv>` only in translation units that depend on it, enable floating-environment access as supported by each compiler, and assert the mode in deterministic tests.
5. Isolate simulation-sensitive math from renderer, audio, and video code so third-party libraries cannot silently redefine the simulation floating environment.
6. Compare replay CRC checkpoints across GCC/Clang and Debug/Release. Where the portable result differs from the Windows reference, identify the calculation and add explicit quantization or a compatibility helper; do not attempt to restore process-wide x87 precision.

## 6. Platform and runtime subsystem work

### Entry point, SDL, and input

Replace `WinMain` with `main(int, char **)`. Startup order is: parse mode/path/verification arguments, initialize logging and XDG paths, mount and validate data when requested, initialize only the SDL subsystems required by the selected mode, assemble real or null engine subsystems, call the requested verification/headless/game entry point, then unwind in reverse order. `--verify-data` and headless modes must not create a window or GPU device; normal gameplay creates the window before the selected renderer device.

Do not change the process working directory. Remove SafeDisc, launcher, registry, splash-window, SEH, CD, and named-mutex behavior. Multiple local processes are useful for LAN testing. Initialization failures must name the subsystem, relevant path or asset, and underlying library error.

Map SDL scancodes to the engine's physical control identifiers. Use SDL keycodes only for layout-dependent shortcuts and SDL text-input/editing events for UTF-8 text and IME composition. Do not synthesize text from key presses. On focus loss, clear held keys/buttons and stop relative capture; keep simulation pause behavior consistent with the existing game mode. Preserve drag, double-click, wheel, edge scrolling, confinement, cursor visibility, fullscreen/windowed switching, and resize behavior.

### Filesystem, data roots, and XDG paths

Add these game-owned interfaces:

```text
--zh-data PATH          Zero Hour retail data root
--generals-data PATH    base Generals data root
--language NAME         supplied retail locale to load
--verify-data           validate data and exit before window/GPU creation
```

Configuration keys are `ZeroHourDataPath`, `GeneralsDataPath`, and `Language`. Command-line values override configuration. The two roots may be identical for a complete combined installation. If either root is unresolved, startup fails and prints the relevant option and configuration key. If no language is selected, accept the only valid locale found by verification; if zero or multiple locales are viable, fail and list the available names rather than guessing. Do not add automatic Steam/Wine/registry scanning to the initial port.

Use XDG locations. When an XDG variable is unset, use the specification's home-relative fallback rather than the working directory:

- configuration: `$XDG_CONFIG_HOME/generals-zero-hour/`, otherwise `$HOME/.config/generals-zero-hour/`;
- saves, replays, user maps, and screenshots: `$XDG_DATA_HOME/generals-zero-hour/`, otherwise `$HOME/.local/share/generals-zero-hour/`;
- logs and crash/run state: `$XDG_STATE_HOME/generals-zero-hour/`, otherwise `$HOME/.local/state/generals-zero-hour/`; and
- regenerable shader/pipeline or asset indices: `$XDG_CACHE_HOME/generals-zero-hour/`, otherwise `$HOME/.cache/generals-zero-hour/`.

Never write to either retail data root.

The virtual filesystem must normalize `\` and `/`, reject absolute paths and `..`, and ASCII-case-fold retail logical paths. Preserve this precedence from highest to lowest:

1. loose files in the Zero Hour root, matching the current `FileSystem` rule that local files win before archive lookup;
2. an explicitly selected mod BIG or BIG files in the selected mod directory, using existing `-mod` semantics;
3. Zero Hour BIG archives;
4. loose files in the base Generals root; and
5. base Generals BIG archives.

Sort archive discovery with the existing case-insensitive `FilenameList` order, then encode and test its first-loaded-wins behavior (`FileSystem.h:66-67`, `Win32BIGFileSystem.cpp:214-235`). Duplicate logical resources across different BIG archives are expected and resolve through this mount order. A collision between two loose host paths, or between two archive names, that differ only by case is an error because Linux directory enumeration cannot reproduce a unique Windows identity; report both physical sources. Do not depend on raw filesystem enumeration order.

Keep the current BIG reader but replace Win32 enumeration, `_access`, path buffers, Winsock byte-order incidental dependencies, unchecked path reads, unaligned casts, and 32-bit offset arithmetic. Validate identifiers, counts, NUL-terminated path lengths, offsets, sizes, offset-plus-size overflow, and allocation limits before exposing an entry.

`--verify-data` is part of the game executable, not a separate asset tool. It must print the resolved roots, locale, archive mount order, representative required resources, detected image/audio/video/font/compression formats, case-fold collisions, and exact missing or unsupported logical paths. It must not hash, upload, copy, or rewrite retail files. The exact required-file manifest is derived from the user's corpus and then checked into the repository only as logical names and format expectations.

### GPU-independent renderer closure

SDL_GPU remains the preferred renderer, but most development must work without a physical GPU. M2 therefore performs a source-and-API closure, not a device test. It mechanically inventories every D3D state, resource, format, primitive, shader, and direct-device escape and maps each one to a public SDL_GPU operation, a shader implementation, a CPU fallback, or an explicit rejection. It must close the following areas:

- vertex/index streaming and all used primitive types, including point-list behavior and vertex-shader point size;
- color and depth render targets, sampling from a completed render target, resize/recreation, and required depth/stencil formats;
- 2D, cube, and 3D textures if the source/corpus uses them;
- BC1, BC2, and BC3 uploads through formats confirmed by `SDL_GPUTextureSupportsFormat`, plus a CPU decode-to-RGBA8 fallback;
- dynamic vertex/index/uniform uploads without per-draw resource leaks or stalls;
- packed per-frame, per-material, and per-object uniform blocks that fit SDL_GPU's shader resource conventions and four uniform-buffer slots per shader stage ([shader creation](https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader));
- blend, color mask, depth, stencil, culling, fill, sampler/addressing, fog, alpha test, texture-coordinate generation, projected textures, bump-environment mapping, and multipass combinations used by W3D/WWShade; and
- representative translated shaders for UI, terrain, water, particles/points, and one WWShade effect, all compiled offline without creating a GPU device.

M2 passes when every source-observed behavior has a documented public-API mapping or tested fallback, shader source and pipeline descriptors can be generated, all shaders compile offline, and no proposed implementation calls Vulkan directly. This closes a provisional backend decision; it cannot prove device support, pixels, synchronization, or performance.

M7 introduces a small `RecordingGpuDevice` behind the same engine-internal device interface as the SDL_GPU implementation. It records resource lifetimes, descriptors, uploads, pipeline keys, render-pass boundaries, draw order, resize/recreation sequencing, and error labels. Renderer, UI, world, effects, and video tests compare normalized command streams and reject invalid lifetimes, missing bindings, out-of-range uploads, pass misuse, and unbounded pipeline growth. It does not pretend to validate rasterized pixels.

The first mandatory real-GPU work is M14, after command generation is complete. M14 opens one real x86-64 Vulkan-capable GPU through SDL_GPU, enables validation, exercises representative retail scenes, and performs visual and lifecycle acceptance. One working GPU and driver are sufficient; multiple graphics cards or driver families are not prerequisites. If M14 reveals an SDL_GPU abstraction gap, reopen M2, record the failed capability and reproducer, choose one backend below, rework M7-M10 against it, and repeat M14. Deferring the device test keeps GPU-less and restricted development sessions productive but explicitly accepts this rework risk.

If it fails:

1. Choose bgfx when the missing behavior is an abstraction capability bgfx supports and its shader/resource model can cover the inventory without engine redesign.
2. Choose raw Vulkan only when both abstractions lack a required capability or impose an unworkable synchronization/resource restriction. This means accepting ownership of instance/device/swapchain creation, descriptors, synchronization, and platform integration.

Do not continue building a private Vulkan escape hatch under SDL_GPU. That would retain both abstraction costs.

### Renderer implementation

- Replace D3D objects with opaque engine handles and descriptors for buffers, textures, samplers, shaders, pipelines, and render targets.
- Keep a state cache at the `DX8Wrapper` seam during migration. Convert the complete effective state tuple into immutable pipeline keys; do not create pipelines opportunistically without bounded caching and diagnostics.
- Define DDS headers with fixed-width local structures rather than `ddraw.h`. Preserve DXT2/4 premultiplied-alpha semantics when mapping BC formats. Retain the existing TGA path; add no general image library unless the corpus proves another required format.
- Translate every in-scope fixed-function and assembly path into named GLSL shaders. Maintain a table from legacy shader/effect names and state combinations to new shader/pipeline definitions. Build-time compilation must cover every table entry.
- Lock coordinate handedness, 0–1 depth, winding/culling, half-pixel UI placement, color packing, depth bias, fog distance, texture origin, and premultiplied alpha with focused render fixtures. SDL_GPU's coordinate normalization reduces backend differences but does not decide legacy D3D semantics for the engine.
- Replace lost-device callbacks with explicit window/swapchain/resource recreation. Retain CPU copies only for generated resources that cannot be recreated from an asset or deterministic generator.
- Use GPU debug labels and validation in development builds. Do not make RenderDoc or Vulkan validation layers runtime requirements.

### Audio

Implement the existing audio manager interface over miniaudio rather than changing gameplay audio call sites. Required behavior includes one-shot and looping 2D/3D sounds, streaming music and speech, priority/voice limits, groups, volume, pan, pitch, listener transforms, min/max distance, attenuation, delay, occlusion/low-pass behavior, completion callbacks, pause/focus handling, and orderly shutdown.

Provide miniaudio VFS callbacks backed by the engine file interface so archived media streams without extraction. Do not let real-time callbacks allocate through engine global allocators, take simulation locks, or call game objects. Queue completion notifications to the owning game thread. Confirm PCM WAV, Microsoft/IMA ADPCM, MP3, and every format found by `--verify-data`.

If no audio device is available, issue one clear warning and allow silent play. A corrupt or unsupported required asset remains a data error, not a no-device condition.

### Video

Implement the existing video-player interface with FFmpeg custom I/O over engine files. First validate headless Bink demux, video decode, pixel conversion, timestamps, and decoded-audio delivery into a null/miniaudio sink. GPU texture upload and presentation are joined to the real renderer later. Preserve localized-path fallback, aspect ratio, scaling, skip input, pause/focus behavior, end-of-stream callbacks, and transition back to game rendering.

Use media timestamps for A/V synchronization and an audio clock when audio exists. Bound demux packets, decoded frame dimensions, conversion buffers, and queue length. Missing Bink decoders must be reported by `--verify-data` or startup capability reporting. Use distribution FFmpeg shared libraries and document their configuration/license obligations; do not enable nonfree components in project builds.

### Fonts and locale

Read `UnicodeFontName` and `LocalFontFile` from the selected language configuration. If `LocalFontFile` resolves through the VFS, retain its bytes for the face lifetime and call `FT_New_Memory_Face`; do not assume an archive resource has a host path and do not install/register it system-wide. Use Fontconfig only to resolve system family names and Windows-family aliases when no supplied font file is selected.

Preserve atlas behavior, baseline, bearing, advance, overlap, hinting, and grayscale alpha. R8 is preferred for new glyph atlases when the shader reproduces the legacy result; RGBA8 is acceptable where color composition is simpler. Log the chosen face and fallback. Missing glyphs use a deliberate fallback or replacement glyph and never index outside the atlas.

The supplied locale determines the minimum text-layout dependency:

1. Use the existing per-codepoint layout plus FreeType for Latin, Cyrillic, CJK, or another locale that passes shaping, ordering, metrics, and screenshot tests.
2. Add HarfBuzz if the locale requires contextual shaping or glyph substitution ([shaping API](https://harfbuzz.github.io/harfbuzz-hb-shape.html)).
3. Add HarfBuzz and FriBidi if it also requires bidirectional layout. HarfBuzz alone does not perform paragraph bidi ordering.

This gate must be resolved from the supplied locale before declaring text complete. Test menus, tooltips, subtitles, captions, save names, player/game names, chat input, composition, truncation, wrapping, fallback, and mixed ASCII/non-ASCII strings.

### Time, threads, diagnostics, and compression

- Use `steady_clock` for frame durations, timeouts, audio scheduling, and profiling. Supply an explicit wrapping 32-bit millisecond clock only where the old protocol expects `timeGetTime` behavior, and test wraparound comparisons.
- Port critical sections to standard mutexes, documenting the few call paths that genuinely require recursion. Join every worker thread during shutdown; do not paper over ownership bugs by making all fields atomic.
- Keep the existing logging facade with a POSIX/XDG sink. Emit a startup capability report containing build revision, architecture, SDL version, FFmpeg Bink decoder availability, audio backend/device, data roots, and locale. When a GPU is intentionally initialized, append its selected driver/device and relevant texture/depth support; headless commands must report GPU initialization as skipped, not failed.
- Keep B-tree, Huffman, RefPack, and other used engine compression. Adapt ZL tags to system zlib without changing tag or level semantics.
- Recognize `NOX\0`/LZH and return a typed unsupported-compression error naming the archive and logical file. Do not compile a speculative decoder and never treat the data as uncompressed.
- Add allocation/size bounds and fuzzable readers for BIG, compressed blocks, saves, replays, packets, INI/CSF, image headers, and media custom I/O.

### LAN and removed online services

Use `getaddrinfo`, `socket`, `bind`, `sendto`, `recvfrom`, `fcntl(O_NONBLOCK)`, `poll`, `close`, and `errno`. Keep broadcast discovery, advertisements, joins, timeouts, existing reliability/order logic, map negotiation/transfer, and simulation synchronization at their current layers.

The existing code assumes one peer per IP and fixed lobby/gameplay ports (8086 and a base of 8088), so repeatable local testing requires narrow injection points. Add developer-only `--lan-bind-address`, `--lan-discovery-address`, and `--lan-direct-connect` arguments. Use `127.0.0.2` and `127.0.0.3` for two local processes so their identities remain distinct without requiring two hosts. Attempt real loopback broadcast discovery; if the host does not deliver it, validate discovery state through an in-process virtual datagram transport and validate real process-to-process joins through direct connect. In that case physical-subnet broadcast remains explicitly unverified, not silently assumed. The release gate requires two local windowed or headless game instances, not two computers.

Compile out GameSpy peer/chat/presence/stats/patch code, WWDownload, and Internet-only actions. Internet menu entries should either be removed or present one local, non-blocking “Internet services are unavailable in this build” explanation. They must not attempt DNS, connect to retired hosts, or leave a waiting UI.

## 7. Prerequisite ledger and dependency order

Every prerequisite has an owner, acquisition method, consumer, validation, and failure action. “Available later” is not a valid prerequisite state. Repository-owned items are produced by an earlier milestone; user-owned items are requested only at the first milestone that consumes them.

| ID | Prerequisite and owner | First consumer | Readiness check | If unavailable or failed |
|---|---|---|---|---|
| PRE-001 | Zero Hour source and legacy `.dsw`/`.dsp` manifests; repository | M0 | Named source directories and project files exist | Block M0 and report missing paths |
| PRE-002 | CMake, Ninja, GCC, Clang, `pkg-config`, `glslc`, and development packages listed in the build guide; developer/user | M0 | Both Debug presets configure; dependency probe prints versions and host architecture | Install the named packages; do not download during configure |
| PRE-003 | Audited include/exclude and third-party-dependency manifests; repository | M0 | Every legacy project/source is classified as build, exclude with reason, or unavailable/out of scope | M0 creates and reviews the manifests before linking |
| PRE-004 | CMake targets, presets, CTest labels, synthetic fixtures, and clean CI entry points; repository | M0 | Documented configure/build/test commands work without retail data | Fix bootstrap; no later milestone may invent a private build path |
| PRE-005 | x86-64-safe ABI, UTF-16, endian, numeric, filesystem, timing, threading, and compression support; repository | M1 | `foundation` tests pass with GCC and Clang Debug/Release plus static width assertions and byte-exact codec fixtures | Fix M1; do not hide Win32/LP64 width differences with compiler flags |
| PRE-006 | GPU-independent legacy-state inventory and SDL_GPU mapping; repository | M2 | Every source-observed state has a public API mapping, shader path, fallback, or rejection; all GLSL compiles | Resolve the mapping or select another backend before M7 |
| PRE-007 | Null platform/render/audio/video implementations and deterministic headless loop; repository | M3 | Headless process reaches controlled ticks and shuts down with no display, GPU, or audio device | Fix headless boundary; it is required by data, simulation, and LAN work |
| PRE-008 | Owned Zero Hour and base Generals data plus selected locale; user, outside Git | M4 | `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, and `ZH_RETAIL_LANGUAGE` resolve to readable inputs | Asset-free tests continue, but M4 and dependent retail gates wait for data |
| PRE-009 | Project-owned logical corpus manifest; repository, derived without copying data | M4 | Manifest names archive order, required logical assets, formats, effects, locale/font, and accepted collision rules | M4 remains open until corpus traversal is complete |
| PRE-010 | `RecordingGpuDevice` and normalized renderer command schema; repository | M7 | Self-tests catch invalid lifetime, pass, binding, upload, and resize sequences | Renderer milestones cannot proceed without observable GPU-free contracts |
| PRE-011 | Reviewed, license-compatible, URL/hash-pinned miniaudio source; repository maintainer | M12 | Hash/license/options are recorded and offline build succeeds | Choose and record a suitable release; no unpinned fetch fallback |
| PRE-012 | The primary Arch x86-64 system with one real Vulkan-capable GPU supported by SDL_GPU; developer/user | M14 | SDL_GPU opens the RTX 4070 through Vulkan and Khronos validation is available | Install Arch `vulkan-validation-layers` if absent; defer M14-M17 if device or validation checks fail |
| PRE-013 | Real-GPU capability, lifecycle, and visual acceptance record; repository | M15 | M14 report identifies device/driver, scenes, expected captures, validation output, and disposition | Reopen renderer milestones on backend gaps; gameplay acceptance cannot bypass it |
| PRE-014 | Local multi-instance bind/discovery/direct-connect overrides and virtual datagram transport; repository | M11 | Two peers can use distinct loopback identities and deterministic injected packets | Fix M11; a second physical host is not an alternative prerequisite |
| PRE-015 | Representative Windows 1.04 saves/replays and legal reference environment; optional user input | M18 | Fixture provenance/version is recorded and files are readable | Skip M18 with an explicit “unverified” status; M0-M17 are unaffected |
| PRE-016 | SDL3-capable interactive display session; developer/user, software or virtualized display is acceptable | M6 | SDL can create a window and deliver focus/input/resize events without creating a GPU device | Run injected headless tests, but defer M6's interactive exit check until a display is available |
| PRE-017 | Clean x86-64 Ubuntu 26.04 and Fedora 44 build environments; maintainer/CI | M17 | Documented packages install and presets configure in a fresh container, VM, or CI runner | M17 remains open; physical hosts are not required |

Current local readiness evidence for PRE-008 is the read-only combined Steam installation exposed as `original_game_symlink`: Zero Hour is at its root, base Generals is under `ZH_Generals`, and `English` is the selected locale. The link is local-only and ignored by Git; no retail bytes or absolute private paths belong in repository artifacts. All 36 observed BIG archives have readable headers, and the initial corpus scan found no `NOX` entries or case-fold collisions.

Current host evidence for PRE-012 is an NVIDIA GeForce RTX 4070 with NVIDIA 610.57.04 and Vulkan 1.4.341. Direct host probing succeeds; restricted development sandboxes may hide the GPU and graphical session. The Khronos validation layer is not currently installed, so PRE-012 cannot close until `vulkan-validation-layers` is installed and SDL_GPU validation succeeds. PRE-015 remains optional: no Windows save or replay fixtures were found in the retail installation, its BIG archives, or the corresponding Steam/Proton user-data directories.

The dependency graph is:

```text
M0 -> M1 -> M3 -> M4 -> M5
             |      |-> M12
             |      `-> M11
             `-> M6

M0 -> M2
M2 + M3 + M4 -> M7
M4 + M6 + M7 -> M8
M4 + M7 -> M9
M4 + M7 -> M10
M4 + M7 + M12 -> M13
M7 + M8 + M9 + M10 + M13 -> M14
M5 + M8 + M9 + M10 + M12 + M13 + M14 -> M15
M8 + M11 + M15 -> M16
M15 + M16 -> M17
M17 -> M18 (optional compatibility validation only)
```

M2 may run alongside M1-M6 because it is source/API analysis and offline shader compilation. M11 transport work may begin after M3 and M4, but final local match acceptance remains M16. M12 is independent of GPU work. M14 is intentionally the first required hardware-GPU milestone.

For every preset, the canonical build check is `cmake --build --preset <preset>` followed by `ctest --preset <preset> -L '<label-regex>' --output-on-failure`. Retail checks use a separate build directory configured with `ZH_ENABLE_RETAIL_TESTS=ON` and the three `ZH_RETAIL_*` variables; GPU checks additionally use `ZH_ENABLE_GPU_TESTS=ON`. These options must never make unlabeled asset-free tests depend on private data or hardware.

| Milestone | Required automated labels/checks at exit |
|---|---|
| M0 | Configure/build smoke with GCC and Clang Debug/Release presets on Arch x86-64; clean Ubuntu/Fedora configure coverage may run early; empty and missing-dependency probes |
| M1 | `foundation`; ASan and UBSan variants on available native hosts |
| M2 | `renderer-contract` offline descriptor/shader table checks; no GPU tests enabled |
| M3 | `foundation|headless` with display, audio, and GPU unavailable |
| M4 | `data` with synthetic inputs, then retail-enabled verification using PRE-008 |
| M5 | `foundation|determinism` on x86-64 GCC/Clang Debug/Release |
| M6 | `platform` injected-event suite plus the PRE-016 interactive smoke |
| M7 | `renderer-contract` including all negative lifetime/pass/upload cases |
| M8 | `platform|renderer-contract|ui` |
| M9 | `renderer-contract` world-scene corpus cases |
| M10 | `renderer-contract` WWShade/effects corpus cases and shader-table completeness |
| M11 | `lan|headless` virtual transport and two-process direct-connect checks |
| M12 | `audio` with null/no-device and corpus decode probes |
| M13 | `audio|video|renderer-contract` headless decode/presentation checks |
| M14 | `gpu|ui|video` plus documented retail visual/lifecycle checklist on PRE-012 |
| M15 | Full non-LAN suite plus documented single-player retail checklist and sanitizers |
| M16 | `lan|headless` plus the two-process complete-match checklist |
| M17 | All labels in clean x86-64 Ubuntu/Fedora environments; retail and GPU jobs remain explicitly provisioned |
| M18 | Optional compatibility-fixture tests, kept in a distinct label so normal suites never require them |

## 8. Delivery milestones

Each exit is binary. A partial demonstration is evidence, not completion. When a precondition fails, record the failed check and stop only that milestone and its dependants.

### M0 — reproducible build readiness

Preconditions: PRE-001 and the user-installed portion of PRE-002.

Work:

- translate all relevant Zero Hour project manifests into explicit CMake source lists and classify every exclusion;
- model `zh_main`, engine/device, W3D, WWShade, support, compression, test, and null-backend targets without source globs;
- add the four native presets, dependency/version probes, `compile_commands.json`, generated build metadata, CTest labels, and offline behavior;
- document exact Arch Linux, Ubuntu 26.04, and Fedora 44 packages, including CMake, Ninja, both compilers, SDL3, `glslc`, FreeType, Fontconfig, zlib, FFmpeg development packages, and Vulkan validation tooling; record the tested Arch package snapshot; and
- add an asset-free CI/smoke target that proves the build graph without pretending the game links yet.

Exit: PRE-003 and PRE-004 are complete; GCC and Clang Debug/Release presets configure from a clean checkout on x86-64, tests can be selected by label, missing packages name their package/capability, and configure performs no network access.

### M1 — portable ABI and support libraries

Preconditions: M0.

Work:

- introduce fixed-width engine types, `char16_t` `WideChar`, UTF-8/UTF-16 conversion, endian codecs, bounded readers/writers, and portable formatting helpers;
- port support libraries needed below the game loop: files, paths, XDG locations, clocks, threads, atomics, sockets' shared types, RefPack, and the zlib adapter;
- replace compiler extensions, inline x87 conversion helpers, `long`/pointer-size assumptions, and unaligned host reads in this closure;
- characterize numeric conversions and establish deterministic compiler/floating-environment flags; and
- add synthetic BIG, Unicode, compression, serialization, numeric, path, and timer fixtures.

Exit: PRE-005 is complete. `foundation` tests pass with GCC and Clang in x86-64 Debug and Release builds and under ASan/UBSan where available. Serialized primitive fixtures are byte-identical across compilers and build types.

### M2 — GPU-independent renderer API closure

Preconditions: M0; no display, Vulkan loader, GPU, or retail data is required.

Work:

- inventory D3D/D3DX types, direct-device calls, formats, fixed-function states, FVF layouts, primitives, shader assembly, render targets, lost-device hooks, and WWShade paths;
- map every entry to SDL_GPU public APIs, repository GLSL, CPU format conversion, or an explicit unsupported decision;
- define engine-owned renderer descriptors and the internal device interface shared by the recorder and real backend;
- define uniform packing, resource limits, pipeline-key composition, shader/effect lookup, coordinate/depth/color conventions, and resize state transitions; and
- compile representative and table-generated GLSL to SPIR-V offline.

Exit: PRE-006 is complete, the mapping table has no unexplained entries, public engine-facing headers no longer need new D3D-shaped dependencies, all listed shaders compile, and SDL_GPU is recorded as the provisional backend. Actual device behavior remains unverified until M14.

### M3 — headless engine and staged startup

Preconditions: M1.

Work:

- replace `WinMain` with argument parsing and staged subsystem construction/destruction;
- provide null platform, renderer, audio, and video implementations selected without initializing SDL video or a GPU;
- remove launcher, DRM, registry, splash, named-mutex, browser, GameSpy, CD, SEH, and working-directory dependencies from the headless path;
- expose deterministic fixed-tick smoke/scenario execution, clean termination, logs, and exit codes; and
- verify multiple simultaneous headless processes use separate writable-state directories when instructed.

Exit: PRE-007 is complete. An asset-free headless executable starts, advances controlled ticks, reports intentionally skipped devices, and shuts down cleanly on x86-64 under both compilers.

### M4 — VFS, retail data, and corpus characterization

Preconditions: M1, M3, and PRE-008. If retail data is temporarily unavailable, synthetic VFS work may proceed but the milestone cannot close.

Work:

- implement explicit Zero Hour/base Generals roots, language selection, XDG configuration, deterministic loose/BIG precedence, case folding/collision reporting, and hardened BIG traversal;
- implement `--verify-data` so it never creates a window, GPU device, or audio device;
- parse enough INI, CSF, W3D, DDS/TGA, audio, video, font, compression, and WWShade metadata to inventory the supplied corpus safely; and
- check in only logical names, format/effect expectations, archive precedence, and synthetic regressions—never retail bytes or private paths.

Exit: PRE-009 is complete. Verification works from any CWD, names each resolved root and locale, produces a stable corpus manifest, accepts the selected owned installation, and gives actionable failures for missing, corrupt, colliding, traversing, oversized, or unsupported inputs.

### M5 — Linux persistence and determinism

Preconditions: M4.

Work:

- replace save, replay, CRC, and snapshot raw-memory serialization with versioned fixed-width fields;
- implement Linux save/load and replay write/read round trips, autosave metadata, bounded collection decoding, and clear version rejection;
- run deterministic headless scenarios repeatedly across GCC/Clang and Debug/Release on x86-64; and
- isolate simulation floating-point state from third-party libraries and diagnose every divergent checkpoint.

Exit: new Linux saves load to equivalent state, Linux replays reproduce matching x86-64 CRC checkpoints across supported compilers/build types, malformed inputs fail before mutation/allocation abuse, and no Windows fixture is required. Windows import remains explicitly unverified until M18.

### M6 — SDL platform and input without GPU

Preconditions: M1, M3, and PRE-016 for the final interactive check; headless event work does not wait for a display. No physical GPU or Vulkan device is needed.

Work:

- implement SDL lifecycle, window/events, scancode mapping, mouse/cursor/clipboard, UTF-8 text input and IME editing, focus, resize, and fullscreen state;
- keep GPU-device creation behind the renderer interface and disabled for these tests;
- test event translation through injectable SDL event fixtures so headless CI covers mappings without a window; and
- provide actionable behavior when no display server exists.

Exit: `platform` tests pass headlessly, and one interactive window test validates keyboard, mouse, text input, focus, resize, and shutdown without creating a GPU device.

### M7 — renderer core contracts with `RecordingGpuDevice`

Preconditions: M2, M3, and M4. M7 produces PRE-010 from the device-interface design closed in M2.

Work:

- implement `RecordingGpuDevice` plus opaque buffers, textures, samplers, shaders, pipelines, render targets, and handles;
- implement resource creation/destruction, dynamic uploads, DDS/TGA parsing, BC fallback conversion, render-pass rules, pipeline caching, and resize/recreation command generation;
- normalize unstable IDs in snapshots while retaining state ordering and error provenance; and
- add negative tests for stale handles, missing bindings, bad ranges, illegal pass nesting, unsupported descriptors, cache explosion, and resource use after destruction.

Exit: PRE-010 is complete. `renderer-contract` tests prove correct command/resource behavior from the corpus and synthetic fixtures with no display or GPU, and no engine caller reaches SDL_GPU or Vulkan directly.

### M8 — UI, fonts, and input integration

Preconditions: M4, M6, and M7.

Work:

- port 2D/UI draw generation, clipping, blending, half-pixel behavior, cursor layers, and menu/loading-screen transitions to renderer commands;
- integrate FreeType memory faces and Fontconfig fallbacks, then decide whether the selected locale requires HarfBuzz and/or FriBidi;
- connect SDL physical input and text composition to existing UI controls; and
- cover menu, tooltip, subtitle, save/player names, chat edit, wrapping, truncation, fallback glyph, focus, and resize command streams.

Exit: menu flows are navigable using the null/recording renderer, the locale decision is closed with tests, every UI shader compiles, and recorded commands cover all selected-corpus UI/font paths. Pixel appearance remains a M14 concern.

### M9 — world rendering command generation

Preconditions: M4 and M7.

Work:

- port meshes, vertex/index streaming, cameras, terrain, roads/decals, trees, shadows, factions, animation-visible geometry, selection markers, and render-target dependencies;
- translate relevant fixed-function transforms, lighting, fog, alpha test, texture stages, addressing, culling, depth, and color masks into shader/pipeline descriptors; and
- assert bounded pipeline/resource growth across map load, repeated frames, teardown, and reload.

Exit: representative maps and faction scenes produce deterministic, validated command streams with no unmapped corpus state and no leaked renderer handles.

### M10 — effects, water, and WWShade command generation

Preconditions: M4 and M7.

Work:

- include WWShade in the real build graph and translate its runtime effect mappings;
- port water, particles and point sprites, projected textures, bump/environment effects, stealth, post-effects, multipass materials, depth bias, and premultiplied-alpha paths;
- maintain a checked table from every observed legacy shader/effect to GLSL and pipeline state; and
- reject unknown required effects with the logical asset/material rather than silently falling back.

Exit: every effect in the corpus manifest compiles and produces a valid recorded command sequence; unsupported formats use tested CPU fallbacks or keep the milestone open.

### M11 — POSIX LAN transport and local peer harness

Preconditions: M3 and M4. M11 produces PRE-014.

Work:

- finish the fixed-width packet codec and POSIX nonblocking UDP transport with bounded decoding, timeouts, wrap-safe clocks, version rejection, and diagnostics;
- add the three developer network overrides and distinct loopback identities;
- add a virtual datagram transport for deterministic discovery, loss, duplication, reorder, corruption, delay, and disconnect tests; and
- prove two real headless processes can advertise or direct-connect, join, negotiate data/map identity, exchange commands, and disconnect cleanly.

Exit: PRE-014 is complete. `lan` tests cover protocol failures and two local peers. If loopback broadcast is unavailable, that limitation is recorded while virtual discovery and real direct-connect still pass. Neither a second host nor Windows peer is required.

### M12 — audio

Preconditions: M4 and PRE-011.

Work:

- implement the existing manager over miniaudio with engine VFS callbacks, 2D/3D voices, streams, groups, priorities, loops, pan, pitch, attenuation, delay, occlusion/filtering, and queued completions;
- keep the audio callback free of simulation locks, engine allocation, and game-object calls;
- validate all corpus encodings plus PCM/ADPCM/MP3 synthetic cases; and
- support deterministic null/no-device execution, focus/pause, device failure, and race-free shutdown.

Exit: `audio` tests and corpus probes pass, representative speech/music/effects schedule correctly, no-device mode is playable, and corrupt assets name their logical path.

### M13 — headless video decode and presentation commands

Preconditions: M4, M7, and M12.

Work:

- implement FFmpeg custom I/O, Bink demux/decode, bounded frame/audio queues, pixel conversion, timestamps, and localized fallback;
- validate every observed Bink variant by decoded metadata/frame hashes or other non-retail derived expectations without displaying it;
- route audio through the audio adapter/null sink and generate texture upload/draw commands through the recorder; and
- cover skip, pause/focus, end-of-stream, corrupt/truncated packets, excessive dimensions, missing decoder, and shutdown.

Exit: `video` tests decode representative media headlessly with stable timing/order and valid recorded presentation commands. Actual A/V presentation and pixels remain M14 work.

### M14 — first real-GPU integration and visual acceptance

Preconditions: M7-M10, M13, PRE-012, and a user-supplied retail corpus. This is the first milestone that requires an actual GPU.

Work:

- implement/connect the SDL_GPU device using the M2 interface and exercise resource, pass, upload, pipeline, resize, loss/recreation, and presentation paths with validation enabled;
- run representative UI, fonts, terrain, each faction, water, particles, shadows, roads/decals, trees, stealth, WWShade, movies, focus, resize, and fullscreen cases;
- capture project-owned reference screenshots or metrics without committing retail assets, and review coordinate, depth, winding, half-pixel, color, alpha, fog, bias, texture-origin, and A/V behavior; and
- record device/driver/SDL versions, capability queries, validation output, scene coverage, deviations, and dispositions.

Exit: PRE-013 is complete on one real x86-64 Vulkan GPU. Required scenes render acceptably, device lifecycle tests pass, no validation errors remain, and performance is sufficient for functional acceptance. A second GPU or driver family is useful evidence but not a gate. If a public SDL_GPU capability is genuinely insufficient, M14 fails and triggers the backend decision in section 9.

### M15 — playable single-player

Preconditions: M5, M8-M10, M12-M14.

Work:

- connect all completed subsystems into campaign, skirmish, loading, pause/options, user-map, save/load, replay, victory/defeat, movie, and shutdown flows;
- complete progression/state directories, error recovery, long-game and repeated load/unload testing;
- validate selected-locale text, speech, movies, effects, music, UI, maps, all factions, and representative missions; and
- run x86-64 GCC/Clang Debug/Release determinism checks plus ASan/UBSan suites.

Exit: the game can start from arbitrary CWD, validate owned data, complete representative campaign and skirmish sessions, save/load and replay Linux-created state, and exit cleanly. No Windows artifact or network peer is required.

### M16 — local multi-instance Linux LAN

Preconditions: M8, M11, and M15.

Work:

- launch two local windowed or headless instances with isolated writable state and `127.0.0.2`/`127.0.0.3` identities;
- validate discovery where supported, direct connection, lobby/join, localized names/chat, map negotiation/transfer, ready/start, synchronized gameplay, pause/focus/minimize, completion, disconnect, and desync reporting;
- exercise virtual loss/reorder/duplication and malformed/version/data/map mismatch cases; and
- compare simulation CRC checkpoints between the local peers.

Exit: two processes on one Linux machine complete a LAN match with matching CRCs and clean shutdown. Physical-subnet broadcast and mixed Windows/Linux LAN are documented as unverified unless separately tested; neither blocks release.

### M17 — reproducible x86-64 release

Preconditions: M15, M16, and PRE-017.

Work:

- run clean x86-64 GCC and Clang Debug/Release builds and labeled asset-free suites without network access;
- run ASan/UBSan, private retail-data smoke/acceptance, shader completeness, install/uninstall staging, arbitrary-CWD, read-only input-data, and missing-device tests;
- install only executable/project assets/defaults/licenses and document data selection, XDG locations, limitations, troubleshooting, and verified hardware; and
- verify the playable release on the primary Arch Linux x86-64 host and verify source builds on Ubuntu 26.04 LTS and Fedora 44. The latter may use clean containers/VMs for build and headless checks; the Arch M14 hardware result satisfies GPU evidence.

Exit: a clean checkout builds with documented distribution dependencies, the Arch x86-64 host can point it at owned data and pass the playable completion checklist, Ubuntu and Fedora clean builds pass, and known unverified compatibility claims are explicit.

### M18 — optional Windows save/replay compatibility

Preconditions: M17 and PRE-015. This milestone is outside the release dependency chain.

Work:

- import provenance-recorded Windows 1.04 save and replay fixtures, compare decoded fields and replay checkpoints, and add legal non-retail derived regression expectations;
- determine whether incompatibilities are codec defects, version differences, data/config differences, or irreproducible Windows floating-point behavior; and
- add narrow versioned compatibility handling only where evidence is sufficient and it does not regress Linux persistence/determinism.

Exit: supported fixture versions import with documented behavior, or the exact incompatibility remains documented. If fixtures never become available, status is “not validated,” not “failed,” and the Linux release remains complete.

## 9. Evidence-gated decisions

Only these decisions remain open because source analysis or user-supplied corpus/hardware evidence is required.

### Renderer backend at M14

**Decision update (2026-09-21):** the second branch is selected. M22 exposed the source-required active-frame camera-viewport color/depth/stencil clear missing from SDL_GPU's public API, and an executable bgfx Vulkan probe passed the corresponding ordered inset/outer preservation test on the RTX 4070. The accepted [renderer backend migration](zero-hour-renderer-backend-migration.md) governs the device-edge replacement and M7–M10/M14 revalidation before M22 resumes. Earlier SDL_GPU descriptions in this plan record the provisional baseline, not current backend acceptance.

- **SDL_GPU passes:** retain it. This remains the smallest integration and expected outcome.
- **SDL_GPU has a demonstrated abstraction gap that bgfx covers:** replace only the device/backend layer with bgfx while retaining SDL3 for platform/input. Rework and rerun M7-M10 and M14.
- **Both abstractions demonstrably fail:** implement raw Vulkan and explicitly budget device selection, swapchains, descriptors, synchronization, memory allocation, pipeline caching, and SDL window interop. Rework and rerun M7-M10 and M14.

The decision record must name the failed public capability, source/corpus consumer, and minimal reproducer. Performance tuning, preference, or a driver bug is not by itself proof of an abstraction gap. Do not keep a private Vulkan escape hatch under SDL_GPU.

### Supplied locale at M8

- **Simple layout passes:** FreeType and Fontconfig remain sufficient.
- **Contextual shaping or substitution is required:** add HarfBuzz at the atlas-layout boundary.
- **Bidirectional paragraph layout is also required:** add FriBidi before HarfBuzz and specify cursor/selection behavior in logical and visual order.

The selected locale is tested end to end. Support for every historical retail locale is not implied.

### Unexpected required asset formats at M4 or consuming milestone

- If a selected dependency already supports the format, add and test the narrow adapter.
- If a maintained, license-compatible in-process decoder exists, record its provenance and adopt only the required path.
- If neither is true, block the consuming milestone and report the archive/logical asset. Do not skip required content or require the user to run a conversion helper.

This applies particularly to `NOX`/LZH, unexpected Bink/audio variants, fonts, textures, effects, and Granny. Granny stays disabled unless an ordinary retail gameplay path proves it is required.

### Loopback discovery at M11/M16

- **Loopback broadcast works:** use it for the real two-process discovery test.
- **Loopback broadcast is not delivered:** use the virtual transport for discovery-state acceptance and direct connect for real two-process acceptance; document physical broadcast as unverified.
- **Distinct loopback binding is unavailable:** fix the injectable address/port design or use network namespaces if available. Do not make a second host mandatory.

### Packaging after M17

The source-build baseline uses distribution packages. A locked dependency superbuild, Flatpak, or AppImage is a later distribution decision and must not complicate the native port. None is required for M17.

## 10. Validation matrix

| Area | GPU-free/portable acceptance | Hardware or optional acceptance | Required failures |
|---|---|---|---|
| Build | Arch x86-64 GCC/Clang Debug/Release; clean offline installed-dependency builds; all GLSL compiles | Ubuntu 26.04 and Fedora 44 clean source-build verification | Missing dependency/compiler/shader names the requirement; unsupported architectures fail clearly; stale generated files cannot mask failure |
| Types/Unicode | Width assertions; UTF-8/UTF-16; surrogate pairs; identical primitive codec bytes across x86-64 compilers/builds | Selected-locale interactive text at M14/M15 | Invalid encoding, overflow, truncation, and forbidden lossy conversion fail predictably |
| Numeric/determinism | Characterized conversions; Linux replay checkpoints across x86-64 compilers/builds | Windows comparison only in optional M18 | NaN/infinity/narrowing behavior is defined; changed rounding mode is detected |
| Data/VFS | Synthetic BIG/collision/traversal tests; headless retail verification; both roots and precedence | User-supplied corpus at M4 onward | Missing/corrupt/colliding/oversized/unsupported logical assets are named |
| Serialization | Linux save/replay round trips; packet byte fixtures; fixed widths and versions | Windows save/replay fixtures only in M18 | Wrong endian/version/length/count and malformed UTF-16 fail before state mutation |
| Renderer contracts | Recorder covers resources, pipelines, uploads, passes, UI/world/effects/video commands, resize, and lifetime failures | One real x86-64 Vulkan GPU at M14; visual and validation acceptance | Unsupported format has tested fallback or clear failure; shader/pipeline errors name source/state key |
| Input/text | Injected SDL events and headless UI command streams | One interactive display plus M14/M15 gameplay | Focus loss releases held state; malformed text/missing glyph cannot corrupt UI/atlas |
| Audio | VFS decode/scheduling, PCM/ADPCM/MP3/corpus formats, null/no-device, callbacks and shutdown | Audible scene checks where a device exists | Corrupt/unsupported asset names its path; callback races/locks are rejected |
| Video | Headless Bink decode, timestamps, frame/audio order, presentation commands | A/V presentation on the M14/M15 system | Missing decoder, corrupt packet, absent fallback, excessive dimensions, and shutdown are explicit |
| Simulation | Headless campaign/skirmish scenarios, Linux persistence/replay, long tests | Full single-player on M15 GPU system | Data/config mismatch and CRC divergence report inputs/checkpoint |
| LAN | Virtual datagrams and two real local headless processes on distinct loopback addresses | Two local windowed instances if practical; physical broadcast optional | Invalid type/length/count/version, timeout, map/data mismatch, disconnect, and CRC disagreement |
| Process | Arbitrary CWD; XDG fallbacks; multiple processes; no display/audio/GPU; staged failure/shutdown | Fullscreen/resize/focus and real devices at M14/M15 | Read-only/unwritable paths and partial subsystem failure are actionable |

## 11. Overall recommendation

Proceed with a compatibility-focused native port that replaces platform edges while preserving Zero Hour's engine, simulation, data formats, and game behavior. SDL3 is the desktop/input foundation. SDL_GPU is the provisional renderer after exhaustive source/API closure and recorder-based command validation, with real device acceptance deliberately delayed to M14. miniaudio is the smallest credible Miles replacement, and FFmpeg is warranted narrowly for Bink. FreeType/Fontconfig, conditional HarfBuzz/FriBidi, POSIX sockets, standard C++ facilities, and system zlib cover the remaining required facilities without creating a new engine.

This ordering keeps M0-M13 useful in x86-64 GPU-less or restricted development sessions. It does not claim that a recorder proves rendering: M14 is a hard gate for pixels and SDL_GPU device behavior, and a failure there can force renderer rework. The verified RTX 4070 Vulkan host is the practical release GPU; the missing validation-layer package must be installed before M14. Multiple GPU families, multiple physical LAN hosts, and Windows save/replay fixtures are not prerequisites. The highest remaining risks are renderer completeness, explicit UTF-16 and fixed-width serialization, floating-point determinism, archive precedence on a case-sensitive host, and Miles/Bink behavior. Each risk now has a named prerequisite, consuming milestone, observable exit, and failure path.
