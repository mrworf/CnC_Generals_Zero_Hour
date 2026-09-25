# M22 plan 01 slice 08P0B3: tree geometry and RNG aggregate

## Goal and boundary

After accepted 08P0B3A–08P0B3B, revalidate the complete source tree
geometry and GameClient RNG transaction together. The source client stream
must advance exactly once on successful admission and not at all on failed
asset, geometry or Recording work. Repeated position/transform and removal
retain source identity; rejected candidates preserve accepted owners and
allow retry. The physical factory remains closed; frame, shroud and shadow
remain 08P0C.

## Validation and commit

Generated aggregate fixtures compare vertex/index/atlas resource bytes and
client RNG progression against a fresh equivalent generation, including
failed admission followed by clean retry. Re-run 08P0A registry negatives
with ready generated assets. Carry 08P0B3B's six full builds, canonical
nonretail suites, strict host LSan, physical Vulkan and serial LAN controls;
check ledger/diff. Commit one aggregate slice:
`delivery: M22 08P0B3 revalidate tree resource transaction`.
