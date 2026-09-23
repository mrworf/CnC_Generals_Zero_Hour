# M22 slice 07F8 evidence: bounded original base-terrain submission

## Delivered boundary

`HeightMapRenderObjClass::Render` now follows the smallest source-ready
terrain path for an initialized, generated flat map. It binds the existing
fixed 32x32 source index/vertex buffers, retains the source owner's outer
material/shader selection, selects the accepted F7 base-terrain passes, and
records exactly two indexed submissions.

The owned probe verifies native 16-bit ranges (`first_index=0`,
`count=6144`, `base_vertex=0`, `4096` vertices), pass ordering, the shared
base/alpha atlas handle, and source-degenerate unfilled cells. It also
verifies no-frame, hidden, texture-disabled, cloud-map, light-map and
post-release rejection; injected abort at pass zero and pass one clears
terrain state and retries through the normal delayed source-state path.

Recovery deliberately reselects the original owner material/shader before
each terrain-pass setup. It does not force-clear `ShaderDirty` or mutate the
recording edge directly; `DX8Wrapper::Draw_Triangles` remains the state
application point.

## Scope controls

All inputs are generated, owned map/atlas fixtures. No retail root or retail
content was read. This is Recording-only evidence: it makes no physical GPU,
pixel, resize/recreation, W3DTerrainVisual, full terrain-load, or active
terrain-effect claim.

## Validation

- Focused flat-terrain Recording probe: GCC Debug and Clang Debug, 1/1 each.
- Focused terrain/source/identity/provider group: GCC Debug and Clang Debug,
  5/5 each.
- Exact asset-free non-LAN/non-GPU suites: GCC Debug 197/197 (216.55s), GCC
  Release 197/197 (146.76s), clean Clang Release rerun 197/197 (93.92s), GCC
  recoverable ASan/UBSan/LSan 197/197 (500.43s), and Clang recoverable
  ASan/UBSan/LSan 197/197 (476.83s).
- Serial LAN suites: 4/4 in each of GCC Debug, GCC Release, Clang Release,
  GCC recoverable ASan/UBSan/LSan, and Clang recoverable ASan/UBSan/LSan.
- `python3 tools/check_original_dependency_ledger.py --root . --ledger
  docs/original-runtime-dependency-ledger.tsv` and `git diff --check` pass.

One earlier Clang Release broad run reported 196/197 because the unrelated
`audio_acceptance` timing assertion (`shutdown queues voice completion`)
failed. Its focused rerun passed 1/1, and the subsequent clean full Clang
Release rerun above is the acceptance result. No audio code or thresholds
were changed for this slice.

## Deferred

Retail scene/recording reachability remains M22 slice 08. Public Vulkan
terrain pixels, resize/recreation, and visual review remain M22 slice 09.
