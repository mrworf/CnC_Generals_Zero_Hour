# M22 plan 01 slice 08P0B2: bounded source tree atlas

## Goal and boundary

After 08P0B1, translate source `W3DTreeBuffer::updateTexture` tile
inventory and packing without GPU upload. Preserve terrain-TGA then
general-TGA search order through the owned file system, actual TGA pixel
format and dimensions, source half-tile/first-tile/tile-width decisions,
case-insensitive texture dedupe, 64-pixel tile-border placement, vertical
tile orientation and UV origin. Enforce 512 tiles, 2048-pixel atlas edge,
valid format/dimensions and overflow-safe allocation.

Stage tile references and atlas pixels before marking a new type texture
ready. Missing, malformed or unsupported TGA; duplicate alias conflict;
capacity exhaustion; partial read or allocation failure; provider removal;
reset and teardown must release only the candidate and preserve accepted
models/tiles. Retry on the same terrain and a fresh map generation must be
deterministic. No synthetic fallback tile, GPU upload, physical factory or
frame admission.

## Validation and commit

Generated TGA files assert exact pixels, formats, vertical orientation,
packing placement, dedupe and limits. Fault every read/allocation boundary;
record immediate pre-teardown reference/pixel residuals and same/fresh-map
retry. Run six complete builds and canonical nonretail suites, focused
strict host LSan, physical Vulkan display/bootstrap/map controls, serial
LAN 4/4 all six, ledger and diff checks on final source. Commit one slice:
`delivery: M22 08P0B2 own tree texture atlas`.
