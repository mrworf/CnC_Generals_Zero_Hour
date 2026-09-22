# M22 plan 01 slice 07F0: original logical WorldHeightMap owner

## Goal and observable outcome

Requires accepted 07E. The canonical original `WorldHeightMap` class joins the
Linux full-draw source target and loads a bounded generated map through the
accepted `OriginalMapLoader`, preserving authored dimensions, border,
boundaries and height bytes under original ref-counted ownership. This is the
smallest prerequisite for map-derived shroud and terrain owners.

## Scope and non-scope

Add a source-local `ZH_WW3D_CPU_ONLY` branch without changing class layout or
the native branch. Only logical height data is accepted. Full visual-map tile,
blend, cliff, texture, seismic, terrain mesh, shroud projection, retail-map and
physical draw behavior remains pending and must reject explicitly. No new
adapter map model or copied retail input is allowed.

## Entry, state, errors and surfaces

An initialized original runtime opens an in-memory/generated read-only logical
map with the canonical chunk stream and constructs `WorldHeightMap`. Valid
metadata and samples remain queryable until the last original ref is released;
draw dimensions clamp to map extent. Null streams, malformed/dimension-invalid
maps, non-logical visual construction and pending seismic calls fail without a
published owner or retained allocation. A second fresh process repeats the
contract. No authorization or user-data mutation applies.

Expected surfaces are canonical `WorldHeightMap.cpp`, full-draw source target,
a focused source probe/test, dependency ledger, plan/index and evidence. The
fixture is generated under temporary XDG/read roots; retail data and symlink
content remain untouched.

## Tests and acceptance

Positive tests cover dimensions, border/boundary metadata, height samples,
draw-extent clamping, ref ownership, teardown and two fresh generations.
Negative tests cover missing/malformed logical input, non-logical visual input
and seismic calls, with allocation-baseline recovery. Run focused GCC/Clang,
source identity/provider-removal and dependency ledger checks, leak-capable
GCC/Clang ASan+UBSan controls and all mandated asset-free suites. This CPU-only
slice requires no physical pixel gate. Commit independently before 07F1.

## Commit boundary

One commit: `delivery: M22 slice 07F0 own logical height map`.

## Result

Complete. The canonical source class now owns the accepted logical height-map
dimensions, boundary metadata and height bytes in the Linux full-draw target.
Malformed headers, incomplete height payloads, invalid bounds, visual-map
construction and seismic state fail closed; a complete height chunk with later
optional chunks absent remains accepted. Two in-process instances and two fresh
processes return their instance allocations to the stable parser baseline.

Five exact-tree non-LAN suites pass 191/191 each, including GCC and Clang
ASan/UBSan/LSan with leak detection enabled. The four LAN tests pass separately
in all five configurations with local-socket permissions. The checked source
ledger, provider-removal and identity controls pass. This CPU owner does not
claim a terrain mesh, shroud projection, map pixels, retail coverage or the
full slice 07 contract; 07F1 remains its next dependency child.
