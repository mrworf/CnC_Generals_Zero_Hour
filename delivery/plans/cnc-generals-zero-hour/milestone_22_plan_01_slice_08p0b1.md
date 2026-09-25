# M22 plan 01 slice 08P0B1: exact tree model references

## Goal and boundary

Translate source `W3DTreeBuffer::addTreeType` model acquisition into a
checked CPU-only owner. Resolve `modelName` through the published original
WW3D asset manager. Preserve case-insensitive type identity and the 64-type
budget. Accept a real mesh or the source HLOD first-child mesh with its
bone/translation offset; capture mesh/model identity, bounds and shadow
inputs. Reject absent assets, missing first child, wrong render class and
malformed mesh/bounds. An ambiguous native `0` failure return must never
become a successful type slot.

The candidate owns each acquired render/HLOD/mesh reference exactly once.
Failure, capacity exhaustion, provider loss, reset and teardown release in
reverse acquisition order without changing accepted registry entries or
scene/terrain owners. Direct generated preparation remains separate from
08P0A registry-only admission until 08P0B3 composes publication. No
texture/atlas, GPU resource, draw, physical factory or frame admission.

## Validation and commit

Generated original W3D mesh/HLOD packets prove identity, offset and bounds;
missing provider/asset, non-mesh and empty child reject. Inject partial
reference and type-capacity failures, observe immediate reference residuals,
reverse release and deterministic retry in the same and fresh map generation.
Run six complete builds and canonical nonretail suites, focused strict host
LSan, physical Vulkan display/bootstrap/map controls, serial LAN 4/4 all
six, ledger and diff checks on final source. Commit one slice:
`delivery: M22 08P0B1 own tree model refs`.
