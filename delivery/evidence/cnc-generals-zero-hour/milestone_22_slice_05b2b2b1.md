# M22 slice 05B2B2B1: original material and mapper source state

## Source and behavior

- Canonical `vertmaterial.cpp` now executes the original material and null
  `Apply` bodies in the WW3D CPU configuration; canonical `mapper.cpp`
  submits its own matrix, UV index and transform-flag decisions through
  canonical `dx8wrapper.cpp`. The CPU-only wrapper retains original
  reference-counted texture/material/shader selections and the original
  delayed shader → texture stages → material application order. The physical
  shader path is typed unavailable pending B2; physical material uniforms,
  pipeline and pass are pending B3/06. This is not a complete render pass.
- The same original `WW3D::Sync` function is enabled in both configurations.
  An owned W3D material selects the original screen mapper; an owned linear
  mapper advances from 100 to 350 ms and yields source UV offsets 0.75/0.5.
  The latter also selects original unlit lighting, COLOR1 diffuse source and
  second-stage UV index 1; ordered original render/stage-state markers are
  asserted before any physical pass.
  Original CameraClass supplies the projection for the screen mapper in this
  bounded fixture. Missing projection rejects without clearing pending state;
  retry succeeds. Refcounts return after material/texture unbind and after
  edge teardown; a fresh edge replays the original material with no stale
  selections. Null material resets source material and stage state. Invalid
  stage/index, absent edge and pending shader are negative controls.
- Test-only retail material inspection is a separate TU compiled with the
  canonical WW3D CPU ABI. The GameClient host passes an opaque render object;
  a regression exposed that including WW3D mesh inline headers in the host
  otherwise substituted an incompatible weak inline and crashed the existing
  full-draw scenario. The negative ABI isolation check rejects that condition
  and missing CPU macro. The selected read-only retail model reports **three
  original meshes, three materials, zero mapper instances**. It therefore
  cannot itself witness a retail mapper. Owned source-positive screen/linear
  families and later scene/family expansion remain distinct gates. No retail
  bytes, model identity or path are recorded here.

## Validation

- GCC and Clang debug full builds and **143 non-LAN tests each** passed;
  **four LAN tests each** passed sequentially outside the socket-restricted
  sandbox. In particular: `original_w3d_cpu_graph`,
  `original_w3d_full_draw_scenario`, `original_w3d_material_abi_isolation`,
  `original_w3d_cpu_identity`, `original_w3d_provider_removal`, all three
  dependency-ledger tests and the existing schema tests.
- GCC and Clang sanitizer focused graph, retail draw and ABI isolation passed
  outside the ptrace-restricted sandbox (ASan/UBSan/LSan).
- Original provider-removal now fails if original `dx8wrapper.cpp`,
  `vertmaterial.cpp` or `mapper.cpp` is not compiled or linked. Link-map and
  compile-definition check isolate the test-only full ABI from the original
  GameClient host. `git diff --check` passes.

## Outstanding M22 gates

Original shader capabilities/combiner B2, public pipeline translation B3,
interleaved original categories and WWShade 06, original GameClient frame 07,
required-resource/retail recording 08, and validation-layer Vulkan/visual,
resize, teardown and four-preset acceptance 09 remain mandatory. This slice
claims no positive GPU material draw or retail-rendered frame.
