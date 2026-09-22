# M22 plan 01 slice 06C3C2B3B0: public bgfx decal depth bias

## Outcome and boundary

Unblock the original source decal draw on public bgfx without changing source geometry or source D3D8 state. The existing original GPU edge maps source ZBIAS 0 and 8 to public raster depth bias 0 and -8. This slice maps exactly those two values through the pinned public bgfx per-draw `setDepthControl` API. Other raster bias values remain rejected. It does not add a new backend, alter retail symlinks, or change game/UI behavior.

## Entry, state and errors

A caller creates a public pipeline and draw with D24S8 and raster depth bias -8 or 0. BgfxGpuDevice validates the pipeline and active target, selects per-draw constant depth control, submits, and resets the control on every subsequent draw. Invalid/unsupported bias retains an explicit rejected result before submission. Resource ownership remains with the public device caller; teardown destroys targets, buffers, shaders, and pipeline.

## Validation

Direct physical positive: two coplanar source-color draws with -8 versus 0 yield a distinguishable visible color outcome; a following zero-bias draw proves no carry-over. Negative: unsupported nonzero bias is rejected with no draw and no leaked public resources. Use Vulkan validation-layer output and GCC/Clang ASan/UBSan as proportional controls. Then retry the B3B original-scene decal frame; B3B still owns its full mixed pixel matrix, lifecycle, and four-preset suite gate. Commit this prerequisite independently with evidence.
