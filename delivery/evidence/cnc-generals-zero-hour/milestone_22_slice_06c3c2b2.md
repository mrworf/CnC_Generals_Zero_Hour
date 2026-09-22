# M22 slice 06C3C2B2 evidence — static-sort failure ownership

## Outcome and boundary

`DefaultStaticSortListClass::Render_And_Clear` retains its authored descending level, hook, object and per-level mesh Flush order. Its Linux exception path now releases a popped node's transferred reference when a hook or object throws; the normal loop increment still releases it once on success. `WW3D::Render_And_Clear_Static_Sort_Lists` restores the entry enable flag even when a callback throws. A CPU-only virtual discard method on the original static-list interface releases queued source refs without invoking callbacks when the enclosing source scene frame aborts; the native ABI is unchanged. The source WW3D abort path also invalidates abandoned mesh categories before selected-state reset.

The owned fixture places original `LightClass`-derived render objects at two levels in `DefaultStaticSortListClass`. Pre-hook, post-hook and object-render failures verify exact `Num_Refs`, entry enable restoration for both true and false, and retained lower-priority work for explicit direct-list retry. A successful retry proves the callback observed static lists disabled during drain, so source rendering cannot requeue itself. Discard releases both levels without callbacks. The second fixture starts a real original WW3D Recording frame with an asset-manager mesh and two queued static objects; injected top-level failure aborts the source frame and clears the remaining list, then a new Begin→Render(scene)→End succeeds without replaying a stale callback.

## Validation

- GCC Debug non-GPU original-rendering: 42/42 pass.
- Clang Debug focused source scene/wrapper/static failure, ABI, provider removal: 5/5 pass.
- GCC and Clang ASan/UBSan/LSan focused frame/scene/wrapper/static failure: 4/4 each pass outside ptrace-restricted sandbox.
- Original dependency ledger and `git diff --check`: pass.

## Residual

This is source failure/retry ownership, not a complete mixed scene or physical pixel acceptance. B3 must prove original scene/static-sort traversal through public bgfx Vulkan and exercise physical clear/category/static/sorting fault boundaries before accepting C2B/C2/C3C/C3.
