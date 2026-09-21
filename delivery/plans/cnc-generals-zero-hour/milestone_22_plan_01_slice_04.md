# M22 plan 01 slice 04: original mesh and WWShade CPU pass graph

## Outcome and dependencies

After accepted slices 01–03, original `HLodClass::Render` traverses its child
meshes and original `MeshClass::Render` executes visibility, frustum, sort,
alpha/shadow override, base/additional-pass, skin/decal, and material
decisions. Original WW3D/DX8Renderer/WWShade CPU schedulers produce ordered
pass descriptions. Physical uploads/draws remain typed unavailable; no
recording frame is claimed in this slice.

## Source closure and ownership

- Existing original `hlod.cpp`, `mesh.cpp`, `meshmdl.cpp`, `meshmatdesc.cpp`,
  `shader.cpp`, `vertmaterial.cpp`, `mapper.cpp`, `camera.cpp`, `scene.cpp`,
  `matpass.cpp`, and original GameClient `W3DScene.cpp` own decisions.
- Inspect, compile and link only the reached CPU portions of original
  `dx8renderer.cpp`, `dx8polygonrenderer.cpp`, `dx8fvf.cpp`, and original
  `wwshade` shader loader/pass sources proved by an owned or read-only retail
  encounter. Retain canonical polygon/material grouping and pass ordering;
  do not assume every DX8/WWShade source or shader variant is reached.
- Original `dx8wrapper.cpp` includes `<D3dx8core.h>` and physical calls;
  neither a fake Direct3D SDK nor a cloned scene/material scheduler is valid.
  The physical edge belongs to slice 05.

## Test and failure contracts

Drive actual GameClient traversal through original model/HLOD/mesh render
methods with authored multi-pass, opaque/alpha, hidden, shadow and skin
variants and sampled read-only retail families. Assert original ordered pass,
shader, texture, material and transform identity, negative missing shader or
material and unsupported sort/override state, typed unavailable at the first
physical command, exactly-once teardown, no mixed ABI, provider removal for
mesh/pass owners, GCC/Clang suites and ledger freshness. No success claim
from calling W3DModelDraw alone or using a proxy renderer.

## Commit boundary

One independently validated commit: `delivery: M22 slice 04 restore original mesh pass graph`.
