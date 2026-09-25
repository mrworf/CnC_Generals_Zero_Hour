# M22 plan 01 slice 08P0C2: shrouded terrain tree pass

## Goal and boundary

After C1, add shadow-disabled tree triangles to the CPU terrain Recording
frame. Preserve native scene order: terrain/track, bounded object decals and
occluded flush, then DoTrees before stencil shadows. Bind the accepted 08P0B
atlas and geometry through exact active-generation wrappers. Implement the
native tree shader stage-1 shroud texture, transform, color/alpha test and
lighting, depth/fog/pass state; never substitute an unshrouded or proxy pass.
Keep projected tree decals and full-instance factory closed for C3/C4.

Preflight every frame resource and capacity before submission. On render or
provider failure, rollback owned Recording state/commands and restore the
prior accepted frame; owner removal and clean retry must work without reset.
Preserve C1 instance identities and state. Authorization is not applicable;
this is generated, asset-free runtime work.

## Validation and commit

Generated fixtures assert source vertex/index/atlas use, exact shroud stage
and texture transform, lighting/depth/fog/pass commands, camera-hidden and
visible behavior, ordering among adjacent scene families and two generations.
Inject frame/resource failures and provider removal; assert no partial frame,
no lost accepted identity and successful retry. Run six complete builds and
canonical nonretail suites, focused strict host LSan, physical Vulkan
controls, serial LAN 4/4 all six, ledger and diff checks on final source.
Commit one slice: `delivery: M22 08P0C2 record shrouded tree terrain pass`.
