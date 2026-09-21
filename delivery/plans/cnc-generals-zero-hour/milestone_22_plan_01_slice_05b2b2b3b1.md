# M22 plan 01 slice 05B2B2B3B1: applied original state semantics

## Outcome and dependencies

Requires accepted B3A. After the actual original `ShaderClass::Apply`, `VertexMaterialClass::Apply`, mapper and texture/filter methods have issued and completed their delayed source commands, expose a read-only original `DX8Wrapper` snapshot and map it into bounded semantic shader/pipeline inputs. Source owns all decisions and ordering; an adapter neither selects a material nor manufactures geometry, pixels, lights, a pass, or a command sequence. The source snapshot must reject unapplied dirty state; no shader/pipeline/uniform GPU resource creation or Vulkan/frame claim belongs here. B3B2 consumes the mapped result and 06 proves original category issuance/interleaving.

## Reachability and mapping

Map every B2-advertised combiner op (DISABLE, SELECTARG1, SELECTARG2, MODULATE, ADD), original DIFFUSE/CURRENT/TEXTURE arguments and the source's independent color/alpha stages, with literal source order and defined stage-disable behavior. Include authored fallback values after `ShaderClass::Apply`, not requested unsupported pre-fallback values. Map original alpha test/reference/comparison, blend source/destination, depth compare/write, cull, fog enable/color/start/end, material coefficients/selector flags, **including enabled lighting**, and UV source/transform/bump state. Preserve D3D ARGB byte semantics in translated uniforms; do not silently reinterpret packed diffuse as RGBA material authority. Every unsupported value, absent required state, bad UV index/transform and unsafe stage/format combination must fail specifically before a pipeline/cache resource mutation. B3B1 reports the source-authored lit requirement without pretending a light environment exists; B3B2 rejects physical lit execution until 06 restores original category light issuance. Stale texture-generation rejection occurs at B3B2's physical binding. No B3B1 draw is claimed.

## Acceptance and negatives

Owned original shader/material/mapper fixtures compare exact snapshot values and semantic outputs, positive and negative every permitted operation/argument, color-vs-alpha stages, alpha/blend/depth/cull/fog/material/UV mapping and original authored fallbacks. Required read-only retail shader/FVF family aggregate is checked without committing names/paths/bytes/hashes. Inject unsupported source variants and unapplied dirty state; verify retry does not mutate cached public state, stage/source references release on session teardown, and generation invalidation. Provider-removal, single original ABI, source ledger, GCC/Clang full suites, focused sanitizers and `git diff --check` pass. B3B2 must execute the mapped state on Recording and Vulkan; 06 must call it at the source's actual category pass/draw and own category-issued world/light/normal decisions; 07–09 retail and visual gates remain unchanged.

## Commit boundary

One independently validated commit: `delivery: M22 slice 05B2B2B3B1 map original applied state`.
