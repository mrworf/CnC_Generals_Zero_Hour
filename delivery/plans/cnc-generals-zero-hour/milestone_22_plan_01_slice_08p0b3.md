# M22 plan 01 slice 08P0B3: atomic tree geometry and Recording resources

## Goal and boundary

After 08P0B1–08P0B2, prepare original tree mesh vertex/index ranges and
Recording resources. Preserve mesh vertices, normals, diffuse/lighting,
atlas UV transform, rotation, scale, offset, winding and per-tree index
identity. Enforce 30000-vertex/60000-index source budgets, 16-bit index
range and atlas bounds; do not substitute proxy geometry. Allocate/upload
source wrapper-backed Recording vertex, index and atlas resources in source
order, with exact owner/device generation.

Only after all references, atlas pixels, geometry and uploads succeed may
the 08P0A type/instance registry publish the candidate. Geometry/capacity,
wrapper create/upload, provider removal, reset and teardown failures unwind
GPU, atlas and model owners in reverse while preserving accepted owners and
allowing retry. Repeated position/transform must retain source identity.
The physical factory remains closed; draw, shroud and shadow remain 08P0C.

## Validation and commit

Generated mesh/TGA fixtures assert vertex/index/UV bytes and Recording
resource counts. Inject every source/device edge; inspect immediate
pre-teardown residuals, then same/fresh-generation retry. Re-run 08P0A
registry-only negative controls alongside generated ready assets. Run six
complete builds and canonical nonretail suites, focused strict host LSan,
physical Vulkan display/bootstrap/map controls, serial LAN 4/4 all six,
ledger and diff checks on final source. Commit one slice:
`delivery: M22 08P0B3 publish tree resources atomically`.
