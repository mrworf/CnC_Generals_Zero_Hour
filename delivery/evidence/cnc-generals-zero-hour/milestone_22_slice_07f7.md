# M22 slice 07F7 evidence: minimum terrain shader passes

## Contract and source result

- The canonical `W3DShaderManager.cpp` now has a bounded Linux CPU-only branch
  for the source minimum-spec `ST_TERRAIN_BASE` lifecycle. It exposes exactly
  two terrain-base passes; optional noise, shroud, effects and higher-capability
  shader families fail closed.
- Pass zero selects the published F6B base atlas with UV set zero, clamp-edge
  bilinear/nearest-mip sampling, texture/diffuse modulation, disabled alpha and
  opaque blending. Pass one selects its alpha alias (the same physical handle)
  with UV set one and source-alpha/inverse-source-alpha blending.
- Missing or unpublished atlas owners, invalid passes, duplicate initialization,
  active-pass shutdown and injected sampler creation all reject without stale
  delayed state. Reset and idle shutdown are idempotent; two map/device
  generations release every sampler and source resource. No terrain submission
  or physical pixel claim is made.

## Validation and controls

- The focused source/Recording, shader-source identity and provider-removal
  controls pass 4/4 in each host GCC and Clang ASan/UBSan/LSan build with leak
  detection enabled and the repository's recoverable UBSan setting.
- Exact-tree non-LAN/non-GPU CTest suites pass 197/197 in GCC Debug, GCC
  Release, Clang Release, GCC ASan/UBSan/LSan and Clang ASan/UBSan/LSan. The
  sanitizer suites ran on the host so LeakSanitizer was meaningful.
- LAN tests ran serially under the established local-loopback permission and
  pass 4/4 in each of the same five builds. The sandbox UDP denial was
  environment-only and was not treated as product evidence.
- Dependency-ledger validation and `git diff --check` pass. The 07F7 source,
  atlas and edge changes are included in the ledger; the pre-existing unrelated
  bgfx depth-bias diagnostic remains unstaged and excluded from this slice.

## Scope boundary

Generated map and texture fixtures are workspace-owned. Retail roots and the
read-only retail symlink were not read or modified. `HeightMapRenderObjClass`
submission, terrain visual loading, active terrain effects, retail recording
and physical terrain pixels remain pending later M22 work.
