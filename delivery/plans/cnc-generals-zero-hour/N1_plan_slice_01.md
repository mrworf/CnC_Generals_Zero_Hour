# N1 slice 01 — public API and rendering semantics

## Goal, scope and entry point

A developer builds pristine official bgfx/bx/bimg plus shaderc and runs generated
physical Vulkan fixtures for original source-required rendering semantics. Cover
clears, selective stencil writes/shadows, authored mip sampling, source formats,
packed attributes, transparency and ordered scene-copy/distortion behavior.
Do not integrate the original world or read retail assets in this slice.

## Dependencies and authority

N0 accepted. Host tools/development headers, actual GPU and validation were
verified by readiness. Public dependency acquisition is authorized. No OS package
installation, user accounts, asset modification, agents or remote writes apply.

## Implementation surfaces

New dependency lock/provenance file, bootstrap/check script, focused CMake build,
owned stock-language shaders and generated rendering tests. Library sources and
build products live under ignored build/, never under the recovery archive.

## State and ownership

Acquire → pin/check cleanliness → build stock tools/runtime → compile owned
shaders → validate generated input → submit/read back → destroy/shutdown.
Source-release callbacks are distinct from GPU completion. Retain readback data
until the public completion frame. No private allocator or transaction protocol.
Pre-submission fixture validation must reject malformed ranges/state before use.

## Investigation tasks before dependent code

Verify the official public contract for mip/layer views, stencil write masks and
depth bias, not just internal helper behavior. Verify shaderc conventions and
upstream builds under installed GCC/Clang. Pin actual official commits. If a
required operation lacks a supported API, produce a minimal source/public-contract
finding and stop dependent integration for architecture reassessment. A source
gap does not require constructing tests around an unsupported private encoding.

## Tests and acceptance

Positive: pristine sources reproduce build and known pixels for all listed
semantics. Negative: wrong/dirty pins, invalid generated descriptors, outside
clear regions, wrong mip selection, overwritten stencil bits and reversed copy
order are detectable failures. Missing GPU/validation is an environment error.
Do not accept only a triangle or full-target clear as source-scoped coverage.

Compile/run commands are persisted with bootstrap/test implementation. No old
build output is valid input. Broaden the matrix after coherent tests pass.
Commit only after checks pass, or commit a documented failed-gate checkpoint
without claiming this slice or N1 accepted.

## Fixture design refinement before completion

Register independent semantic families as individual CTest cases, retaining a
same-process all-family run. Projected-shadow coverage uses a generated render
target, explicit STQ matrix transport, fragment division and multiplicative
blending traced to matrixmapper.cpp/texproject.cpp. Test orthographic and varying-Q
projection with outside-shadow controls; this is capability evidence, not full
original shadow integration. Exercise generated BC1/BGRA uploads separately.
Validate fixture mip views before submission (zero count, overflow, non-finite
coordinates, invalid handle), and prove that rejection leaves a valid candidate
usable. Both compiler sanitizer builds instrument the repository-owned fixture;
the stock runtime is built without local source/configuration patches. Full scene
capacity and original allocator integration remain slice 02/N2 obligations.

## Completion

Passed the normal-host 12-case suite and both compiler ASan/UBSan 12-case suites
using the explicitly documented isolated-bus fixture. Stock source verification
passed after execution. Evidence: `evidence/qa/N1-stock-renderer-semantics.md`.
Parent commit: `ade34a49051105b59c2654a1a9f3a51c7b0dda3d`.
The slice commit is the Git commit introducing this file and the test sources;
resolve with `git log --diff-filter=A --format=%H -- delivery/plans/cnc-generals-zero-hour/N1_plan_slice_01.md`.
N1 remains incomplete until slice 02 passes.
