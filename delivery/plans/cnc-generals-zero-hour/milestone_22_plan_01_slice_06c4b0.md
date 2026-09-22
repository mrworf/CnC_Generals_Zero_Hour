# M22 plan 01 slice 06C4B0: original-loader classification of opaque retail W3D entries

## Outcome and boundary

Requires 06C4A. Resolve that slice's five opaque entries in the available ignored Zero Hour BIG by asking the canonical `WW3DAssetManager::Load_3D_Assets(FileClass&)` whether each can load. This is a read-only prerequisite to the C4 WWShade decision, not a rendering change or a complete retail-family audit. Do not print/commit private archive or entry names, paths, bytes, hashes or model identities; do not extract entries to disk or modify the retail symlink. Base Generals archive coverage remains separate.

## Transaction and checks

Extend the bounded BIG/W3D audit to pass each already-classified opaque payload through stdin to a fresh original W3D CPU loader process, with a size limit, timeout and captured output. The helper returns only an accepted/rejected result; crash, timeout or ambiguous status fail the classification. An owned valid W3D packet and deliberately malformed chunk prove the loader's positive/negative behavior, and an owned BIG with both cases proves the report never treats a parseable chunk as opaque. For retail, report only aggregate opaque count, accepted count, rejected count and any unresolved count. If every opaque entry is rejected, the available archive cannot require SHDMESH through those payloads; do not generalize that result to other archives or assert enabled WWShade is unnecessary in all retail scenes. Run source-only GCC/Clang Debug and sanitizer controls, ABI/provider/ledger and four asset-free full suites; commit the plan, tests, source-only helper, aggregate evidence and checker extension as one independent slice. C4 remains pending.
