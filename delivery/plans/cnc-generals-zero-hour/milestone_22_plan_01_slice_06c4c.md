# M22 plan 01 slice 06C4C: guarded enabled WWShade source-branch compile witness

## Outcome and boundary

Requires 06C4B1. Preserve the authored disabled production `USE_WWSHADE` branch and prove the guarded enabled branch compiles the same canonical `WW3DAssetManager` and `WW3D` source bodies with the same CPU-only ABI. The supplied-retail family audit found no source-required SHDMESH, so this slice does **not** activate enabled WWShade at runtime, add shaders, change loader semantics or accept C4. It is a branch-preservation prerequisite; C4D owns the complete owned mixed frame and remaining source WWShade/flush ordering.

## Entry, negative and verification

Use each GCC/Clang Debug `compile_commands.json` canonical `zh_w3d` command for `assetmgr.cpp` and `ww3d.cpp`, changing only a temporary output object path and adding `-DUSE_WWSHADE=1`. Do not edit generated build objects, compile flags, production targets or class layouts. The enabled objects must compile and reference original `SHD_Register_Loader` and `SHD_Flush` respectively; the production disabled objects must not reference those calls. The existing macro-expansion/source-identity test remains an independent negative witness, and the test must reject a missing source command, unexpected enabled production flag, or missing enabled relocation. Record that this is compile/symbol evidence, not a link/runtime/shader-family claim. Run GCC/Clang focused branch/ABI/provider and four full asset-free suites plus sanitizer controls. One commit with plan, test, evidence and CTest registration; retail content remains read-only.
