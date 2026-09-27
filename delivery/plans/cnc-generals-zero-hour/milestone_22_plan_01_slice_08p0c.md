# M22 plan 01 slice 08P0C: physical tree provider aggregate

Status: accepted evidence-only composition of C1–C4; executable baseline
`24e7f91c0a83184ecc5e7d157cf28199ef551dce` (C4). No executable or test changes.
Acceptance and unchanged-source gate reuse are recorded in
[C evidence](../../evidence/cnc-generals-zero-hour/milestone_22_slice_08p0c_physical_tree_aggregate.md).

## Goal and boundary

After accepted 08P0B, compose four independently owned children in order:
08P0C1 tree frame-state/cull/sway/topple; 08P0C2 shadow-disabled terrain
tree draw with exact atlas, shroud and lighting pass; 08P0C3 optional
projected tree-decal resource and queue/flush; then 08P0C4 exact full-instance
`W3DTreeDraw` factory publication. The source `doDrawModule` is empty: trees
draw from the terrain pass. Preserve native frame ordering, not an invented
per-Drawable draw. M20/M21 schema-only factory and the other eight unavailable
draw providers remain unchanged.

Revalidate the complete composed owner: matching map/terrain and device
generation, source asset/atlas/buffers, exact Drawable ID, frame update and
visibility, optional projected decal, shrouded tree triangles, move/removal,
failure rollback and clean retry. Do not count a child as physical provider
admission; that occurs only in C4. Frame failures must leave no stale queued
shadow, Recording frame state, registry/terrain entry or Drawable/module owner.

## Validation and commit

Generated tree fixtures cover two generations, real source mesh, optional
shadow variants, first/repeated/moved/removed trees, sway/topple, multiple
types, visibility/shroud and exact Recording frame families/order. Inject
child-owned failure boundaries, assert immediate pre-teardown residuals,
owner removal and retry without world reset. Reuse C4 final-source six
complete builds/nonretail suites, strict host LSan, physical Vulkan controls,
serial LAN 4/4 all six, ledger and diff checks. Commit the aggregate alone:
`delivery: M22 08P0C revalidate physical tree provider`.
