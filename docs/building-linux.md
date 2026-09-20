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

## What the bootstrap proves

The `zh_main --bootstrap-smoke` test links project-owned stubs across the final engine/device, W3D, WWShade, support, compression, POSIX, SDL, renderer, audio, video, test, and null-backend boundaries. It compiles a project-owned GLSL shader to SPIR-V and reports build revision, architecture, compiler, SDL, and FFmpeg metadata. Invoking `zh_main` without `--bootstrap-smoke` fails so this milestone cannot be mistaken for a playable build.
