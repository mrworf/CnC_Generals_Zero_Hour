# M0 slice 02: offline bootstrap graph

## Goal and observable outcome

On x86-64, all four named GCC/Clang Debug/Release presets configure offline, build an asset-free graph with the intended final target names, compile a bootstrap shader, and pass labelled smoke/probe tests.

## Scope

- Add CMake 3.25+/C++17 configuration, the six authorized options, explicit target graph, deterministic flags, imported distribution dependencies, FFmpeg imported targets, and generated revision metadata.
- Add `linux-gcc-debug`, `linux-clang-debug`, `linux-gcc-release`, and `linux-clang-release` configure/build/test presets.
- Compile a project-owned GLSL bootstrap shader with a declared `glslc` custom command.
- Add asset-free smoke behavior, actionable negative probes, stable CTest label vocabulary, and a distribution package build guide with the tested Arch snapshot.

## Non-scope

- Claim that legacy game sources link, initialize SDL/GPU/audio/video, consume retail media, or pass later milestone labels.
- Download dependencies or vendor packages.

## Dependencies and ordering

Depends on slice 01's explicit inventory. The graph exposes that inventory as target metadata without compiling it.

## Entry point and behavior

Contributors run the canonical preset configure/build/test commands. Configure prints the x86-64 host and resolved dependency versions. `zh_main --bootstrap-smoke` verifies the component graph and generated metadata and exits without devices, display, network, or assets.

## Data and state transitions

Configure creates build-local generated metadata; build creates the executable and SPIR-V. Source and retail trees remain unchanged.

## Authorization and permissions

Not applicable. Normal configure/build writes only its preset build directory and performs no network or retail access.

## Validation and error handling

- Positive: each preset configures, builds, and passes CTest; foundation label selection runs smoke and inventory checks.
- Negative: a missing required program names the capability/package; an unsupported architecture names x86-64; absent retail variables remain harmless with retail tests OFF.
- Boundary: Debug/Release and GCC/Clang all generate compile commands and identical graph metadata except expected compiler/build fields.

## Implementation surfaces

- Root CMake files/presets and modules under `cmake/`
- Bootstrap sources under `src/bootstrap/` and one shader under `shaders/bootstrap/`
- `docs/building-linux.md` and M0 probe tests

## Validation commands

- `cmake --preset <each preset>`
- `cmake --build --preset <each preset>`
- `ctest --preset <each preset> --output-on-failure`
- `ctest --preset linux-gcc-debug -L foundation --output-on-failure`
- `git diff --check`

## Acceptance criteria

- Canonical commands work under four presets on Arch x86-64 without retail data or devices.
- Only the six supported `ZH_*` options are public, GPU/retail default OFF, and production sources are not globbed.
- All named target boundaries including WWShade, tests, and null backends are modeled.
- Missing dependency/compiler/shader and unsupported architecture diagnostics are actionable.
- Build revision, architecture, compiler, dependency versions, and package instructions are observable.

## Commit boundary

Commit the CMake graph, presets, bootstrap implementation, tests, build guide, and completed plan evidence as `delivery: M0 slice 02 add bootstrap build graph`.
