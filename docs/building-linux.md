# Building Zero Hour natively on Linux

M0 provides an asset-free bootstrap build for the native port. It verifies the intended target graph and installed dependencies; the legacy game implementation is introduced by later milestones.

## Supported host and commands

The supported architecture is x86-64. Arch Linux is the primary development system. Configure performs no download and does not inspect retail data.

```sh
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug --output-on-failure
```

Replace the preset with `linux-clang-debug`, `linux-gcc-release`, or `linux-clang-release` for the other supported configurations. Each build directory contains `compile_commands.json`. Tests may be selected with `ctest --preset <preset> -L foundation`; the stable label vocabulary for later milestones is `foundation`, `headless`, `data`, `determinism`, `platform`, `renderer-contract`, `ui`, `audio`, `video`, `lan`, `gpu`, and `compatibility`.

`ZH_BUILD_TESTS`, `ZH_ENABLE_ASAN`, `ZH_ENABLE_UBSAN`, and `ZH_ENABLE_LAN` control supported local build behavior. `ZH_ENABLE_GPU_TESTS` and `ZH_ENABLE_RETAIL_TESTS` default to `OFF`. Retail-enabled builds additionally provide local `ZH_RETAIL_ZH_DATA`, `ZH_RETAIL_GENERALS_DATA`, and `ZH_RETAIL_LANGUAGE` cache values. No retail path belongs in a preset or commit.

## Distribution packages

Install these packages before configuring. Package installation is an explicit host action; CMake never uses `FetchContent` or a network fallback.

- Arch Linux: `base-devel cmake ninja gcc clang pkgconf sdl3 shaderc freetype2 fontconfig zlib ffmpeg vulkan-validation-layers`
- Ubuntu 26.04: `build-essential cmake ninja-build gcc g++ clang pkg-config libsdl3-dev glslc libfreetype-dev libfontconfig1-dev zlib1g-dev libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libswresample-dev vulkan-validationlayers`
- Fedora 44: `cmake ninja-build gcc gcc-c++ clang pkgconf-pkg-config SDL3-devel glslc freetype-devel fontconfig-devel zlib-devel ffmpeg-free-devel vulkan-validation-layers`

The Vulkan validation package is listed for the eventual M14 hardware gate; M0 does not open a GPU. If configure cannot find a compiler, library, `pkg-config` module, or `glslc`, it stops and names the missing capability. The Arch host used for M0 had:

| Package | Tested version |
|---|---|
| cmake | 4.4.2-1 |
| ninja | 1.13.2-3 |
| gcc | 16.2.1+r23+gd564253eb6c8-1 |
| clang | 22.1.8-1 |
| pkgconf | 3.0.5-1 |
| sdl3 | 3.4.14-1 |
| shaderc | 2026.3-1 |
| freetype2 | 2.14.3-1 |
| fontconfig | 2:2.18.3-1 |
| zlib | 1:1.3.2-3 |
| ffmpeg | 2:9.0.1-1 |

`vulkan-validation-layers` was not installed during M0 and is required before M14, as recorded by the port plan.

## Asset-free headless execution

M3 adds a native process path that does not initialize SDL, a display, GPU, audio device, video presentation, networking, retail data, or any retired Windows service:

```sh
./build/linux-gcc-debug/zh_main --headless --ticks 60 --state-dir /absolute/writable/path
```

`--ticks` accepts 0 through 1,000,000 and defaults to one. When `--state-dir` is omitted, the process uses `$XDG_STATE_HOME/generals-zero-hour`, falling back to `$HOME/.local/state/generals-zero-hour`. It writes `logs/headless.log` and `last-headless-run.txt` only below that root. The command reports build, architecture, SDL, FFmpeg/Bink, data-root/locale, and intentionally skipped-device capabilities. Exit codes are 0 for success, 2 for command usage, 3 for path/log setup, 4 for initialization failure, and 5 for a runtime failure.

The headless path is independent of the current working directory. It does not invoke a launcher, DRM/CD check, registry, splash window, named mutex, embedded browser, GameSpy, SEH, or `chdir`.

## Retail data verification

M4 adds an in-process, device-free verification entry. Both data roots must be absolute, readable directories; they may name the same combined installation. Command-line values override `ZeroHourDataPath`, `GeneralsDataPath`, and `Language` in `$XDG_CONFIG_HOME/generals-zero-hour/options.ini` (falling back below `$HOME/.config`). No registry, Steam, Wine, or working-directory scan is performed.

```sh
./build/linux-gcc-debug/zh_main --verify-data \
  --zh-data /absolute/path/to/zero-hour \
  --generals-data /absolute/path/to/generals \
  --language English
```

If `--language` and `Language` are absent, verification accepts exactly one populated `Data/<language>` directory. Zero or multiple candidates are reported rather than guessed. This command does not initialize SDL, a window, GPU, audio, or video device and never writes either retail root.

Repeat `--mod /absolute/path/to/mod.big` (or name a directory containing BIG files) to insert explicitly selected mod archives between Zero Hour loose files and the normal Zero Hour archives. Archive discovery is case-insensitive sorted and first-loaded wins. The verifier rejects ambiguous case-only names, traversal, malformed headers/tables, and out-of-range or oversized entries before exposing a resource.

The embedded project manifest comes from `data/corpus/english-manifest.tsv`; it contains only logical names and format, locale, font, effect, compression, and precedence expectations. Verification bounds and validates INI, CSF, W3D, DDS, TGA, WAV, MP3, Bink, TrueType/OpenType, RefPack, zlib, and WWShade metadata. Unknown optional extensions are not treated as required content, while a missing, corrupt, or unsupported manifest entry fails by logical name. Detected NOX/LZH content is an explicit unsupported-format failure; Granny is reported and remains disabled unless a consuming gameplay path proves it required.

Opt-in corpus coverage uses a separate local build so private paths never enter presets or tracked files:

```sh
cmake -S . -B build/retail -G Ninja \
  -DZH_ENABLE_RETAIL_TESTS=ON \
  -DZH_RETAIL_ZH_DATA=/absolute/path/to/zero-hour \
  -DZH_RETAIL_GENERALS_DATA=/absolute/path/to/generals \
  -DZH_RETAIL_LANGUAGE=English
cmake --build build/retail
ctest --test-dir build/retail -L data --output-on-failure
```

For lifecycle testing, `--fail-init <stage>` injects a controlled failure at `paths`, `logging`, `platform`, `renderer`, `audio`, `video`, or `engine`. It exits with code 4 after reporting the stage and tearing down only earlier initialized stages in reverse order. This option is a developer test seam, not a gameplay setting.

`headless_process_isolation` launches two processes before waiting for either. They run from unrelated working directories, use different state roots and deliberately invalid SDL display/audio settings, and must produce disjoint logs and completion records. The test is asset-free and makes no network request.

## What the bootstrap proves

The `zh_main --bootstrap-smoke` test links project-owned stubs across the final engine/device, W3D, WWShade, support, compression, POSIX, SDL, renderer, audio, video, test, and null-backend boundaries. It compiles a project-owned GLSL shader to SPIR-V and reports build revision, architecture, compiler, SDL, and FFmpeg metadata. Invoking `zh_main` without `--bootstrap-smoke` fails so this milestone cannot be mistaken for a playable build.

## Audio validation

M12 vendors the reviewed miniaudio 0.11.25 single header at its pinned upstream
commit; normal configuration never downloads it. Run the asset-free audio suite
with `ctest --preset <preset> -L audio --output-on-failure`. It verifies PCM WAV,
Microsoft and IMA ADPCM WAV, MP3, VFS/BIG seeking, voice scheduling and controls,
queued completions, device-failure fallback, and race-free shutdown. If no audio
device exists, the adapter emits one warning, selects its deterministic silent
sink, and remains playable. Corrupt or missing media is still a data error and
reports the logical asset path.

The optional retail audio corpus probe uses the same separate retail build as
data verification: enable `ZH_ENABLE_RETAIL_TESTS` and provide the two absolute
data roots plus `ZH_RETAIL_LANGUAGE`. It reads media through the VFS without
extraction and prints only committed logical names and detected encodings; it
does not copy, hash, modify, or report physical retail paths. Audible device
checking is optional and is not needed for the null-sink acceptance suite.
