# M22 plan 01 slice 08P0C3: optional projected tree decals

## Goal and boundary

After C2, own native optional projected tree-decal resources and frame
queue/flush on the CPU path. The native projected-shadow source is not linked
there, and the existing bounded object-decal owner requires a `RenderObj`;
neither is a substitute for tree decal admission. Preserve source eligibility
(visible/upright/DoShadow), terrain-conforming projection, bounded capacity,
texture/resource lifetime and queue/flush before tree triangles. Keep the
physical `W3DTreeDraw` factory closed until C4. If resource creation and
queue/flush prove separately reviewable, split this plan before code.

Stage optional resources and frame commands without disturbing accepted C1/C2
owners. A failed create, queue, flush, removal or provider replacement clears
only candidate entries and Recording state in reverse order, retains accepted
tree identities and allows retry without reset. Authorization is not
applicable; fixtures are generated and asset-free.

## Validation and commit

Generated fixtures cover shadow-disabled equivalence, eligible/ineligible
trees, optional texture, terrain projection, ordering before tree triangles,
batch bounds, two generations and exact owner removal. Inject each resource
and queue/flush boundary; assert immediate residuals and clean retry. Run six
complete builds and canonical nonretail suites, focused strict host LSan,
physical Vulkan controls, serial LAN 4/4 all six, ledger and diff checks on
final source. Commit one slice:
`delivery: M22 08P0C3 own projected tree decals`.
