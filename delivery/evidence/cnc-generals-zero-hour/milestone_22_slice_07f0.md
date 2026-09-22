# M22 slice 07F0 evidence: original logical WorldHeightMap owner

## Source and ownership result

- The canonical `WorldHeightMap` translation unit is linked into the Linux
  full-draw target without changing its class layout or native branch.
- Its CPU-only logical constructor consumes the accepted `OriginalMapLoader`
  provider and owns copied 8x8 height bytes plus border and boundary metadata
  until the last original ref is released.
- Draw dimensions clamp to the loaded map. Two source instances in one process
  and two fresh processes return per-map allocations to the stable post-parser
  baseline; complete process shutdown retains the existing absolute allocation
  check.
- Visual-map construction and seismic state remain typed unavailable. Terrain
  mesh, shroud projection, active effects and physical map pixels are not
  claimed by this CPU-only slice.

## Parser controls

- Positive: complete logical height chunk, exact dimensions/boundary metadata,
  first and last height samples, draw clamping and re-entry.
- Positive optional-tail control: input ends exactly after the complete
  `HeightMapData` chunk, proving later optional chunks may be absent. No generic
  chunk-stream or optional-chunk behavior changed.
- Negative: malformed map magic, probe-scoped strict EOF inside the height
  payload, oversized dimensions, non-logical visual construction and seismic
  access all reject without publishing or retaining a map owner.
- The generated fixture lives in a temporary read-only source root. No retail
  content or symlink content is read or changed.

## Acceptance gates

- Focused GCC Debug and Clang Release source probes pass after the exact
  optional-chunk boundary correction.
- Exact-tree non-LAN suites pass 191/191 in GCC Debug, GCC Release, Clang
  Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan. Leak detection remains
  enabled for both sanitizer suites.
- The four LAN-labeled tests pass 4/4 in all five configurations under the
  established local-socket permission. An earlier GCC Debug 193/195 run is
  retained only as a sandbox diagnostic: its two UDP tests failed with `EPERM`,
  not as acceptance evidence.
- Source identity, provider-removal and all three dependency-ledger checks pass;
  `git diff --check` is clean.

07F1 shroud CPU ownership, visual terrain construction, map pixels, production
default factory selection and the full 07/08/09 acceptance boundaries remain
pending.
