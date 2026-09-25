# M22 plan 01 slice 08P0C: physical tree provider and frame

## Goal and boundary

After accepted 08P0B, complete the original `W3DTreeDraw` physical provider
through the exact terrain-owned tree buffer. Review its source call graph
end-to-end: factory create proc and module data; first transform; `addTree`,
move, remove and reset; cull, sway, push-aside/topple/sink and sort; atlas
binding, shroud texture, lighting, depth/fog/pass state, tree triangles and
optional projected shadow queue/flush. Preserve source decisions and frame
order on the public Recording GPU edge. The source `doDrawModule` is empty:
trees draw from the terrain pass, so do not invent a per-Drawable model draw.

Only once generated frame and rollback contracts pass may the full-instance
`W3DModuleFactory` register the exact physical `W3DTreeDraw` create proc.
Keep the M20/M21 schema-only factory unchanged. Require loaded matching
terrain/map, valid asset/atlas/buffers, exact Drawable ID and test-only
Recording provider before publication. Failure at creation, admission,
resource upload, frame submission, shadow queue or replacement must unwind
tree/module/terrain/GPU ownership in reverse; no stale partition, drawable or
queued shadow; clean retry must succeed without resetting the world. Retain
unavailable-provider rejection for the other eight schema-only classes.

## Validation and commit

Generated tree fixtures cover real source mesh and optional shadow variants,
first transform, repeated/moved/removed trees, source sway/topple state,
multiple types, visibility/shroud, exact Recording frame families/order and
two generations. Inject every owned boundary; assert immediate pre-teardown
residuals, reverse unwind, provider removal and retry. Defaults and other
schema-only draw providers remain closed. Run six complete builds/suites,
focused strict host LSan, physical Vulkan 2/2, serial LAN 4/4 each six,
ledger and diff checks on final source. Commit one slice:
`delivery: M22 08P0C record physical tree frames`.
