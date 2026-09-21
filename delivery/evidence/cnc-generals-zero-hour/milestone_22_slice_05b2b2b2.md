# M22 slice 05B2B2B2: original ShaderClass decisions

## Canonical source and bounded policy

- The existing original `ShaderClass::Apply` body now executes in the WW3D
  CPU configuration; it remains the owner of dirty/unchanged checks,
  blend/alpha-test, fog, depth, cull and primary/secondary color and alpha
  combiner decisions. Original `DX8Wrapper` retains source-issued render and
  texture-stage state and applies the shader before texture and material.
  There is no replacement shader scheduler or physical GPU pipeline in B2.
- The Linux software-combiner profile advertises only `SELECTARG1`,
  `MODULATE` and `ADD` to the original capability-dependent source. Stage
  `DISABLE` and `SELECTARG2` are also direct pass-through semantics. This is
  an explicit B3 implementation obligation, **not** a claim about Vulkan or
  old GPU hardware. The original `ADDSIGNED→ADD`, `SCALE2X→MODULATE` and
  other authored fallback choices run in the source; unsupported details
  without valid fallback (`SUBTRACT`, texture/current-alpha blend,
  alpha `ADDSMOOTH`) reject with a family-specific error. N-patches and the
  source's primary `MODULATE2X` unsupported case reject. The original
  Voodoo3 direct third-stage branch is unreachable with the Linux profile
  and remains in the canonical native branch without fabricated vendor facts.
- On a source exception, shader cache/dirty state and pending stage/render
  maps are restored for a retry. Edge teardown releases references and
  invalidates source shader cache. The original `Set_Fog` global selection
  invalidates the shader before it re-emits authored fog state.
- The read-only retail model already loaded by the original W3D manager
  yields **one unique shader variant** from three meshes: primary
  `MODULATE`, detail color/alpha disabled, fog disabled, opaque blend. The
  aggregate reports only family enums/counts; no private names, bytes,
  asset paths or bit patterns are recorded. This is a selected-model
  observation, not all later campaign/skirmish shader families; 08 must
  expand encounter evidence and any required unsupported variant must be
  escalated rather than skipped.
- Current production `ww3d.cpp` compiles the original `USE_WWSHADE` disabled
  macro branch. An enabled preprocessor witness calls the original
  `SHD_Flush`; it does **not** claim an enabled WWShade runtime frame.

## Validation

- Original shader Recording witness: positive original default and changed
  blend, alpha test/ref, fog, depth/write/cull, both stage operators,
  unchanged short circuit, authored fallbacks, unsupported variant/retry,
  no implicit pass/resource and bounded teardown. The owned W3D mesh source
  graph also issues its actual shader before the typed physical null-texture
  edge; it does not claim a drawn mesh or material pipeline.
- GCC and Clang debug builds and **146 non-LAN + four sequential LAN tests
  each** pass; one GCC parallel schema-identity transient passed both an
  isolated rerun and a complete 146/146 repeat. Link-map/source witness,
  canonical WW3D ABI, original shader/mapper/wrapper provider removal,
  WWShade branch selection, dependency ledger and retail draw pass.
- GCC and Clang focused ASan/UBSan/LSan checks each pass five original
  shader/source-identity/owned-mesh/retail/WWShade tests outside the
  ptrace-restricted sandbox. `git diff --check` passes.

## Outstanding M22 gates

B3 must lower every advertised software operator and the required material,
blend/depth/fog/UV semantics via the public GPU API. Original interleaved
category/WWShade scheduling 06, GameClient scene 07, required retail/recording
08 and physical validation-layer Vulkan/visual/full-presets 09 remain. No
generated stand-in shader, synthetic retail frame or toy pass satisfies them.
