# M22 slice 07F6B evidence: source terrain atlas ownership

## Contract and source result

- Canonical `WorldHeightMap::getTerrainTexture` creates the source-mandatory
  flat-map resource set in native order: a 2048x128 A1R5G5B5 base atlas, an
  `AlphaTerrainTextureClass` alias of that exact handle, and an independently
  owned 2048x1 A8R8G8B8 alpha-edge atlas.
- The generated authored tile occupies the original four-pixel inset. Recording
  witnesses exact converted bytes and all three bounded mip uploads for both
  physical textures, stable coordinates/flip state and zero draw submissions.
- Base and alpha wrappers hold separate source references to one published
  handle. Releasing the base keeps it valid; releasing the alias destroys it
  exactly once; the edge atlas is destroyed separately. Two full generations
  repeat with no stale handle or resource.

## Negative and ownership controls

- Injected failure covers base creation, first base upload, edge creation and
  first edge upload. Every partial transaction leaves all map getters
  unpublished, returns device resources to zero, and permits a successful
  retry. Alias publication rejects missing, duplicate, self and prior-generation
  handles.
- Normal lifetime is map before edge. Destroying an edge with a still-live
  procedural texture owner fails fast instead of looping. Map teardown follows
  native base-then-alias-then-edge order.
- The allocation oracle first runs an equivalent empty Recording/edge lifecycle
  and then captures its baseline, normalizing the already characterized four
  process-scoped DX8/edge bootstrap allocations. Every failure, retry and atlas
  generation returns exactly to that baseline, while device resources return
  absolutely to zero.
- Material `Apply`, terrain submission, `W3DTerrainVisual::load`, active
  blends/cliffs/shroud/effects and physical terrain pixels remain pending.
  Generated config/image inputs are workspace-owned and read-only; retail data
  and retail symlink content were not touched.

## Acceptance gates

- Final leak-capable focused source/Recording controls pass 5/5 under both GCC
  and Clang ASan/UBSan/LSan on the host. The sandbox's LSan ptrace limitation is
  environment-only; leak detection remained enabled for acceptance.
- Final exact-tree non-LAN/non-GPU suites pass 197/197 in GCC Debug, GCC
  Release, Clang Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan. Full
  builds pass in all five presets. Full sanitizer acceptance uses the
  repository's canonical recoverable UBSan mode; focused F6B checks also pass
  with `UBSAN_OPTIONS=halt_on_error=1`.
- LAN tests pass 4/4 serially in all five presets under the established
  local-socket permission.
- Dependency-ledger, presentation-identity and provider-removal controls pass
  3/3. `git diff --check` passes on the slice tree.
