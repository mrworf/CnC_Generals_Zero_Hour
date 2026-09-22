# M22 slice 07F2 evidence: original base terrain map/shroud binding

## Source and ownership result

- The canonical Linux `BaseHeightMapRenderObjClass` branch now constructs and
  destroys its source `W3DShroud`, binds exactly one ref to an accepted logical
  `WorldHeightMap`, records dimensions and height extrema, and returns both
  owners through explicit free and destruction.
- Reset preserves the source map ref while clearing shroud levels; explicit
  free releases the ref and permits a second successful bind. Two fresh
  processes complete the same ownership and teardown path.
- The production default remains unchanged. A zero partition cell size rejects
  before publication; the focused probe then installs `MAP_XY_FACTOR` only for
  the bind control and restores the prior value before teardown. This records a
  required upstream partition prerequisite rather than creating a product
  default.

## Negative and isolation controls

- Missing original display/edge ownership, null and dimension-mismatched maps,
  duplicate binding and requested visual preprocessing all reject without
  changing refs or owner state.
- Base mesh/render entry points remain unsupported in the probe and Recording
  captures zero resources and commands. Derived terrain mesh, visual load,
  active shroud projection and physical pixels remain pending.
- The generated map lives under a temporary read-only root; failures redact
  private paths. Retail inputs and retail symlink content are untouched.
- Allocation counts return to the post-display baseline after owner teardown,
  and the accepted display/edge teardown contract remains intact.

## Acceptance gates

- Focused source controls: GCC Debug base-owner 1/1; Clang Release map, shroud
  and base-owner 3/3; GCC ledger/display/view/base controls 6/6.
- Exact-tree non-LAN: GCC Debug 193/193, GCC Release 193/193, Clang Release
  193/193, GCC ASan/UBSan/LSan 193/193 and Clang ASan/UBSan/LSan 193/193.
  Leak detection remains enabled for both sanitizer runs.
- LAN label under established local-socket permission: 4/4 in all five
  configurations.
- Source identity, provider-removal, dependency-ledger controls and
  `git diff --check` pass.

Derived `HeightMapRenderObjClass` mesh ownership, `W3DTerrainVisual::load`,
terrain/effect pixels, active shroud projection, retail coverage and the full
07/08/09 boundaries remain pending.
