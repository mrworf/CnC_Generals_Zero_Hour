# M22 slice 08D: active water-owner reset after display detachment

## Goal

Close the source `GameClient::reset` lifecycle required immediately after 08C:
the selected source active translucent `WaterRenderObjClass` must reset safely
after `W3DDisplay::reset` has removed it from the scene, while retaining the
original published owner identity until terrain teardown.  A generated
Recording fixture proves the exact source ordering and a redacted retail gate
proves both selected modes complete this lifecycle boundary before the next
unsupported consumer is reported by the outer typed boundary.

## Scope

- Adapt only the original W3D active-water reset/re-entry contract reached by
  `GameClient::reset` / `W3DTerrainVisual::reset` after display reset.
- Keep the active plane's original buffer/resource identity, exact provider
  ownership, and no-water defaults.  The existing selector-scoped fixed 1x1
  cloud owner remains a generated negative/control, not the retail premise.
- Add a generated source-owner probe for display-detach → terrain reset →
  active-water reset, injected failure/retry, two generations, provider removal and
  zero Recording/original ownership.
- Extend the redacted read-only retail audit with an aggregate post-reset
  stage marker only; it may not disclose paths, filenames, hashes, scalar
  values, or bytes.

## Explicit non-scope

- No retail map decoding, map frame, scene draw, pixels, raw D3D/private
  Vulkan, water-grid simulation, new terrain configuration, or relaxed
  unknown/foreign/nonempty-owner behavior.
- No change to ordinary zero-water or active-water ownership semantics.

## Dependencies and source order

08D depends on 08C and is a new prerequisite of aggregate 08.  The original
`GameClient::reset` calls display reset before terrain visual reset.  The
generated fixture must therefore use that order, prove that the active owner
was detached, and prove it remains resettable only under the accepted retail
selector and exact published owner.  The original terrain reset owns the
composition; the fixture may not reorder it or substitute a Recording-only
owner.

## State and failure behavior

The selected active-water owner remains source-owned with its bounded buffers.
A display reset may detach it from the active scene, but must not turn it into
a foreign or unowned object.  Missing selector, pre-detach invocation,
foreign/nonempty owner, cloud-only/no-buffer mismatch, unavailable edge/scene,
resource failure, and stale provider are typed failures.  Failure restores the
prior published state or fully retires owned state; retry and a second
generation must succeed with no retained Recording resources or source
providers.

## Implementation surfaces

- `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp`
  and, only if source composition requires it,
  `W3DTerrainVisual.cpp`.
- The existing generated terrain-water probe/test and the redacted retail
  Recording audit.
- This plan, governing slice index, dependency ledger rows if source imports
  change, and `delivery/evidence/cnc-generals-zero-hour/milestone_22_slice_08d.md`.

## Validation

1. GCC and Clang focused generated reset probe and redacted dual-mode retail
   reachability gate.
2. Source identity/provider-removal/dependency-ledger checks.
3. Fresh six-config non-GPU/non-LAN broad CTest matrix; focused host
   `detect_leaks=1` in GCC and Clang; proportional public Vulkan validation;
   serial LAN 4/4 in all six configurations.
4. `git diff --check`, sanitized evidence review, exact-path staging, and one
   independent `delivery: M22 slice 08D ...` commit.  Preserve the unrelated
   `tests/renderer/test_bgfx_device.cpp` diagnostic.

## Acceptance

- Generated source-owner evidence proves exact display-detach → terrain reset
  → active-water owner reset ordering, positive retry/two-generation behavior, and
  zero resources/providers after teardown.
- Default/selector-absent, foreign, duplicate/nonempty, cloud-only/no-buffer,
  unavailable-provider and injected failure paths fail closed without
  contaminating retry.
- Both read-only retail modes emit only an aggregate post-reset advance marker
  and clean rollback/teardown evidence.
- No deferred map/draw/pixel claim is made.
