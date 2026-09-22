# M22 plan 01 slice 07F1: map-owned original shroud data

## Goal and observable outcome

Requires accepted 07F0. The canonical original `W3DShroud` can initialize its
CPU-visible shroud grid from a bounded generated `WorldHeightMap`, preserve
source cell sizing and border-level semantics, accept source set/fill/filter
operations, reset, and initialize again without retaining map or device state.
This is the smallest mandatory map-loaded terrain prerequisite: native
`BaseHeightMapRenderObjClass::initHeightData` always owns and initializes a
shroud in the production build.

## Scope and non-scope

Implement only the Linux CPU branch of the original class without changing its
layout or the native branch. Reuse existing class storage for CPU shroud levels
and keep all allocation/teardown in the original owner. Active texture
projection, material installation, terrain mesh generation, shroud pixels,
map-loaded `W3DTerrainVisual`, tracks, water, shadows, particles, retail maps
and production-default factory selection remain pending. A render or material
operation must continue to reject before a successful frame; this slice may
not turn an absent GPU projection into a silent success.

## Entry, state and error handling

An initialized original-process fixture constructs a bounded generated map via
the canonical `WorldHeightMap` parser, then initializes one `W3DShroud` with
positive cell dimensions. The shroud computes the authored map-cell extent,
starts at the source border level, allows in-range set/get and fill, preserves
out-of-range border reads, and records filter/border changes. Reset releases
all CPU grid storage and permits a second initialization, including a second
fresh process generation. Null maps, non-positive cell dimensions,
oversized dimensions, pre-init cell mutation, duplicate init and active
render/resource/material installation fail with `OriginalW3DDeviceUnavailable` without
changing the prior valid grid. Partial allocation failure unwinds to the
uninitialized state. No authorization or user-data mutation applies.

## Surfaces and ordering

Expected surfaces are canonical `W3DShroud.cpp`, the full-draw source probe,
focused test/CMake registration, dependency ledger, plan/index and evidence.
The generated map stays repository-owned and in memory; retail inputs and the
retail symlink remain untouched. This slice precedes map-data publication in
`BaseHeightMapRenderObjClass`, terrain mesh allocation and any physical shroud
projection.

## Tests and acceptance

Positive tests cover dimensions, initial/border values, set/get, fill, filter,
reset/re-entry and teardown across GCC/Clang. Negative tests cover null/invalid
or oversized input, out-of-range mutation, duplicate init and active
render/resource/material paths, proving state and allocation counts remain unchanged.
Run the focused source test, source identity/provider-removal and dependency
ledger checks, leak-capable GCC/Clang ASan+UBSan controls and all mandated
asset-free suites. The CPU/no-draw control proves the rejected active path
cannot claim projection or physical shroud pixels. Record exact evidence and
commit this slice independently.

## Commit boundary

One commit: `delivery: M22 slice 07F1 own map shroud data`.

## Result

Complete for CPU shroud ownership only. The canonical source class derives its
bounded grid from an accepted logical `WorldHeightMap`, owns current/final
levels, preserves border/query/mutation semantics and releases/reset/re-enters
without retaining an allocation. Pre-init, invalid/oversized, out-of-range and
duplicate operations fail without changing state. Render, initialized resource
reacquire and material installation remain typed unavailable; no projection or
physical shroud pixel is claimed.

GCC Debug/Release, Clang Release and both leak-capable ASan/UBSan/LSan exact
non-LAN suites pass 192/192 each. LAN passes 4/4 separately in every
configuration. Focused GCC/Clang, source identity/provider-removal and the
checked dependency ledger pass. Visual terrain publication, mesh and active
effect projection remain pending dependency children of full slice 07.
