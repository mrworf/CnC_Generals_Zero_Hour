# M29 slice 03 — reproducible offline bgfx shader closure

## Goal and observable outcome

All required owned UI, terrain, water, point, WWShade, original-FVF and effects shader families build as bgfx-compatible binaries offline under all four x86-64 presets, with repeatable outputs and a checked semantic mapping for uniforms, textures, alpha, points, BC fallback and states.

## Scope and non-scope

Pin/review public bgfx, bx, bimg and shader-compiler revisions/licenses; provide reproducible source/tool invocation and checked artifact inventory. Preserve old SPIR-V outputs as historical SDL_GPU evidence until M30 switches device ownership. No retail shaders/assets or GPU hardware acceptance.

## Dependencies and ordering

Requires slices 01–02 final command vocabulary and inventory. Investigate compiler input dialect and dependency size before editing production paths; do not claim existing glslc SPIR-V bytes are bgfx binaries.

## Entry point and behavior

CMake offline shader target consumes explicitly pinned public tool/source inputs, builds every required family or fails if the toolchain, pin, shader or expected output is missing. A manifest check compares source families to generated outputs and asserts relevant semantic features. Repeat builds produce identical bytes.

## Data/state, authorization and errors

Generated binaries stay in build directories, not retail roots. Only public permissively licensed toolchain input is allowed. No user authorization applies. Missing/wrong revision, license, compiler, unsupported shader feature or nondeterministic output is a hard build/test failure, not silent fallback.

## Surfaces

`CMakeLists.txt`, `cmake/`, `tools/renderer/`, `shaders/`, `tests/renderer/` and focused shader tests, `third_party/` pin/license record or equivalent documented external source cache, renderer and source dependency docs.

## Tests and validation

- Positive: every family builds from pinned inputs in all four GCC/Clang Debug/Release presets; same inputs repeat byte-for-byte; state/uniform/format mapping checked.
- Negative: missing/wrong pin/tool, omitted family or unsupported feature fails explicitly.
- Run `cmake --preset` and `cmake --build --preset` for four presets, focused shader/inventory tests, then full asset-free CTest for M29 completion. Record historical evidence grades and unmodified SDL3 platform/input.

## Acceptance and commit

Meets M29 offline shader and cumulative validation clauses. One reviewable commit contains this plan, toolchain/shader build path, tests and evidence; record any genuine dependency blocker at this exact slice rather than waiving the gate.
