# M22 plan 01 slice 07F7: minimum-spec terrain shader passes

## Goal and observable outcome

Requires accepted 07F6C. The canonical `W3DShaderManager` owns a bounded Linux
minimum-spec `ST_TERRAIN_BASE` lifecycle. It reports exactly two source passes,
selects the F6B base atlas for opaque pass zero and its alpha alias for
source-alpha pass one, then resets all delayed texture/sampler/state without a
draw submission.

## Dependency finding and scope

The source `TerrainShader2Stage::init` is the minimum-spec fallback and assigns
two passes to `ST_TERRAIN_BASE`. Pass zero clamps and modulates the base texture
with diffuse color while disabling alpha blend. Pass one uses the alpha alias,
the second UV set and source-alpha/inverse-source-alpha blending. The 8-stage
and legacy pixel-shader implementations are optional chipset overrides; they
are not source-mandatory on the public GPU edge.

Compile canonical `W3DShaderManager.cpp` in the full original W3D target with a
bounded Linux branch for init/shutdown, texture slots, pass count, base pass
selection and reset only. Noise/lightmap modes, shroud/mask/road/cloud/flat
shader families, render-to-texture filters, benchmark/capability classification
and D3D shader loading remain typed unavailable. Do not add
`HeightMapRenderObjClass::Render`, any draw, physical pixels,
`W3DTerrainVisual::load`, active effects or retail inputs.

## Entry, state, errors and surfaces

Reuse the generated flat-map atlas fixture and production-compatible Recording
edge. Initialize the manager, publish the base and alias into slots zero and
one, then run both `ST_TERRAIN_BASE` passes. Recording must witness source order,
the shared physical handle with distinct pass ownership, clamp/filter state,
pass-zero opaque modulate state, pass-one alpha modulation/blend state, and
zero draws.

Calling before init, missing either source texture, null/unpublished owners,
wrong pass, noise/effect shader kinds, duplicate init and shutdown with an
active pass must fail without stale state. Injected sampler failure rolls back
the pass and permits retry. Reset and shutdown are idempotent only from an idle
base-mode state. Two manager/map/device generations repeat and return all
resources to zero.

Expected surfaces are canonical `W3DShaderManager.cpp`, its existing header
only for a layout-neutral CPU guard if required, a narrow Linux implementation
include, full-W3D source list, focused generated-map probe, ledger, plan/index
and evidence. Retail inputs and retail symlink content remain untouched.

## Tests and acceptance

Positive tests cover canonical two-pass count/order, exact base/alias handle,
UV/operator/blend/filter state, reset/shutdown, zero draws and two-generation
re-entry. Negative tests cover lifecycle order, absent/unpublished texture,
invalid pass, unsupported shader families, injected sampler failure and retry.

Run GCC and Clang focused source/Recording tests with leak detection, identity,
provider-removal and ledger controls, then exact five non-LAN suites and serial
LAN split. No terrain submission or physical pixel claim is made.

## Commit boundary

One commit: `delivery: M22 slice 07F7 select minimum terrain shader passes`.
